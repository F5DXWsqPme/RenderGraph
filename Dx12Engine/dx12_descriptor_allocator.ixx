module;

#include <vector>
#include <memory>

#include <boost/assert.hpp>
#include <boost/container/static_vector.hpp>

export module engine.render.core.dx12.descriptor_allocator;

import engine.render.core.dx12.descriptor_special_allocator;
import engine.render.core.dx12.descriptor;
import engine.render.core.dx12.descriptor_heap_builder;
import engine.render.core.dx12.descriptor_ref;

namespace dx12
{
  namespace bc = boost::container;

  export class DescriptorAllocator final
  {
  public:
    DescriptorAllocator(const DescriptorHeapBuilder& descriptorHeapBuilder)
    {
      for (int typeId = 0; typeId < Descriptor::HEAP_TYPE_COUNT; typeId++)
      {
        Descriptor::HEAP_TYPE type = static_cast<Descriptor::HEAP_TYPE>(typeId);
        int poolSize = GetPoolSize(type);
        specialAllocators.emplace_back(descriptorHeapBuilder, type, poolSize);
      }
    }

    DescriptorRef Allocate(Descriptor::HEAP_TYPE type)
    {
      DescriptorSpecialAllocator& specialAllocator = specialAllocators[type];
      int index = specialAllocator.Allocate();
      return DescriptorRef{specialAllocator, index};
    }

  private:
    static int GetPoolSize(Descriptor::HEAP_TYPE type)
    {
      return 32;
    }

    bc::static_vector<DescriptorSpecialAllocator, Descriptor::HEAP_TYPE_COUNT> specialAllocators;
  };
}
