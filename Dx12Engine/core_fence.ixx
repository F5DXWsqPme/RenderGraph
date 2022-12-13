module;

#include <chrono>

export module engine.render.core.fence;

namespace core
{
  export class Fence
  {
  public:
    virtual ~Fence() = default;

    virtual bool Wait(const std::chrono::nanoseconds& maxWaitTime = (std::chrono::nanoseconds::max)()) const = 0;

    virtual void Reset() = 0;
  };
}
