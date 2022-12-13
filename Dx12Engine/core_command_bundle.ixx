module;

#include <array>

export module engine.render.core.command_bundle;

import engine.render.core.buffer;
import engine.render.core.rtv;
import engine.render.core.resource;
import engine.render.core.vertex_buffer_view;
import engine.render.core.pso;
import engine.render.core.descriptor_pool;
import engine.render.core.shaders_table;

namespace core
{
  export class CommandBundle
  {
  public:
    virtual ~CommandBundle() = default;

    virtual void Begin() = 0;
    virtual void End() = 0;

    virtual void CopyBufferRegion(Buffer& dst, size_t dstOffset, Buffer& src, size_t srcOffset, size_t size) = 0;

    virtual void Barrier(core::Resource& resource, core::Resource::STATE newState) = 0;

    virtual void ClearRtv(const Rtv& rtv, const std::array<float, 4>& value) = 0;

    virtual void BindVertexBuffer(const core::VertexBufferView& vertexBufferView) = 0;

    virtual void BindGraphicsPso(const core::Pso& pso) = 0;

    virtual void BindComputePso(const core::Pso& pso) = 0;

    virtual void Dispatch(size_t width, size_t height, size_t depth) = 0;

    virtual void CopyResource(const core::Resource& src, const core::Resource& dst) = 0;

    virtual void SetViewport(float minDepth, float maxDepth, float offsetX, float offsetY, float sizeX, float sizeY) = 0;

    virtual void SetScissorRect(int offsetX, int offsetY, int sizeX, int sizeY) = 0;

    virtual void SetRtv(const Rtv& rtv) = 0;

    virtual void SetDescriptorPoolAndTlas(const DescriptorPool& pool, const Buffer& tlas) = 0;

    virtual void BindRtPso(const Pso& pso) = 0;

    virtual void DispatchRays(const ShadersTable& shadersTable, size_t width, size_t height, size_t depth) = 0;

    virtual void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t vertexOffset, uint32_t instanceCount, uint32_t instanceOffset) = 0;
  };
}
