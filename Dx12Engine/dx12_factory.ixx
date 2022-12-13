module;

#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.factory;

import utils.noncopiable;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Factory final : private utils::Noncopiable
  {
  public:
    Factory(bool createDebugFactory)
    {
      unsigned int createFlags = 0;

      if (createDebugFactory)
        createFlags |= DXGI_CREATE_FACTORY_DEBUG;

      BOOST_VERIFY(SUCCEEDED(CreateDXGIFactory2(createFlags, IID_PPV_ARGS(&dxgiFactory4))));
    }

    ComPtr<IDXGIFactory4> GetDxgiFactory4() const
    {
      return dxgiFactory4;
    }

    ComPtr<IDXGISwapChain4> CreateSwapChainForWindow(
      HWND hWnd, int numBuffers, const ComPtr<ID3D12CommandQueue>& commandQueue, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
      DXGI_ALPHA_MODE alphaMode = DXGI_ALPHA_MODE_IGNORE, DXGI_SWAP_EFFECT swapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
      DXGI_USAGE bufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, int width = 0, int height = 0,
      unsigned int flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING, bool stereo = false) const
    {
      DXGI_SWAP_CHAIN_DESC1 desc = {};
      desc.Width = width;
      desc.Height = height;
      desc.Format = format;
      desc.Stereo = stereo;
      desc.SampleDesc = {1, 0};
      desc.BufferUsage = bufferUsage;
      desc.BufferCount = numBuffers;
      desc.Scaling = DXGI_SCALING_NONE;
      desc.SwapEffect = swapEffect;
      desc.AlphaMode = alphaMode;
      if (!GetTearingSupport())
        flags &= ~DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
      desc.Flags = flags;

      ComPtr<IDXGISwapChain1> swapChain1;
      BOOST_VERIFY(SUCCEEDED(dxgiFactory4->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1)));

      ComPtr<IDXGISwapChain4> swapChain4;
      BOOST_VERIFY(SUCCEEDED(swapChain1.As(&swapChain4)));

      return swapChain4;
    }

    void MakeWindowAssociation(HWND hWnd, unsigned int flags) const
    {
      BOOST_VERIFY(SUCCEEDED(dxgiFactory4->MakeWindowAssociation(hWnd, flags)));
    }

    bool GetTearingSupport() const
    {
      ComPtr<IDXGIFactory5> dxgiFactory5;
      BOOST_VERIFY(SUCCEEDED(dxgiFactory4.As(&dxgiFactory5)));

      BOOL isTearingSupported = FALSE;
      BOOST_VERIFY(SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
        &isTearingSupported, sizeof(isTearingSupported))));

      return isTearingSupported == TRUE;
    }

  private:
    ComPtr<IDXGIFactory4> dxgiFactory4;
  };
}
