module;

#include <memory>

export module engine.render.core.resource_allocator;

import engine.render.core.buffer;
import engine.render.core.image;

namespace core
{
  export class ResourceAllocator
  {
  public:
    virtual ~ResourceAllocator() = default;

    virtual std::unique_ptr<Buffer> CreateBuffer(size_t size, bool isUpload, bool needUav = false) const = 0;

    virtual std::unique_ptr<Image> CreateImage(size_t width, size_t height, bool needUav = false) const = 0;
  };
}
