module;

#include <utility>
#include <memory>

export module engine.render.core.dx12.shaders_table;

import engine.render.core.shaders_table;
import engine.render.core.buffer;

namespace dx12
{
  export class ShadersTable final : public core::ShadersTable
  {
  public:
    ShadersTable(std::unique_ptr<core::Buffer>&& raygen, std::unique_ptr<core::Buffer>&& hit, std::unique_ptr<core::Buffer>&& miss) :
      raygen(std::move(raygen)),
      hit(std::move(hit)),
      miss(std::move(miss))
    {
    }

    core::Buffer& GetRaygen() const
    {
      return *raygen;
    }

    core::Buffer& GetHit() const
    {
      return *hit;
    }

    core::Buffer& GetMiss() const
    {
      return *miss;
    }

  private:
    std::unique_ptr<core::Buffer> raygen;
    std::unique_ptr<core::Buffer> hit;
    std::unique_ptr<core::Buffer> miss;
  };
}
