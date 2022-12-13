module;

#include <optional>
#include <memory>
#include <array>
#include <ranges>

#include <d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.device;

import engine.render.core.device;
import engine.render.core.dx12.command_queue;
import engine.render.core.command_queue;
import engine.render.core.dx12.descriptor_pool;
import engine.render.core.descriptor_pool;
import engine.render.core.descriptor;
import engine.render.core.dx12.descriptor;
import engine.render.core.dx12.command_buffer;
import engine.render.core.command_bundle;
import engine.render.core.dx12.command_bundle;
import engine.render.core.pipeline_layout;
import engine.render.core.dx12.pipeline_layout;
import engine.render.core.shader_bytecode;
import engine.render.core.dx12.shader_bytecode;
import engine.render.core.dx12.pso;
import engine.render.core.dx12.resource_allocator;
import engine.render.core.dx12.descriptor_heap_builder;
import engine.render.core.dx12.descriptor_allocator;
import engine.render.core.dx12.fence;
import engine.render.core.dx12.tlas_builder;
import engine.render.core.image_uav;
import engine.render.core.dx12.image_uav;
import engine.render.core.image;
import engine.render.core.dx12.resource_holder;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Device final : public core::Device
  {
  public:
    Device(
      ComPtr<ID3D12Device2>&& device, const ComPtr<IDXGIAdapter4>& dxgiAdapter4, bool isDebugDevice, D3D_FEATURE_LEVEL featureLevel) :
      d3d12Device2(std::move(device)), descriptorHeapBuilder(d3d12Device2), descriptorAllocator(descriptorHeapBuilder)
    {
      if (isDebugDevice)
      {
        BOOST_VERIFY(SUCCEEDED(d3d12Device2.As(&d3d12InfoQueue)));

        d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
      }

      BOOST_VERIFY(SUCCEEDED(dxgiAdapter4->QueryInterface(IID_PPV_ARGS(&dxgiAdapter))));
    }

    std::unique_ptr<core::CommandQueue> CreateCommandQueue(
      core::CommandBuffer::TYPE type, core::CommandQueue::PRIORITY priority,
      bool tryDisableTimeout) const override final
    {
      D3D12_COMMAND_LIST_TYPE d3d12Type = dx12::CommandBuffer::GetD3d12Type(type);
      D3D12_COMMAND_QUEUE_PRIORITY d3d12Priority = dx12::CommandQueue::GetD3d12Priority(priority);
      D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      if (tryDisableTimeout)
        flags |= D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;

      D3D12_COMMAND_QUEUE_DESC desc = {};
      desc.Type = d3d12Type;
      desc.Priority = d3d12Priority;
      desc.Flags = flags;
      desc.NodeMask = 0;

      ComPtr<ID3D12CommandQueue> d3d12CommandQueue;
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateCommandQueue(&desc, IID_PPV_ARGS(&d3d12CommandQueue))));

      return std::make_unique<CommandQueue>(std::move(d3d12CommandQueue), CreateD3d12Fence());
    }

    std::unique_ptr<core::DescriptorPool> CreateDescriptorPool(core::Descriptor::HEAP_TYPE type, int numDescriptors) const override final
    {
      return descriptorHeapBuilder.CreateDescriptorHeap(type, numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }

    std::unique_ptr<core::CommandBundle> CreateCommandBundle(core::CommandBuffer::TYPE type) const override final
    {
      D3D12_COMMAND_LIST_TYPE d3d12Type = CommandBuffer::GetD3d12Type(type);

      ComPtr<ID3D12CommandAllocator> commandAllocator;
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateCommandAllocator(d3d12Type, IID_PPV_ARGS(&commandAllocator))));

      return std::make_unique<CommandBundle>(d3d12Device2, std::move(commandAllocator), d3d12Type);
    }

    std::unique_ptr<core::Fence> CreateFence() const override final
    {
      ComPtr<ID3D12Fence> fence = CreateD3d12Fence();

      return std::make_unique<Fence>(std::move(fence));
    }

    std::unique_ptr<core::PipelineLayout> CreatePipelineLayout() const override
    {
      D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
      rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
      rootSignatureDesc.NumParameters = 0;
      rootSignatureDesc.pParameters = nullptr;
      rootSignatureDesc.NumStaticSamplers = 0;
      rootSignatureDesc.pStaticSamplers = nullptr;

      ComPtr<ID3D12RootSignature> rootSignature;
      /// TODO: cache
      ComPtr<ID3DBlob> rootSignatureBlob;
      ComPtr<ID3DBlob> error;
      BOOST_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &error)));
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))));

      return std::make_unique<PipelineLayout>(std::move(rootSignature));
    }

    std::unique_ptr<core::PipelineLayout> CreatePipelineLayoutWithTlas(const std::span<DescriptorRange>& inputRanges) const override
    {
      std::vector<CD3DX12_DESCRIPTOR_RANGE> ranges;
      ranges.reserve(inputRanges.size());
      for (const DescriptorRange& inputRange : inputRanges)
      {
        CD3DX12_DESCRIPTOR_RANGE range;
        range.Init(Descriptor::GetD3d12RangeType(inputRange.type), inputRange.count, inputRange.baseRegister);
        ranges.push_back(range);
      }

      std::array<CD3DX12_ROOT_PARAMETER, 2> rootParameters;

      rootParameters[0].InitAsDescriptorTable((unsigned)ranges.size(), ranges.data());
      rootParameters[1].InitAsShaderResourceView(0);
      
      CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc((unsigned)rootParameters.size(), rootParameters.data());

      ComPtr<ID3D12RootSignature> rootSignature;
      /// TODO: cache
      ComPtr<ID3DBlob> rootSignatureBlob;
      ComPtr<ID3DBlob> error;
      BOOST_VERIFY(SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &error)));
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature))));

      return std::make_unique<PipelineLayout>(std::move(rootSignature));
    }

    std::unique_ptr<core::ImageUav> CreateImageUav(const core::Image& image) override final
    {
      const ResourceHolder* dx12ResourceHolder = dynamic_cast<const ResourceHolder*>(image.GetResourceHolder());
      BOOST_ASSERT(dx12ResourceHolder != nullptr);

      DescriptorRef uav = descriptorAllocator.Allocate(Descriptor::HEAP_TYPE_CBV_SRV_UAV);
      D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
      desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
      d3d12Device2->CreateUnorderedAccessView(
        dx12ResourceHolder->GetD3d12Resource().Get(), nullptr, &desc, uav->GetCpuHandle());

      return std::make_unique<ImageUav>(std::move(uav));
    }

    void WriteImageUavToPool(const core::DescriptorPool& descriptorPool, const core::ImageUav& uav, uint32_t slot) const override final
    {
      const ImageUav* dx12Uav = dynamic_cast<const ImageUav*>(&uav);
      BOOST_ASSERT(dx12Uav != nullptr);

      const DescriptorPool* dx12DescriptorPool = dynamic_cast<const DescriptorPool*>(&descriptorPool);
      BOOST_ASSERT(dx12DescriptorPool != nullptr);

      D3D12_CPU_DESCRIPTOR_HANDLE dst = dx12DescriptorPool->GetDescriptor(slot).GetCpuHandle();
      D3D12_CPU_DESCRIPTOR_HANDLE src = dx12Uav->GetDescriptorRef()->GetCpuHandle();

      d3d12Device2->CopyDescriptorsSimple(
        1, dst, src, dx12DescriptorPool->GetDescriptorHeapType());
    }

    std::unique_ptr<core::Pso> CreateComputePso(const core::PipelineLayout& pipelineLayout, const core::ShaderBytecode& cs) const override
    {
      D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};

      const ShaderBytecode* dx12Cs = dynamic_cast<const ShaderBytecode*>(&cs);
      BOOST_ASSERT(dx12Cs != nullptr);
      desc.CS = dx12Cs->GetDx12ShaderBytecode();
      
      const PipelineLayout* dx12PipelineLayout = dynamic_cast<const PipelineLayout*>(&pipelineLayout);
      BOOST_ASSERT(dx12PipelineLayout != nullptr);
      desc.pRootSignature = dx12PipelineLayout->GetD3d12RootSignature().Get();

      desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

      ComPtr<ID3D12PipelineState> pso;
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso))));

      return std::make_unique<Pso>(std::move(pso), dx12PipelineLayout->GetD3d12RootSignature());
    }

    std::unique_ptr<core::Pso> CreateGraphicsPso(const core::PipelineLayout& pipelineLayout, const core::ShaderBytecode& vs, const core::ShaderBytecode& ps) const override
    {
      const D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
      {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
      };

      D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
      psoDesc.InputLayout = {inputElementDescs, 2};

      const PipelineLayout* dx12PipelineLayout = dynamic_cast<const PipelineLayout*>(&pipelineLayout);
      BOOST_ASSERT(dx12PipelineLayout != nullptr);
      psoDesc.pRootSignature = dx12PipelineLayout->GetD3d12RootSignature().Get();

      const ShaderBytecode* dx12Vs = dynamic_cast<const ShaderBytecode*>(&vs);
      BOOST_ASSERT(dx12Vs != nullptr);
      psoDesc.VS = dx12Vs->GetDx12ShaderBytecode();

      const ShaderBytecode* dx12Ps = dynamic_cast<const ShaderBytecode*>(&ps);
      BOOST_ASSERT(dx12Ps != nullptr);
      psoDesc.PS = dx12Ps->GetDx12ShaderBytecode();

      psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
      psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
      psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
      psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
      psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
      psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
      psoDesc.RasterizerState.DepthClipEnable = FALSE;
      psoDesc.RasterizerState.MultisampleEnable = FALSE;
      psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
      psoDesc.RasterizerState.ForcedSampleCount = 1;
      psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

      psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
      psoDesc.BlendState.IndependentBlendEnable = FALSE;
      for (D3D12_RENDER_TARGET_BLEND_DESC& rtDesc : psoDesc.BlendState.RenderTarget)
      {
        rtDesc = 
        {
          FALSE, FALSE,
          D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
          D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
          D3D12_LOGIC_OP_NOOP,
          D3D12_COLOR_WRITE_ENABLE_ALL
        };
      }

      psoDesc.DepthStencilState.DepthEnable = FALSE;
      psoDesc.DepthStencilState.StencilEnable = FALSE;
      psoDesc.SampleMask = UINT_MAX;
      psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      psoDesc.NumRenderTargets = 1;
      psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
      psoDesc.SampleDesc.Count = 1;

      ComPtr<ID3D12PipelineState> pso;
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso))));

      return std::make_unique<Pso>(std::move(pso), dx12PipelineLayout->GetD3d12RootSignature());
    }

    std::unique_ptr<core::Pso> CreateRtPso(
      const core::PipelineLayout& pipelineLayout, const core::ShaderBytecode& shader, int payloadSize, int recursionDepth) override
    {
      CD3DX12_STATE_OBJECT_DESC psoDesc{D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE};

      auto lib = psoDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

      const ShaderBytecode* dx12Shader = dynamic_cast<const ShaderBytecode*>(&shader);
      BOOST_ASSERT(dx12Shader != nullptr);
      D3D12_SHADER_BYTECODE bytecode = dx12Shader->GetDx12ShaderBytecode();
      lib->SetDXILLibrary(&bytecode);

      auto hitGroup = psoDesc.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
      hitGroup->SetClosestHitShaderImport(Pso::closestHitShaderName);
      hitGroup->SetHitGroupExport(Pso::hitGroupName);
      hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

      auto shaderConfig = psoDesc.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
      int attributeSize = 2 * sizeof(float); // barycentrics
      shaderConfig->Config(payloadSize, attributeSize);

      auto rootSignature = psoDesc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
      const PipelineLayout* dx12PipelineLayout = dynamic_cast<const PipelineLayout*>(&pipelineLayout);
      BOOST_ASSERT(dx12PipelineLayout != nullptr);
      rootSignature->SetRootSignature(dx12PipelineLayout->GetD3d12RootSignature().Get());

      auto pipelineConfig = psoDesc.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
      pipelineConfig->Config(recursionDepth);

      ComPtr<ID3D12StateObject> pso;
      BOOST_VERIFY(SUCCEEDED(GetD3d12Device5()->CreateStateObject(psoDesc, IID_PPV_ARGS(&pso))));

      return std::make_unique<Pso>(std::move(pso), dx12PipelineLayout->GetD3d12RootSignature());
    }

    std::unique_ptr<core::ResourceAllocator> CreateResourceAllocator() const override final
    {
      return std::make_unique<ResourceAllocator>(dxgiAdapter, d3d12Device2);
    }

    std::unique_ptr<core::TlasBuilder> CreateTlasBuilder(const core::ResourceAllocator& resourceAllocator) override final
    {
      const ResourceAllocator* dx12ResourceAllocator = dynamic_cast<const ResourceAllocator*>(&resourceAllocator);
      BOOST_ASSERT(dx12ResourceAllocator != nullptr);
      return std::make_unique<TlasBuilder>(GetD3d12Device5(), *dx12ResourceAllocator);
    }

    ComPtr<ID3D12Device2> GetD3d12Device2() const
    {
      return d3d12Device2;
    }

    DescriptorAllocator& GetDescriptorAllocator()
    {
      return descriptorAllocator;
    }

  private:
    ComPtr<ID3D12Fence> CreateD3d12Fence() const
    {
      ComPtr<ID3D12Fence> fence;
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))));
      return fence;
    }

    const ComPtr<ID3D12Device5>& GetD3d12Device5()
    {
      if (!d3d12Device5Opt)
      {
        ComPtr<ID3D12Device5> d3d12Device5;
        BOOST_VERIFY(SUCCEEDED(d3d12Device2->QueryInterface(IID_PPV_ARGS(&d3d12Device5))));
        d3d12Device5Opt = d3d12Device5;
      }

      return *d3d12Device5Opt;
    }

    ComPtr<IDXGIAdapter> dxgiAdapter;
    ComPtr<ID3D12Device2> d3d12Device2;
    std::optional<ComPtr<ID3D12Device5>> d3d12Device5Opt;
    ComPtr<ID3D12InfoQueue> d3d12InfoQueue;
    DescriptorHeapBuilder descriptorHeapBuilder;
    DescriptorAllocator descriptorAllocator;
  };
}
