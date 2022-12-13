export module engine.render.core.command_queue;

import engine.render.core.command_bundle;
import engine.render.core.fence;

namespace core
{
  export class CommandQueue
  {
  public:
    virtual ~CommandQueue() = default;

    enum PRIORITY
    {
      PRIORITY_NORMAL,
      PRIORITY_HIGH,
      PRIORITY_MAX,
      PRIORITY_COUNT
    };

    virtual void Submit(const CommandBundle& bundle, Fence& fence) = 0;

    virtual void WaitIdle() = 0;
  };
}
