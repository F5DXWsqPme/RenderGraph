export module engine.render.core.resource;

import engine.render.core.resource_holder;

namespace core
{
  export class Resource
  {
  public:
    virtual ~Resource() = default;

    enum STATE
    {
      STATE_COMMON = 0,
      STATE_COPY_DST,
      STATE_COPY_SRC,
      STATE_UAV,
      STATE_SRV,
      STATE_PRESENT,
      STATE_UPLOAD,
      STATE_ACCELERATION_STRUCTURE,
      STATE_RTV
    };

    virtual void SetState(STATE newState) = 0;
    virtual STATE GetState() const = 0;

    virtual ResourceHolder* GetResourceHolder() = 0;
    virtual const ResourceHolder* GetResourceHolder() const = 0;
  };
}
