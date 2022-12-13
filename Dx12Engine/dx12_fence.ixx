module;

#include <utility>
#include <chrono>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.fence;

import engine.render.core.fence;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Fence final : public core::Fence
  {
  public:
    Fence(ComPtr<ID3D12Fence>&& fence) :
      fence(std::move(fence))
    {
      fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    }

    bool Wait(const std::chrono::nanoseconds& maxWaitTime = (std::chrono::nanoseconds::max)()) const override final
    {
      BOOST_ASSERT(valueForWait);

      if (fence->GetCompletedValue() < *valueForWait)
      {
        BOOST_VERIFY(SUCCEEDED(fence->SetEventOnCompletion(*valueForWait, fenceEvent)));

        const std::chrono::milliseconds maxWaitTimeInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(maxWaitTime);
        DWORD result = WaitForSingleObject(fenceEvent, (DWORD)maxWaitTimeInMilliseconds.count());
        
        if (result == WAIT_TIMEOUT) {
          return false;
        }
        BOOST_ASSERT(result == WAIT_OBJECT_0);
      }

      return true;
    }

    void Reset() override final
    {
      valueForWait.reset();
    }

    const ComPtr<ID3D12Fence>& GetD3d12Fence() const
    {
      return fence;
    }

    void SetValueForWait(uint64_t newValueForWait)
    {
      BOOST_ASSERT(!valueForWait);
      valueForWait = newValueForWait;
    }

    ~Fence()
    {
      CloseHandle(fenceEvent);
    }

  private:
    ComPtr<ID3D12Fence> fence;
    HANDLE fenceEvent;
    std::optional<uint64_t> valueForWait;
  };
}
