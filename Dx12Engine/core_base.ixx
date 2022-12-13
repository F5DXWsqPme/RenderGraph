module;

#include <vector>
#include <memory>

export module engine.render.core.base;

import engine.render.core.adapter_properties;
import engine.render.core.device;
import engine.render.core.swapchain;
import engine.render.core.command_queue;
import engine.os.sdl.window;

namespace core
{
  export class Base
  {
  public:
    virtual ~Base() = default;

    virtual std::unique_ptr<Device> CreateDevice(unsigned int adapterId, bool isDebugDevice) = 0;

    virtual std::vector<AdapterProperties> GetAdapterProperties() const = 0;

    virtual std::unique_ptr<Swapchain> CreateSwapchain(const sdl::Window& window, int numBuffers,
      const core::CommandQueue& commandQueue, core::Device& device) const = 0;
  };
}
