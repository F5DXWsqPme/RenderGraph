module;

#include <memory>

export module engine.render.core.pso;

import engine.render.core.resource_allocator;
import engine.render.core.buffer;
import engine.render.core.shaders_table;

namespace core
{
  export class Pso
  {
  public:
    virtual ~Pso() = default;

    virtual std::unique_ptr<core::ShadersTable> CreateShadersTable(ResourceAllocator& resourceAllocator) const = 0;
  };
}
