module;

#include <vector>
#include <memory>
#include <algorithm>
#include <ranges>

#include <boost/assert.hpp>

export module engine.render.core.dx12.descriptor_special_allocator;

import engine.render.core.dx12.descriptor;
import engine.render.core.dx12.descriptor_pool;
import engine.render.core.dx12.descriptor_heap_builder;

namespace dx12
{
  export class DescriptorSpecialAllocator final
  {
  public:
    DescriptorSpecialAllocator(const DescriptorHeapBuilder& descriptorHeapBuilder, Descriptor::HEAP_TYPE type, int poolSize) :
      descriptorHeapBuilder(descriptorHeapBuilder), type(type), poolSize(poolSize)
    {
      BOOST_ASSERT(poolSize > 0);
    }

    int Allocate()
    {
      ProvideChunkAllocationIfNeeded();

      int chunkId = availableChunks.back();
      Chunk& chunk = chunks[chunkId];
      int index = chunk.Allocate();

      if (chunk.IsBusy())
        availableChunks.pop_back();

      return CombineChunkAndIndex(chunkId, index);
    }

    void Free(int globalId)
    {
      auto [chunkId, index] = SplitChunkAndIndex(globalId);
      BOOST_ASSERT(chunkId < chunks.size() && index < poolSize);
      Chunk& chunk = chunks[chunkId];

      if (chunk.IsBusy())
        availableChunks.push_back(chunkId);

      chunk.Free(index);
    }

    const Descriptor& GetDescriptor(int globalId) const noexcept
    {
      auto [chunkId, index] = SplitChunkAndIndex(globalId);
      BOOST_ASSERT(chunkId < chunks.size() && index < poolSize);
      const Chunk& chunk = chunks[chunkId];
      
      return chunk.GetDescriptor(index);
    }

  private:
    int CombineChunkAndIndex(int chunkId, int index) const noexcept
    {
      BOOST_ASSERT(chunkId < chunks.size() && index < poolSize);
      return chunkId * poolSize + index;
    }

    std::pair<int, int> SplitChunkAndIndex(int globalId) const noexcept
    {
      return {globalId / poolSize, globalId % poolSize};
    }

    void ProvideChunkAllocationIfNeeded()
    {
      if (availableChunks.empty())
      {
        std::unique_ptr<DescriptorPool> newPool = descriptorHeapBuilder.CreateDescriptorHeap(type, poolSize, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
        availableChunks.push_back(static_cast<int>(chunks.size()));
        chunks.emplace_back(std::move(newPool));
      }
    }

    class Chunk
    {
    public:
      Chunk(std::unique_ptr<DescriptorPool>&& descriptorPool) : pool(std::move(descriptorPool))
      {
        int poolSize = pool->GetSize();
        freeList.reserve(poolSize);
        std::ranges::copy(std::ranges::iota_view(0, poolSize), std::back_inserter(freeList));
      }
       
      bool IsBusy() const noexcept
      {
        return freeList.empty();
      }

      int Allocate()
      {
        BOOST_ASSERT(!IsBusy());
        int index = freeList.back();
        freeList.pop_back();
        return index;
      }

      void Free(int index)
      {
        freeList.push_back(index);
      }

      const Descriptor& GetDescriptor(int index) const noexcept
      {
        return pool->GetDescriptor(index);
      }

    private:
      std::unique_ptr<DescriptorPool> pool;
      std::vector<int> freeList;
    };

    const DescriptorHeapBuilder& descriptorHeapBuilder;
    std::vector<Chunk> chunks;
    std::vector<int> availableChunks;
    int poolSize;
    Descriptor::HEAP_TYPE type;
  };
}
