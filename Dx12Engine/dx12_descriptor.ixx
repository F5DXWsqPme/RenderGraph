module;

#include <utility>

#include <d3d12.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.descriptor;

import engine.render.core.descriptor;
import engine.render.core.rtv;

namespace dx12
{
  export class Descriptor final : public core::Descriptor, public core::Rtv
  {
  public:
    Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) noexcept :
      cpuHandle(cpuHandle), gpuHandle(gpuHandle)
    {
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() const noexcept
    {
      return cpuHandle;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetGpuHandle() const noexcept
    {
      return gpuHandle;
    }

    static D3D12_DESCRIPTOR_HEAP_TYPE GetD3d12Type(HEAP_TYPE type) noexcept
    {
      switch (type)
      {
      case HEAP_TYPE_RTV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      case HEAP_TYPE_CBV_SRV_UAV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      default:
        BOOST_ASSERT_MSG(false, "not implemented");
      }

      std::unreachable();
    }

    static D3D12_DESCRIPTOR_RANGE_TYPE GetD3d12RangeType(RANGE_TYPE type) noexcept
    {
      switch (type)
      {
      case RANGE_TYPE_UAV:
        return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      case RANGE_TYPE_SRV:
        return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      default:
        BOOST_ASSERT_MSG(false, "not implemented");
      }

      std::unreachable();
    }

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
  };
}
