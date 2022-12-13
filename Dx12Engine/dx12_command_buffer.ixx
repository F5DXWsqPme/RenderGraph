module;

#include <utility>
#include <optional>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.command_buffer;

import engine.render.core.command_buffer;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class CommandBuffer final : public core::CommandBuffer
  {
  public:
    CommandBuffer(const ComPtr<ID3D12Device2>& d3d12Device2, const ComPtr<ID3D12CommandAllocator>& d3d12CommandAllocator, D3D12_COMMAND_LIST_TYPE d3d12Type)
    {
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateCommandList(0, d3d12Type, d3d12CommandAllocator.Get(),
        nullptr, IID_PPV_ARGS(&graphicsCommandList))));

      BOOST_VERIFY(SUCCEEDED(graphicsCommandList->Close()));
    }

    static D3D12_COMMAND_LIST_TYPE GetD3d12Type(TYPE type) noexcept
    {
      switch (type)
      {
      case TYPE_DIRECT:
        return D3D12_COMMAND_LIST_TYPE_DIRECT;
      case TYPE_COPY:
        return D3D12_COMMAND_LIST_TYPE_COPY;
      case TYPE_COMPUTE:
        return D3D12_COMMAND_LIST_TYPE_COMPUTE;
      default:
        BOOST_ASSERT_MSG(false, "not implemented");
      }

      std::unreachable();
    }

    const ComPtr<ID3D12GraphicsCommandList>& GetD3d12CommandList() const
    {
      return graphicsCommandList;
    }

    const ComPtr<ID3D12GraphicsCommandList4>& GetD3d12CommandList4()
    {
      if (!graphicsCommandList4Opt)
      {
        ComPtr<ID3D12GraphicsCommandList4> graphicsCommandList4;
        BOOST_VERIFY(SUCCEEDED(graphicsCommandList->QueryInterface(IID_PPV_ARGS(&graphicsCommandList4))));
        graphicsCommandList4Opt = graphicsCommandList4;
      }

      return *graphicsCommandList4Opt;
    }

  private:
    ComPtr<ID3D12GraphicsCommandList> graphicsCommandList;
    std::optional<ComPtr<ID3D12GraphicsCommandList4>> graphicsCommandList4Opt;
  };
}
