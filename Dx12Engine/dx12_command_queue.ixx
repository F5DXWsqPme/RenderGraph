module;

#include <optional>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.command_queue;

import engine.render.core.command_queue;
import engine.render.core.fence;
import engine.render.core.dx12.command_bundle;
import engine.render.core.dx12.fence;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class CommandQueue final : public core::CommandQueue
  {
  public:
    CommandQueue(ComPtr<ID3D12CommandQueue>&& d3d12CommandQueue, ComPtr<ID3D12Fence>&& internalFence) noexcept :
      d3d12CommandQueue(d3d12CommandQueue), internalFence(std::move(internalFence))
    {
    }

    ComPtr<ID3D12CommandQueue> GetD3d12CommandQueue() const
    {
      return d3d12CommandQueue;
    }

    static D3D12_COMMAND_QUEUE_PRIORITY GetD3d12Priority(PRIORITY priority) noexcept
    {
      switch (priority)
      {
      case PRIORITY_NORMAL:
        return D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      case PRIORITY_HIGH:
        return D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
      case PRIORITY_MAX:
        return D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
      default:
        BOOST_ASSERT_MSG(false, "not implemented");
      }

      std::unreachable();
    }

    void Submit(const core::CommandBundle& bundle, core::Fence& fence) override final
    {
      const CommandBundle* dx12Bundle = dynamic_cast<const CommandBundle*>(&bundle);
      BOOST_ASSERT(dx12Bundle != nullptr);
      ID3D12CommandList* d3d12CommandList = dx12Bundle->GetD3d12CommandList().Get();

      d3d12CommandQueue->ExecuteCommandLists(1, &d3d12CommandList);

      Fence* dx12Fence = dynamic_cast<Fence*>(&fence);
      BOOST_ASSERT(dx12Fence != nullptr);
      SignalFence(dx12Fence);
    }

    void WaitIdle() override final
    {
      SignalFence(&internalFence);
      internalFence.Wait();
      internalFence.Reset();
    }

  private:
    void SignalFence(Fence* dx12Fence)
    {
      lastSignaledMark++;
      d3d12CommandQueue->Signal(dx12Fence->GetD3d12Fence().Get(), lastSignaledMark);
      dx12Fence->SetValueForWait(lastSignaledMark);
    }

    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;
    Fence internalFence;
    uint64_t lastSignaledMark;
  };
}
