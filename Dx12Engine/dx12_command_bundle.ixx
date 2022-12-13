module;

#include <utility>
#include <array>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>
#include <iostream>
#include <iostream>

export module engine.render.core.dx12.command_bundle;

import engine.render.core.command_bundle;
import engine.render.core.dx12.command_buffer;
import engine.render.core.buffer;
import engine.render.core.dx12.buffer;
import engine.render.core.dx12.rtv;
import engine.render.core.dx12.descriptor;
import engine.render.core.dx12.descriptor_pool;
import engine.render.core.dx12.resource_holder;
import engine.render.core.vertex_buffer_view;
import engine.render.core.dx12.vertex_buffer_view;
import engine.render.core.pso;
import engine.render.core.dx12.pso;
import engine.render.core.dx12.shaders_table;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class CommandBundle final : public core::CommandBundle
  {
  public:
    CommandBundle(const ComPtr<ID3D12Device2>& d3d12Device2, ComPtr<ID3D12CommandAllocator>&& d3d12CommandAllocator, D3D12_COMMAND_LIST_TYPE d3d12Type) :
      commandAllocator(std::move(d3d12CommandAllocator)),
      commandBuffer(d3d12Device2, commandAllocator, d3d12Type)
    {
      BOOST_VERIFY(SUCCEEDED(commandAllocator->Reset()));
    }

    void Begin() override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      BOOST_VERIFY(SUCCEEDED(graphicsCommandList->Reset(commandAllocator.Get(), nullptr))); /// TODO: setup pso if possible
    }

    void End() override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      BOOST_VERIFY(SUCCEEDED(graphicsCommandList->Close()));
    }

    void CopyBufferRegion(core::Buffer& dst, size_t dstOffset, core::Buffer& src, size_t srcOffset, size_t size) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      ResourceHolder& dx12Dst = GetDx12ResourceHolder(dst);
      ResourceHolder& dx12Src = GetDx12ResourceHolder(src);

      const ComPtr<ID3D12Resource>& dstResource = dx12Dst.GetD3d12Resource();
      const ComPtr<ID3D12Resource>& srcResource = dx12Src.GetD3d12Resource();

      graphicsCommandList->CopyBufferRegion(
        dstResource.Get(), dstOffset,
        srcResource.Get(), srcOffset,
        size);
    }

    void Barrier(core::Resource& resource, core::Resource::STATE newState) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      D3D12_RESOURCE_BARRIER barrier = CreateBarrier(resource, newState);
      graphicsCommandList->ResourceBarrier(1, &barrier);

      resource.SetState(newState);
    }

    void ClearRtv(const core::Rtv& rtv, const std::array<float, 4>& value) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      const Descriptor& descriptor = GetRtvDx12Descriptor(rtv);

      graphicsCommandList->ClearRenderTargetView(descriptor.GetCpuHandle(), value.data(), 0, nullptr);
    }

    void BindVertexBuffer(const core::VertexBufferView& vertexBufferView) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      const VertexBufferView* dx12View = dynamic_cast<const VertexBufferView*>(&vertexBufferView);
      BOOST_ASSERT(dx12View != nullptr);

      graphicsCommandList->IASetVertexBuffers(0, 1, dx12View->GetDx12View());
    }

    void BindGraphicsPso(const core::Pso& pso) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      const Pso* dx12Pso = dynamic_cast<const Pso*>(&pso);
      BOOST_ASSERT(dx12Pso != nullptr);

      graphicsCommandList->SetPipelineState(dx12Pso->GetD3d12Pso().Get());
      graphicsCommandList->SetGraphicsRootSignature(dx12Pso->GetD3d12RootSignature().Get());
    }

    void BindComputePso(const core::Pso& pso) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      const Pso* dx12Pso = dynamic_cast<const Pso*>(&pso);
      BOOST_ASSERT(dx12Pso != nullptr);

      graphicsCommandList->SetPipelineState(dx12Pso->GetD3d12Pso().Get());
      graphicsCommandList->SetComputeRootSignature(dx12Pso->GetD3d12RootSignature().Get());
    }

    void BindRtPso(const core::Pso& pso) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      const Pso* dx12Pso = dynamic_cast<const Pso*>(&pso);
      BOOST_ASSERT(dx12Pso != nullptr);

      GetD3d12CommandList4()->SetPipelineState1(dx12Pso->GetD3d12RtPso().Get());
      graphicsCommandList->SetComputeRootSignature(dx12Pso->GetD3d12RootSignature().Get());
    }

    void SetViewport(float minDepth, float maxDepth, float offsetX, float offsetY, float sizeX, float sizeY) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      D3D12_VIEWPORT viewport{};
      viewport.MinDepth = minDepth;
      viewport.MaxDepth = maxDepth;
      viewport.TopLeftX = offsetX;
      viewport.TopLeftY = offsetY;
      viewport.Width = sizeX;
      viewport.Height = sizeY;

      graphicsCommandList->RSSetViewports(1, &viewport);
    }

    void SetScissorRect(int offsetX, int offsetY, int sizeX, int sizeY) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      D3D12_RECT rect{};
      rect.left = offsetX;
      rect.right = offsetX + sizeX;
      rect.bottom = offsetY + sizeY;
      rect.top = offsetY;

      graphicsCommandList->RSSetScissorRects(1, &rect);
    }

    void SetRtv(const core::Rtv& rtv) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      const Descriptor& descriptor = GetRtvDx12Descriptor(rtv);

      graphicsCommandList->OMSetRenderTargets(1, &descriptor.GetCpuHandle(), FALSE, nullptr);
    }

    void SetDescriptorPoolAndTlas(const core::DescriptorPool& pool, const core::Buffer& tlas) override final
    {
      const DescriptorPool* dx12DescriptorPool = dynamic_cast<const DescriptorPool*>(&pool);
      BOOST_ASSERT(dx12DescriptorPool != nullptr);

      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();
      graphicsCommandList->SetDescriptorHeaps(1, dx12DescriptorPool->GetDescriptorHeap().GetAddressOf());

      graphicsCommandList->SetComputeRootDescriptorTable(0, dx12DescriptorPool->GetDescriptor(0).GetGpuHandle());

      graphicsCommandList->SetComputeRootShaderResourceView(
        1, GetD3d12GpuVirtualAddress(tlas));
    }

    void DispatchRays(const core::ShadersTable& shadersTable, size_t width, size_t height, size_t depth) override final
    {
      D3D12_DISPATCH_RAYS_DESC desc = {};

      const ShadersTable* dx12ShadersTable = dynamic_cast<const ShadersTable*>(&shadersTable);
      BOOST_ASSERT(dx12ShadersTable != nullptr);

      desc.RayGenerationShaderRecord.StartAddress = GetD3d12GpuVirtualAddress(dx12ShadersTable->GetRaygen());
      desc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

      desc.MissShaderTable.StartAddress = GetD3d12GpuVirtualAddress(dx12ShadersTable->GetMiss());
      desc.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
      desc.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

      desc.HitGroupTable.StartAddress = GetD3d12GpuVirtualAddress(dx12ShadersTable->GetHit());
      desc.HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
      desc.HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

      desc.Width = (unsigned)width;
      desc.Height = (unsigned)height;
      desc.Depth = (unsigned)depth;

      GetD3d12CommandList4()->DispatchRays(&desc);
    }

    void Dispatch(size_t width, size_t height, size_t depth) override final
    {
      GetD3d12CommandList()->Dispatch((unsigned)width, (unsigned)height, (unsigned)depth);
    }

    void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t vertexOffset, uint32_t instanceCount, uint32_t instanceOffset) override final
    {
      const ComPtr<ID3D12GraphicsCommandList>& graphicsCommandList = commandBuffer.GetD3d12CommandList();

      graphicsCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      graphicsCommandList->DrawInstanced(vertexCountPerInstance, instanceCount, vertexOffset, instanceOffset);
    }

    void CopyResource(const core::Resource& src, const core::Resource& dst) override final
    {
      GetD3d12CommandList()->CopyResource(GetD3d12Resource(dst).Get(), GetD3d12Resource(src).Get());
    }

    const ComPtr<ID3D12GraphicsCommandList>& GetD3d12CommandList() const
    {
      return commandBuffer.GetD3d12CommandList();
    }

    const ComPtr<ID3D12GraphicsCommandList4>& GetD3d12CommandList4()
    {
      return commandBuffer.GetD3d12CommandList4();
    }

  private:
    D3D12_RESOURCE_BARRIER CreateBarrier(core::Resource& resource, core::Resource::STATE newState)
    {
      ResourceHolder& dx12ResourceHolder = GetDx12ResourceHolder(resource);

      D3D12_RESOURCE_BARRIER barrier = {};
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      if (resource.GetState() == core::Resource::STATE_UAV && newState == core::Resource::STATE_UAV)
      {
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = dx12ResourceHolder.GetD3d12Resource().Get();
      }
      else
      {
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = dx12ResourceHolder.GetD3d12Resource().Get();
        barrier.Transition.StateBefore = dx12ResourceHolder.GetD3d12State();
        barrier.Transition.StateAfter = ResourceHolder::ConvertStateToD3d12State(newState);
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      }

      return barrier;
    }

    ResourceHolder& GetDx12ResourceHolder(core::Resource& resource)
    {
      core::ResourceHolder* resourceHolder = resource.GetResourceHolder();
      ResourceHolder* dx12ResourceHolder = dynamic_cast<ResourceHolder*>(resourceHolder);
      BOOST_ASSERT(dx12ResourceHolder != nullptr);
      return *dx12ResourceHolder;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetD3d12GpuVirtualAddress(const core::Buffer& buffer)
    {
      return GetD3d12Resource(buffer)->GetGPUVirtualAddress();
    }

    ComPtr<ID3D12Resource> GetD3d12Resource(const core::Resource& resource)
    {
      const core::ResourceHolder* resourceHolder = resource.GetResourceHolder();
      const ResourceHolder* dx12ResourceHolder = dynamic_cast<const ResourceHolder*>(resourceHolder);
      BOOST_ASSERT(dx12ResourceHolder != nullptr);
      return dx12ResourceHolder->GetD3d12Resource();
    }

    const Descriptor& GetRtvDx12Descriptor(const core::Rtv& rtv)
    {
      const Rtv* dx12Rtv = dynamic_cast<const Rtv*>(&rtv);
      BOOST_ASSERT(dx12Rtv != nullptr);
      return dx12Rtv->GetDescriptor();
    }

    ComPtr<ID3D12CommandAllocator> commandAllocator;
    CommandBuffer commandBuffer;
  };
}
