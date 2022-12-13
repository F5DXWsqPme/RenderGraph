module;

#include <utility>
#include <memory>

#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.tlas_builder;

import engine.render.core.tlas_builder;
import engine.render.core.dx12.resource_allocator;
import engine.render.core.resource;
import engine.render.core.dx12.resource_holder;
import engine.render.core.dx12.buffer;
import engine.render.core.buffer;
import engine.render.core.command_bundle;
import engine.render.core.dx12.command_bundle;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class TlasBuilder final : public core::TlasBuilder
  {
  public:
    TlasBuilder(const ComPtr<ID3D12Device5>& d3d12Device5, const ResourceAllocator& resourceAllocator) :
      d3d12Device5(d3d12Device5), resourceAllocator(resourceAllocator)
    {
    }

    void CreateResourcesAndWriteBuildCommands(
      const core::Buffer& indexBuffer, int numIndices,
      const core::Buffer& vertexBuffer, int numVertices,
      core::CommandBundle& commandBundle) override final
    {
      D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = CreateGeometryDesc(indexBuffer, numIndices, numVertices, vertexBuffer);

      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = CreateTopLevelInputs(buildFlags);

      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
      d3d12Device5->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
      BOOST_ASSERT(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
      bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
      bottomLevelInputs.pGeometryDescs = &geometryDesc;
      d3d12Device5->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
      BOOST_ASSERT(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

      CreateBuffers(topLevelPrebuildInfo, bottomLevelPrebuildInfo);

      CreateInstanceBuffer();

      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
      bottomLevelBuildDesc.Inputs = bottomLevelInputs;
      bottomLevelBuildDesc.ScratchAccelerationStructureData = GetD3d12GpuVirtualAddress(*scratchBuffer);
      bottomLevelBuildDesc.DestAccelerationStructureData = GetD3d12GpuVirtualAddress(*blas);

      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
      topLevelInputs.InstanceDescs = GetD3d12GpuVirtualAddress(*uploadInstanceBuffer);
      topLevelBuildDesc.Inputs = topLevelInputs;
      topLevelBuildDesc.DestAccelerationStructureData = GetD3d12GpuVirtualAddress(*tlas);
      topLevelBuildDesc.ScratchAccelerationStructureData = GetD3d12GpuVirtualAddress(*scratchBuffer);

      WriteBuildCommands(commandBundle, bottomLevelBuildDesc, topLevelBuildDesc);
    }

    void CreateInstanceBuffer()
    {
      D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
      instanceDesc.Transform[0][0] = 1;
      instanceDesc.Transform[1][1] = 1;
      instanceDesc.Transform[2][2] = 1;
      instanceDesc.InstanceMask = 1;
      instanceDesc.AccelerationStructure = GetD3d12GpuVirtualAddress(*blas);

      uploadInstanceBuffer = resourceAllocator.CreateBuffer(sizeof(instanceDesc), true);
      void* mappedMemory = uploadInstanceBuffer->Map({0, sizeof(instanceDesc)});
      memcpy(mappedMemory, &instanceDesc, sizeof(instanceDesc));
    }

    void WriteBuildCommands(
      core::CommandBundle& commandBundle,
      const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& bottomDesc,
      const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& topDesc)
    {
      CommandBundle* dx12CommandBundle = dynamic_cast<CommandBundle*>(&commandBundle);
      BOOST_ASSERT(dx12CommandBundle != nullptr);
      const ComPtr<ID3D12GraphicsCommandList4>& commandList4 = dx12CommandBundle->GetD3d12CommandList4();

      commandList4->BuildRaytracingAccelerationStructure(&bottomDesc, 0, nullptr);
      {
        D3D12_RESOURCE_BARRIER barriers[] =
        {
          CD3DX12_RESOURCE_BARRIER::UAV(GetD3d12Resource(*blas).Get()),
          CD3DX12_RESOURCE_BARRIER::UAV(GetD3d12Resource(*scratchBuffer).Get())
        };
        commandList4->ResourceBarrier(2, barriers);
      }
      commandList4->BuildRaytracingAccelerationStructure(&topDesc, 0, nullptr);
      {
        D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(GetD3d12Resource(*tlas).Get());
        commandList4->ResourceBarrier(1, &barrier);
      }
    }

    void CreateBuffers(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& topInfo, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& bottomInfo)
    {
      uint64_t scratchSize = max(topInfo.ScratchDataSizeInBytes, bottomInfo.ScratchDataSizeInBytes);
      scratchBuffer = resourceAllocator.CreateBuffer(scratchSize, false, true);

      const D3D12_RESOURCE_FLAGS accelerationStructureFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      const core::Resource::STATE accelerationStructureInitialState = core::Resource::STATE_ACCELERATION_STRUCTURE;
      const D3D12_HEAP_TYPE accelerationStructureHeapType = D3D12_HEAP_TYPE_DEFAULT;
      blas = resourceAllocator.CreateBuffer(
        bottomInfo.ResultDataMaxSizeInBytes, accelerationStructureFlags,
        accelerationStructureInitialState, accelerationStructureHeapType);
      tlas = resourceAllocator.CreateBuffer(
        topInfo.ResultDataMaxSizeInBytes, accelerationStructureFlags,
        accelerationStructureInitialState, accelerationStructureHeapType);
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS CreateTopLevelInputs(
      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags)
    {
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;
      inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
      inputs.Flags = buildFlags;
      inputs.NumDescs = 1;
      inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
      return inputs;
    }

    core::Buffer& core::TlasBuilder::GetTlas()
    {
      return *tlas;
    }

  private:
    D3D12_RAYTRACING_GEOMETRY_DESC CreateGeometryDesc(
      const core::Buffer& indexBuffer, int numIndices, int numVertices, const core::Buffer& vertexBuffer)
    {
      D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc;
      geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
      geometryDesc.Triangles.IndexBuffer = GetD3d12GpuVirtualAddress(indexBuffer);
      geometryDesc.Triangles.IndexCount = numIndices;
      geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
      geometryDesc.Triangles.Transform3x4 = 0;
      geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
      geometryDesc.Triangles.VertexCount = numVertices;
      geometryDesc.Triangles.VertexBuffer.StartAddress = GetD3d12GpuVirtualAddress(vertexBuffer);
      geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(float) * 3;
      geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
      return geometryDesc;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetD3d12GpuVirtualAddress(const core::Buffer& buffer)
    {
      return GetD3d12Resource(buffer)->GetGPUVirtualAddress();
    }

    ComPtr<ID3D12Resource> GetD3d12Resource(const core::Buffer& buffer)
    {
      const core::ResourceHolder* resourceHolder = buffer.GetResourceHolder();
      const ResourceHolder* dx12ResourceHolder = dynamic_cast<const ResourceHolder*>(resourceHolder);
      BOOST_ASSERT(dx12ResourceHolder != nullptr);
      return dx12ResourceHolder->GetD3d12Resource();
    }

    ComPtr<ID3D12Device5> d3d12Device5;
    const ResourceAllocator& resourceAllocator;
    std::unique_ptr<core::Buffer> scratchBuffer;
    std::unique_ptr<core::Buffer> blas;
    std::unique_ptr<core::Buffer> tlas;
    std::unique_ptr<core::Buffer> uploadInstanceBuffer;
  };
}
