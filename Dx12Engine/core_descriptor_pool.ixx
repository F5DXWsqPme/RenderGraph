export module engine.render.core.descriptor_pool;

import engine.render.core.descriptor;

namespace core
{
  export class DescriptorPool
  {
  public:
    virtual ~DescriptorPool() = default;

    virtual int GetSize() const noexcept = 0;
  };
}
