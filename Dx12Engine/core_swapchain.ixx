export module engine.render.core.swapchain;

import engine.render.core.rtv;
import engine.render.core.resource;

namespace core
{
  export class Swapchain
  {
  public:
    virtual ~Swapchain() = default;

    virtual const core::Rtv& GetCurrentBackbufferRtv() = 0;

    virtual core::Resource& GetCurrentBackbufferResource() = 0;

    virtual void Present() = 0;
  };
}
