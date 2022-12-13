module;

#include <utility>
#include <memory>
#include <optional>

#include <d3d12.h>
#include <wrl.h>
#include <atlbase.h>

#include <D3D12MemAlloc.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.buffer;

import engine.render.core.buffer;
import engine.render.core.dx12.d3dma_ref;
import engine.render.core.dx12.resource_holder;
import engine.render.core.vertex_buffer_view;
import engine.render.core.dx12.vertex_buffer_view;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Buffer final : public core::Buffer
  {
  public:
    Buffer(ComPtr<ID3D12Resource>&& buffer, D3dmaRef<D3D12MA::Allocation>&& allocation, Resource::STATE state) :
      resourceHolder(std::move(buffer), state), allocation(std::move(allocation))
    {
    }

    void* Map(const std::pair<size_t, size_t>& range) override final
    {
      void* mappedPtr = nullptr;
      D3D12_RANGE d3d12Range = {};
      d3d12Range.Begin = range.first;
      d3d12Range.End = range.second;
      BOOST_VERIFY(SUCCEEDED(resourceHolder.GetD3d12Resource()->Map(0, &d3d12Range, &mappedPtr)));

      BOOST_ASSERT(!mappedRange);
      mappedRange = d3d12Range;

      return mappedPtr;
    }

    void Unmap() override final
    {
      BOOST_ASSERT(mappedRange);
      resourceHolder.GetD3d12Resource()->Unmap(0, &*mappedRange);
      mappedRange.reset();
    }

    void SetState(STATE newState) override final
    {
      resourceHolder.SetState(newState);
    }

    STATE GetState() const override final
    {
      return resourceHolder.GetState();
    }

    ResourceHolder* GetResourceHolder() override final
    {
      return &resourceHolder;
    }

    const ResourceHolder* GetResourceHolder() const override final
    {
      return &resourceHolder;
    }

    std::unique_ptr<core::VertexBufferView> CreateVertexBufferView(int size, int offset, int stride) override final
    {
      return std::make_unique<VertexBufferView>(*this, size, offset, stride);
    }

  private:
    ResourceHolder resourceHolder;
    D3dmaRef<D3D12MA::Allocation> allocation;
    std::optional<D3D12_RANGE> mappedRange;
  };
}
