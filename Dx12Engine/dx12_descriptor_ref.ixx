module;

#include <optional>

#include <boost/assert.hpp>

export module engine.render.core.dx12.descriptor_ref;

import engine.render.core.dx12.descriptor_special_allocator;
import engine.render.core.dx12.descriptor;
import utils.noncopiable;

namespace dx12
{
  export class DescriptorRef final : private utils::Noncopiable
  {
  public:
    DescriptorRef() = default;

    DescriptorRef(DescriptorSpecialAllocator& specialAllocator, int index) noexcept :
      specialAllocator(&specialAllocator), index(index)
    {
    }

    DescriptorRef(DescriptorRef&& ref)
    {
      Swap(ref);
    }

    const DescriptorRef& operator=(DescriptorRef&& ref)
    {
      Swap(ref);
      return *this;
    }

    ~DescriptorRef()
    {
      if (index)
      {
        specialAllocator->Free(*index);
      }
    }

    const Descriptor& operator*() const noexcept
    {
      BOOST_ASSERT(index);
      return specialAllocator->GetDescriptor(*index);
    }

    const Descriptor* operator->() const noexcept
    {
      BOOST_ASSERT(index);
      return &specialAllocator->GetDescriptor(*index);
    }

  private:
    void Swap(DescriptorRef& ref)
    {
      std::swap(ref.specialAllocator, specialAllocator);
      std::swap(ref.index, index);
    }

    DescriptorSpecialAllocator* specialAllocator = nullptr;
    std::optional<int> index;
  };
}
