module;

#include <utility>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.resource_holder;

import engine.render.core.resource;
import engine.render.core.resource_holder;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class ResourceHolder final : public core::ResourceHolder, public core::Resource
  {
  public:
    ResourceHolder(ComPtr<ID3D12Resource>&& resource, STATE state) :
      resource(std::move(resource)), state(state)
    {
    }

    const ComPtr<ID3D12Resource>& GetD3d12Resource() const
    {
      return resource;
    }

    D3D12_RESOURCE_STATES GetD3d12State() const
    {
      return ConvertStateToD3d12State(state);
    }

    void SetState(STATE newState) override final
    {
      state = newState;
    }

    STATE GetState() const override final
    {
      return state;
    }

    ResourceHolder* GetResourceHolder() override final
    {
      return this;
    }

    const ResourceHolder* GetResourceHolder() const override final
    {
      return this;
    }

    static D3D12_RESOURCE_STATES ConvertStateToD3d12State(core::Resource::STATE state)
    {
      switch (state)
      {
      case core::Resource::STATE_COMMON:
        return D3D12_RESOURCE_STATE_COMMON;
      case core::Resource::STATE_COPY_DST:
        return D3D12_RESOURCE_STATE_COPY_DEST;
      case core::Resource::STATE_COPY_SRC:
        return D3D12_RESOURCE_STATE_COPY_SOURCE;
      case core::Resource::STATE_PRESENT:
        return D3D12_RESOURCE_STATE_PRESENT;
      case core::Resource::STATE_SRV:
        return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
      case core::Resource::STATE_UAV:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
      case core::Resource::STATE_UPLOAD:
        return D3D12_RESOURCE_STATE_GENERIC_READ;
      case core::Resource::STATE_RTV:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
      case core::Resource::STATE_ACCELERATION_STRUCTURE:
        return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
      default:
        BOOST_ASSERT_MSG(false, "Not implemented");
      }

      std::unreachable();
    }

  private:
    const ComPtr<ID3D12Resource> resource;
    STATE state = STATE_COMMON;
  };
}
