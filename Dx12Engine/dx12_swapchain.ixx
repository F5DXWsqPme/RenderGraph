module;

#include <vector>
#include <utility>

#include <dxgi1_5.h>
#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.swapchain;

import engine.render.core.swapchain;
import engine.render.core.dx12.descriptor_allocator;
import engine.render.core.dx12.descriptor;
import engine.render.core.dx12.rtv;
import engine.render.core.dx12.resource_holder;
import engine.render.core.dx12.descriptor_ref;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Swapchain final : public core::Swapchain
  {
  public:
    Swapchain(ComPtr<IDXGISwapChain4>&& swapchain, const ComPtr<ID3D12Device2>& d3d12Device2,
      int numImagesInSwapchain, DescriptorAllocator& allocator) :
      dxgiSwapchain(std::move(swapchain))
    {
      rtvResources.reserve(numImagesInSwapchain);
      rtvDescriptors.reserve(numImagesInSwapchain);

      for (int i = 0; i < numImagesInSwapchain; ++i)
      {
        {
          ComPtr<ID3D12Resource> rtvResource;
          BOOST_VERIFY(SUCCEEDED(dxgiSwapchain->GetBuffer(i, IID_PPV_ARGS(&rtvResource))));
          rtvResources.emplace_back(std::move(rtvResource), core::Resource::STATE_COMMON);
        }

        rtvDescriptors.push_back(allocator.Allocate(core::Descriptor::HEAP_TYPE_RTV));
        const core::Descriptor& descriptor = *rtvDescriptors.back();
        const dx12::Descriptor& dx12Descriptor = dynamic_cast<const dx12::Descriptor&>(descriptor);
        rtvs.emplace_back(dx12Descriptor);

        const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle = dx12Descriptor.GetCpuHandle();
        d3d12Device2->CreateRenderTargetView(rtvResources.back().GetD3d12Resource().Get(), nullptr, cpuHandle);
      }
    }

    const core::Rtv& GetCurrentBackbufferRtv() override final
    {
      uint32_t currentImageInSwapchain = dxgiSwapchain->GetCurrentBackBufferIndex();
      return rtvs[currentImageInSwapchain];
    }

    core::Resource& GetCurrentBackbufferResource() override final
    {
      uint32_t currentImageInSwapchain = dxgiSwapchain->GetCurrentBackBufferIndex();
      return rtvResources[currentImageInSwapchain];
    }

    void Present() override final
    {
      BOOST_VERIFY(SUCCEEDED(dxgiSwapchain->Present(1, 0)));
    }

  private:
    ComPtr<IDXGISwapChain4> dxgiSwapchain;
    std::vector<ResourceHolder> rtvResources;
    std::vector<DescriptorRef> rtvDescriptors;
    std::vector<Rtv> rtvs;
  };
}
