module;

#include <memory>
#include <array>

#include <d3d12.h>
#include <pix3.h>

export module dxr_test;

import engine.os.sdl.window;
import engine.render.core.dx12.base;
import engine.render.core.dx12.shader_compiler;
import engine.render.core.resource;


export void RunDxrTest()
{
  sdl::Window wnd("Engn", 800, 600);
  dx12::Base base(true);
  std::unique_ptr<core::Device> device{base.CreateDevice(1, true)};
  std::unique_ptr<core::CommandQueue> commandQueue{device->CreateCommandQueue(core::CommandBuffer::TYPE_DIRECT,
    core::CommandQueue::PRIORITY_NORMAL, false)};
  std::unique_ptr<core::Swapchain> swapchain{base.CreateSwapchain(wnd, 2, *commandQueue, *device)};
  std::unique_ptr<core::CommandBundle> commandBundle{device->CreateCommandBundle(core::CommandBuffer::TYPE_DIRECT)};
  dx12::ShaderCompiler shaderCompiler{};
  std::unique_ptr<core::ShaderBytecode> rts{shaderCompiler.LoadShaderBytecodeFromFile("shaders-build/RtShader.cso")};
  std::array<core::Device::DescriptorRange, 1> ranges;
  ranges[0].type = core::Descriptor::RANGE_TYPE_UAV;
  ranges[0].baseRegister = 0;
  ranges[0].count = 1;
  std::unique_ptr<core::PipelineLayout> pipelineLayout{device->CreatePipelineLayoutWithTlas(ranges)};
  std::unique_ptr<core::Pso> pso{device->CreateRtPso(*pipelineLayout, *rts, sizeof(float) * 4, 1)};
  std::unique_ptr<core::ResourceAllocator> resourceAllocator{device->CreateResourceAllocator()};
  std::unique_ptr<core::DescriptorPool> descriptorPool{device->CreateDescriptorPool(core::Descriptor::HEAP_TYPE_CBV_SRV_UAV, (int)ranges.size())};
  std::unique_ptr<core::Fence> fence{device->CreateFence()};

  const float vertexData[] =
  {
    -0.5f, -0.5f, 0.1f,
    1, 0, 0,
    0, 0.5f, 0.1f,
    0, 1, 0,
    0.5f, -0.5f, 0.1f,
    0, 0, 1
  };
  std::unique_ptr<core::Buffer> vertexBuffer{resourceAllocator->CreateBuffer(sizeof(vertexData), false)};
  std::unique_ptr<core::Buffer> uploadVertexBuffer{resourceAllocator->CreateBuffer(sizeof(vertexData), true)};
  void* mappedMemory = uploadVertexBuffer->Map(std::pair<size_t, size_t>(0, sizeof(vertexData)));
  memcpy(mappedMemory, vertexData, sizeof(vertexData));
  uploadVertexBuffer->Unmap();

  const uint32_t indexData[] =
  {
    0, 1, 2
  };
  std::unique_ptr<core::Buffer> indexBuffer{resourceAllocator->CreateBuffer(sizeof(indexData), false)};
  std::unique_ptr<core::Buffer> uploadIndexBuffer{resourceAllocator->CreateBuffer(sizeof(indexData), true)};
  mappedMemory = uploadIndexBuffer->Map(std::pair<size_t, size_t>(0, sizeof(indexData)));
  memcpy(mappedMemory, indexData, sizeof(indexData));
  uploadIndexBuffer->Unmap();

  commandBundle->Begin();

  commandBundle->Barrier(*vertexBuffer, core::Resource::STATE_COPY_DST);
  commandBundle->CopyBufferRegion(*vertexBuffer, 0, *uploadVertexBuffer, 0, sizeof(vertexData));
  commandBundle->Barrier(*vertexBuffer, core::Resource::STATE_COMMON);

  commandBundle->Barrier(*indexBuffer, core::Resource::STATE_COPY_DST);
  commandBundle->CopyBufferRegion(*indexBuffer, 0, *uploadIndexBuffer, 0, sizeof(indexData));
  commandBundle->Barrier(*indexBuffer, core::Resource::STATE_COMMON);
  
  std::unique_ptr<core::TlasBuilder> tlasBuilder{device->CreateTlasBuilder(*resourceAllocator)};
  tlasBuilder->CreateResourcesAndWriteBuildCommands(*indexBuffer, 3, *vertexBuffer, 3, *commandBundle);

  std::unique_ptr<core::ShadersTable> shadersTable{pso->CreateShadersTable(*resourceAllocator)};

  std::unique_ptr<core::Image> image{resourceAllocator->CreateImage(wnd.GetWidth(), wnd.GetHeight(), true)};
  std::unique_ptr<core::ImageUav> imageUav{device->CreateImageUav(*image)};

  device->WriteImageUavToPool(*descriptorPool, *imageUav, 0);
  
  commandBundle->BindRtPso(*pso);
  commandBundle->SetDescriptorPoolAndTlas(*descriptorPool, tlasBuilder->GetTlas());
  
  commandBundle->DispatchRays(*shadersTable, wnd.GetWidth(), wnd.GetHeight(), 1);

  commandBundle->End();

  commandQueue->Submit(*commandBundle, *fence);

  swapchain->Present();

  fence->Wait();

  while (wnd.ProcessEvents())
    ;

  commandQueue->WaitIdle();
}
