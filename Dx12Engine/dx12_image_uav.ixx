module;

#include <utility>

#include <wrl.h>
#include <d3d12.h>
#include <D3D12MemAlloc.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.image_uav;

import engine.render.core.image_uav;
import engine.render.core.dx12.descriptor_ref;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class ImageUav final : public core::ImageUav
  {
  public:
    ImageUav(DescriptorRef&& uav) :
      uav(std::move(uav))
    {
    }

    const DescriptorRef& GetDescriptorRef() const
    {
      return uav;
    }

  private:
    DescriptorRef uav;
  };
}
