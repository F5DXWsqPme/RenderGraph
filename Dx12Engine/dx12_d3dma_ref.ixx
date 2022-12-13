module;

#include <utility>

#include <D3D12MemAlloc.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.d3dma_ref;

import utils.noncopiable;

namespace dx12
{
  export template <class Type>
  class D3dmaRef final : private utils::Noncopiable
  {
  public:
    D3dmaRef() = default;

    D3dmaRef(Type* d3dmaAllocation) :
      d3dmaData(d3dmaAllocation)
    {
    }

    D3dmaRef& operator=(D3dmaRef&& allocation)
    {
      std::swap(d3dmaData, allocation.d3dmaData);
      return *this;
    }

    D3dmaRef(D3dmaRef&& allocation)
    {
      std::swap(d3dmaData, allocation.d3dmaData);
    }

    ~D3dmaRef()
    {
      if (d3dmaData != nullptr)
      {
        d3dmaData->Release();
      }
    }

    Type* operator->() const
    {
      BOOST_ASSERT(d3dmaData != nullptr);
      return d3dmaData;
    }

  private:
    Type* d3dmaData = nullptr;
  };
}
