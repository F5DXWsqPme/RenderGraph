module;

#include <map>
#include <memory>

export module engine.render.core.buffer;

import engine.render.core.resource;
import engine.render.core.vertex_buffer_view;

namespace core
{
  export class Buffer : public core::Resource
  {
  public:
    virtual ~Buffer() = default;

    virtual void* Map(const std::pair<size_t, size_t>& range) = 0;
    virtual void Unmap() = 0;

    virtual std::unique_ptr<core::VertexBufferView> CreateVertexBufferView(int size, int offset, int stride) = 0;
  };
}
