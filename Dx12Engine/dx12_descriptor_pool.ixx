module;

#include <vector>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.descriptor_pool;

import engine.render.core.descriptor_pool;
import engine.render.core.dx12.descriptor;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class DescriptorPool final : public core::DescriptorPool
  {
  public:
    DescriptorPool(ComPtr<ID3D12DescriptorHeap>&& descriptorPool, int size, int incrementSize, ComPtr<ID3D12Device2> d3d12Device2) :
      d3d12DescriptorPool(std::move(descriptorPool)), d3d12Device2(d3d12Device2)
    {
      descriptors.reserve(size);

      D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = d3d12DescriptorPool->GetCPUDescriptorHandleForHeapStart();
      D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = d3d12DescriptorPool->GetGPUDescriptorHandleForHeapStart();
      
      for (int i = 0; i < size; i++)
      {
        descriptors.emplace_back(cpuHandle, gpuHandle);
        cpuHandle.ptr += incrementSize;
        gpuHandle.ptr += incrementSize;
      }
    }

    const Descriptor& GetDescriptor(int index) const noexcept
    {
      return descriptors[index];
    }

    int GetSize() const noexcept override final
    {
      return static_cast<int>(descriptors.size());
    }

    D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType() const
    {
      return d3d12DescriptorPool->GetDesc().Type;
    }

    const ComPtr<ID3D12DescriptorHeap>& GetDescriptorHeap() const
    {
      return d3d12DescriptorPool;
    }

  private:
    ComPtr<ID3D12DescriptorHeap> d3d12DescriptorPool;
    std::vector<Descriptor> descriptors;
    ComPtr<ID3D12Device2> d3d12Device2;
  };
}
