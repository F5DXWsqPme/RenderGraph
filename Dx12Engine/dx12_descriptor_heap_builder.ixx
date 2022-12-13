module;

#include <utility>
#include <memory>
#include <array>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.descriptor_heap_builder;

import engine.render.core.dx12.descriptor_pool;
import utils.noncopiable;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class DescriptorHeapBuilder final : private utils::Noncopiable
  {
  public:
    DescriptorHeapBuilder(const ComPtr<ID3D12Device2>& d3d12Device2) :
      d3d12Device2(d3d12Device2)
    {
      FillDescriptorHandleIncrementSizes();
    }

    std::unique_ptr<DescriptorPool> CreateDescriptorHeap(core::Descriptor::HEAP_TYPE type, int numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS flags) const
    {
      unsigned int nodeMask = 0;

      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.Type = Descriptor::GetD3d12Type(type);
      desc.NumDescriptors = numDescriptors;
      desc.Flags = flags;
      desc.NodeMask = nodeMask;

      ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap;
      BOOST_VERIFY(SUCCEEDED(d3d12Device2->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&d3d12DescriptorHeap))));

      int incrementSize = descriptorHandleIncrementSizes[type];

      return std::make_unique<DescriptorPool>(std::move(d3d12DescriptorHeap), numDescriptors, incrementSize, d3d12Device2);
    }

  private:
    void FillDescriptorHandleIncrementSizes()
    {
      for (int descriptorType = 0; descriptorType < core::Descriptor::HEAP_TYPE_COUNT; descriptorType++)
      {
        D3D12_DESCRIPTOR_HEAP_TYPE d3d12DescriptorType = Descriptor::GetD3d12Type(core::Descriptor::HEAP_TYPE{descriptorType});
        descriptorHandleIncrementSizes[descriptorType] = d3d12Device2->GetDescriptorHandleIncrementSize(d3d12DescriptorType);
      }
    }

    ComPtr<ID3D12Device2> d3d12Device2;
    std::array<int, core::Descriptor::HEAP_TYPE_COUNT> descriptorHandleIncrementSizes;
  };
}
