module;

#include <utility>

#include <d3d12.h>
#include <wrl.h>

export module engine.render.core.dx12.rtv;

import engine.render.core.rtv;
import engine.render.core.dx12.descriptor;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Rtv final : public core::Rtv
  {
  public:
    Rtv(const Descriptor& rtvHandle) :
      rtvHandle(rtvHandle)
    {
    }

    const Descriptor& GetDescriptor() const
    {
      return rtvHandle;
    }

  private:
    const Descriptor& rtvHandle;
  };
}
