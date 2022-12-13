module;

#include <memory>

#include <dxgi1_6.h>
#include <d3dx12.h>
#include <wrl.h>

#include <D3D12MemAlloc.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.resource_allocator;

import engine.render.core.resource_allocator;
import engine.render.core.resource;
import engine.render.core.dx12.buffer;
import engine.render.core.dx12.image;
import engine.render.core.dx12.d3dma_ref;
import engine.render.core.dx12.resource_holder;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class ResourceAllocator final : public core::ResourceAllocator
  {
  public:
    ResourceAllocator(
      const ComPtr<IDXGIAdapter>& dxgiAdapter, const ComPtr<ID3D12Device2>& d3d12Device2)
    {
      D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
      allocatorDesc.pDevice = d3d12Device2.Get();
      allocatorDesc.pAdapter = dxgiAdapter.Get();
 
      D3D12MA::Allocator* d3dmaAllocator;
      BOOST_VERIFY(SUCCEEDED(D3D12MA::CreateAllocator(&allocatorDesc, &d3dmaAllocator)));
      allocator = d3dmaAllocator;
    }

    std::unique_ptr<core::Buffer> CreateBuffer(
      size_t size, D3D12_RESOURCE_FLAGS flags, core::Resource::STATE initialState, D3D12_HEAP_TYPE heapType) const
    {
      D3D12_RESOURCE_DESC desc;
      desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      desc.Alignment = 0;
      desc.Width = size;
      desc.Height = 1;
      desc.DepthOrArraySize = 1;
      desc.MipLevels = 1;
      desc.Format = DXGI_FORMAT_UNKNOWN;
      desc.SampleDesc.Count = 1;
      desc.SampleDesc.Quality = 0;
      desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      desc.Flags = flags;

      D3D12MA::ALLOCATION_DESC allocDesc;
      allocDesc.HeapType = heapType;
      allocDesc.CustomPool = nullptr;
      allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
      allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;

      D3D12MA::Allocation* d3dmaAllocation = nullptr;
      ComPtr<ID3D12Resource> bufferResource = nullptr;
      BOOST_VERIFY(SUCCEEDED(allocator->CreateResource(&allocDesc, &desc,
        ResourceHolder::ConvertStateToD3d12State(initialState), nullptr,
        &d3dmaAllocation, IID_PPV_ARGS(&bufferResource))));

      return std::make_unique<Buffer>(std::move(bufferResource), d3dmaAllocation, initialState);
    }

    std::unique_ptr<core::Buffer> CreateBuffer(size_t size, bool isUpload, bool needUav = false) const override final
    {
      D3D12_RESOURCE_FLAGS flags = needUav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
      core::Resource::STATE initialState = isUpload ? core::Resource::STATE_UPLOAD : core::Resource::STATE_COMMON;
      D3D12_HEAP_TYPE heapType = isUpload ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;

      return std::move(CreateBuffer(size, flags, initialState, heapType));
    }

    std::unique_ptr<core::Image> CreateImage(
      size_t width, size_t height, bool needUav = false) const override final
    {
      D3D12_RESOURCE_FLAGS flags = needUav ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;
      core::Resource::STATE initialState = needUav ? core::Resource::STATE_UAV : core::Resource::STATE_COMMON;
      D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;

      D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM, width, (unsigned)height, 1, 1, 1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

      D3D12MA::ALLOCATION_DESC allocDesc;
      allocDesc.HeapType = heapType;
      allocDesc.CustomPool = nullptr;
      allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
      allocDesc.Flags = D3D12MA::ALLOCATION_FLAG_NONE;

      D3D12MA::Allocation* d3dmaAllocation = nullptr;
      ComPtr<ID3D12Resource> imageResource = nullptr;
      BOOST_VERIFY(SUCCEEDED(allocator->CreateResource(&allocDesc, &desc,
        ResourceHolder::ConvertStateToD3d12State(initialState), nullptr,
        &d3dmaAllocation, IID_PPV_ARGS(&imageResource))));

      return std::make_unique<Image>(std::move(imageResource), d3dmaAllocation, initialState);
    }

  private:
    D3dmaRef<D3D12MA::Allocator> allocator;
  };
}
