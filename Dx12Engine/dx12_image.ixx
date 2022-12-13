module;

#include <utility>

#include <wrl.h>
#include <d3d12.h>
#include <D3D12MemAlloc.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.image;

import engine.render.core.image;
import engine.render.core.dx12.d3dma_ref;
import engine.render.core.dx12.resource_holder;
import engine.render.core.dx12.descriptor_ref;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Image final : public core::Image
  {
  public:
    Image(ComPtr<ID3D12Resource>&& buffer, D3dmaRef<D3D12MA::Allocation>&& allocation, Resource::STATE state) :
      resourceHolder(std::move(buffer), state), allocation(std::move(allocation))
    {
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

  private:
    ResourceHolder resourceHolder;
    D3dmaRef<D3D12MA::Allocation> allocation;
    DescriptorRef uavDescriptorRef;
  };
}
