module;

#include <memory>
#include <span>

export module engine.render.core.device;

import engine.render.core.descriptor;
import engine.render.core.descriptor_pool;
import engine.render.core.command_queue;
import engine.render.core.command_buffer;
import engine.render.core.command_bundle;
import engine.render.core.pso;
import engine.render.core.pipeline_layout;
import engine.render.core.shader_bytecode;
import engine.render.core.resource_allocator;
import engine.render.core.fence;
import engine.render.core.tlas_builder;
import engine.render.core.image;
import engine.render.core.image_uav;

namespace core
{
  export class Device
  {
  public:
    virtual ~Device() = default;

    virtual std::unique_ptr<DescriptorPool> CreateDescriptorPool(Descriptor::HEAP_TYPE type, int numDescriptors) const = 0;
    
    virtual std::unique_ptr<CommandQueue> CreateCommandQueue(CommandBuffer::TYPE type, CommandQueue::PRIORITY priority,
      bool tryDisableTimeout) const = 0;

    virtual std::unique_ptr<CommandBundle> CreateCommandBundle(CommandBuffer::TYPE type) const = 0;

    virtual std::unique_ptr<PipelineLayout> CreatePipelineLayout() const = 0;

    virtual std::unique_ptr<Pso> CreateGraphicsPso(const PipelineLayout& pipelineLayout, const ShaderBytecode& vs, const ShaderBytecode& ps) const = 0;
  
    virtual std::unique_ptr<core::Pso> CreateComputePso(const core::PipelineLayout& pipelineLayout, const core::ShaderBytecode& cs) const = 0;

    virtual std::unique_ptr<ResourceAllocator> CreateResourceAllocator() const = 0;

    virtual std::unique_ptr<ImageUav> CreateImageUav(const Image& image) = 0;

    virtual void WriteImageUavToPool(const DescriptorPool& descriptorPool, const ImageUav& uav, uint32_t slot) const = 0;

    virtual std::unique_ptr<Fence> CreateFence() const = 0;

    struct DescriptorRange
    {
      core::Descriptor::RANGE_TYPE type;
      int count;
      int baseRegister;
    };

    virtual std::unique_ptr<core::PipelineLayout> CreatePipelineLayoutWithTlas(const std::span<DescriptorRange>& inputRanges) const = 0;

    virtual std::unique_ptr<core::Pso> CreateRtPso(
      const core::PipelineLayout& pipelineLayout, const core::ShaderBytecode& shader, int payloadSize, int recursionDepth) = 0;

    virtual std::unique_ptr<core::TlasBuilder> CreateTlasBuilder(const core::ResourceAllocator& resourceAllocator) = 0;
  };
}
