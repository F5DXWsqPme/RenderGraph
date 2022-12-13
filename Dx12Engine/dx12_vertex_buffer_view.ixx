module;

#include <utility>
#include <memory>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.vertex_buffer_view;

import engine.render.core.vertex_buffer_view;
import engine.render.core.buffer;
import engine.render.core.dx12.resource_holder;
import engine.render.core.resource_holder;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class VertexBufferView final : public core::VertexBufferView
  {
  public:
    VertexBufferView(core::Buffer& buffer, int size, int offset, int stride)
    {
      core::ResourceHolder* resourceHolder = buffer.GetResourceHolder();
      dx12::ResourceHolder* dx12ResourceHolder = dynamic_cast<dx12::ResourceHolder*>(resourceHolder);
      BOOST_ASSERT(dx12ResourceHolder != nullptr);
      const ComPtr<ID3D12Resource>& resource = dx12ResourceHolder->GetD3d12Resource();
      vertexBufferView.BufferLocation = resource->GetGPUVirtualAddress() + offset;
      vertexBufferView.SizeInBytes = size;
      vertexBufferView.StrideInBytes = stride;
    }

    const D3D12_VERTEX_BUFFER_VIEW* GetDx12View() const noexcept
    {
      return &vertexBufferView;
    }

  private:
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
  };
}
