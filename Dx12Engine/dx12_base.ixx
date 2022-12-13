module;

#include <vector>
#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <sdl2/SDL_syswm.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.base;

import engine.render.core.base;
import engine.render.core.adapter_properties;
import engine.render.core.swapchain;
import engine.render.core.dx12.factory;
import engine.render.core.dx12.device;
import engine.render.core.dx12.descriptor_allocator;
import engine.os.sdl.window;
import engine.render.core.command_queue;
import engine.render.core.dx12.command_queue;
import engine.render.core.dx12.swapchain;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Base final : public core::Base
  {
  public:
    Base(bool createDebugFactory) noexcept : factory{createDebugFactory}
    {
    }

    std::vector<core::AdapterProperties> GetAdapterProperties() const override final
    {
      std::vector<core::AdapterProperties> result;
      ComPtr<IDXGIFactory4> dxgiFactory4{factory.GetDxgiFactory4()};

      ComPtr<IDXGIAdapter1> dxgiAdapter1;

      for (unsigned int i = 0; true; i++)
      {
        HRESULT enumAdaptersResult = dxgiFactory4->EnumAdapters1(i, &dxgiAdapter1);

        if (enumAdaptersResult == DXGI_ERROR_NOT_FOUND)
          break;

        BOOST_ASSERT(enumAdaptersResult == S_OK);

        DXGI_ADAPTER_DESC1 dxgiAdapterDesc1{GetDxgiAdapterDesc1(dxgiAdapter1)};
        core::AdapterProperties properties{};
        properties.description = dxgiAdapterDesc1.Description;
        properties.isSoftware = (dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE);
        properties.videoMemorySize = dxgiAdapterDesc1.DedicatedVideoMemory;

        result.push_back(properties);
      }

      return result;
    }

    std::unique_ptr<core::Device> CreateDevice(unsigned int adapterId, bool isDebugDevice) override final
    {
      const ComPtr<IDXGIFactory4> dxgiFactory4{factory.GetDxgiFactory4()};

      ComPtr<IDXGIAdapter1> dxgiAdapter1;
      BOOST_VERIFY(dxgiFactory4->EnumAdapters1(adapterId, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND);

      ComPtr<IDXGIAdapter4> dxgiAdapter4;
      BOOST_VERIFY(SUCCEEDED(dxgiAdapter1.As(&dxgiAdapter4)));

      if (isDebugDevice)
      {
        BOOST_VERIFY(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))));
        debugInterface->EnableDebugLayer();

        BOOST_VERIFY(SUCCEEDED(debugInterface->QueryInterface(IID_PPV_ARGS(&debugInterface1))));
        debugInterface1->SetEnableGPUBasedValidation(TRUE);
      }

      const D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_12_1;

      ComPtr<ID3D12Device2> d3d12Device2;
      BOOST_VERIFY(SUCCEEDED(D3D12CreateDevice(dxgiAdapter4.Get(), featureLevel, IID_PPV_ARGS(&d3d12Device2))));

      return std::make_unique<Device>(std::move(d3d12Device2), dxgiAdapter4, isDebugDevice, featureLevel);
    }

    std::unique_ptr<core::Swapchain> CreateSwapchain(const sdl::Window& window, int numBuffers,
      const core::CommandQueue& commandQueue, core::Device& device) const override final
    {
      const CommandQueue& queue = dynamic_cast<const CommandQueue&>(commandQueue);
      ComPtr<ID3D12CommandQueue> d3d12Queue = queue.GetD3d12CommandQueue();

      SDL_SysWMinfo winInfo = window.GetWindowSystemInfo();
      HWND hWnd = winInfo.info.win.window;

      ComPtr<IDXGISwapChain4> dxgiSwapchain4 = factory.CreateSwapChainForWindow(hWnd, numBuffers, d3d12Queue);

      factory.MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

      dx12::Device& dx12Device = dynamic_cast<dx12::Device&>(device);

      return std::make_unique<Swapchain>(std::move(dxgiSwapchain4),
        dx12Device.GetD3d12Device2(), numBuffers, dx12Device.GetDescriptorAllocator());
    }

  private:
    static DXGI_ADAPTER_DESC1 GetDxgiAdapterDesc1(const ComPtr<IDXGIAdapter1>& dxgiAdapter1)
    {
      DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
      BOOST_VERIFY(SUCCEEDED(dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1)));

      return dxgiAdapterDesc1;
    }

    Factory factory;
    ComPtr<ID3D12Debug> debugInterface;
    ComPtr<ID3D12Debug1> debugInterface1;
  };
}
