module;

#include <memory>
#include <array>

#include <d3d12.h>
#include <pix3.h>

export module inline_dxr_test;

import engine.os.sdl.window;
import engine.render.core.dx12.base;
import engine.render.core.dx12.shader_compiler;
import engine.render.core.resource;
import engine.geometry.heightmap_regular_grid;

export void RunInlineDxrTest()
{
  //auto dll = PIXLoadLatestWinPixGpuCapturerLibrary();
  //PIXCaptureParameters params = {};
  //params.GpuCaptureParameters.FileName = L"Capture.wpix";
  //if (!SUCCEEDED(PIXBeginCapture(PIX_CAPTURE_GPU, &params)))
  //{
  //  return;
  //};

  sdl::Window wnd("Engn", 800, 600);
  dx12::Base base(true);
  std::unique_ptr<core::Device> device{base.CreateDevice(1, true)};
  std::unique_ptr<core::CommandQueue> commandQueue{device->CreateCommandQueue(core::CommandBuffer::TYPE_DIRECT,
    core::CommandQueue::PRIORITY_NORMAL, false)};
  std::unique_ptr<core::Swapchain> swapchain{base.CreateSwapchain(wnd, 2, *commandQueue, *device)};
  std::unique_ptr<core::CommandBundle> commandBundle{device->CreateCommandBundle(core::CommandBuffer::TYPE_DIRECT)};
  dx12::ShaderCompiler shaderCompiler{};
  std::unique_ptr<core::ShaderBytecode> rts{shaderCompiler.LoadShaderBytecodeFromFile("shaders-build/InlineRtShader.cso")};
  std::array<core::Device::DescriptorRange, 1> ranges;
  ranges[0].type = core::Descriptor::RANGE_TYPE_UAV;
  ranges[0].baseRegister = 0;
  ranges[0].count = 1;
  std::unique_ptr<core::PipelineLayout> pipelineLayout{device->CreatePipelineLayoutWithTlas(ranges)};
  std::unique_ptr<core::Pso> pso{device->CreateComputePso(*pipelineLayout, *rts)};
  std::unique_ptr<core::ResourceAllocator> resourceAllocator{device->CreateResourceAllocator()};
  std::unique_ptr<core::DescriptorPool> descriptorPool{device->CreateDescriptorPool(core::Descriptor::HEAP_TYPE_CBV_SRV_UAV, (int)ranges.size())};
  std::unique_ptr<core::Fence> fence{device->CreateFence()};

  constexpr int sizeX = 100;
  constexpr int sizeY = 100;
  const geometry::HeightmapRegularGrid grid{sizeX, sizeY, 0.01f, 0.01f, std::filesystem::path("heightmaps") / "height.png"};
  
  std::unique_ptr<core::Buffer> vertexBuffer{resourceAllocator->CreateBuffer(grid.GetVertexDataSize(), false)};
  std::unique_ptr<core::Buffer> uploadVertexBuffer{resourceAllocator->CreateBuffer(grid.GetVertexDataSize(), true)};
  void* mappedMemory = uploadVertexBuffer->Map(std::pair<size_t, size_t>(0, grid.GetVertexDataSize()));
  memcpy(mappedMemory, grid.GetVertexData(), grid.GetVertexDataSize());
  uploadVertexBuffer->Unmap();

  std::unique_ptr<core::Buffer> indexBuffer{resourceAllocator->CreateBuffer(grid.GetIndexDataSize(), false)};
  std::unique_ptr<core::Buffer> uploadIndexBuffer{resourceAllocator->CreateBuffer(grid.GetIndexDataSize(), true)};
  mappedMemory = uploadIndexBuffer->Map(std::pair<size_t, size_t>(0, grid.GetIndexDataSize()));
  memcpy(mappedMemory, grid.GetIndexData(), grid.GetIndexDataSize());
  uploadIndexBuffer->Unmap();

  commandBundle->Begin();

  commandBundle->Barrier(*vertexBuffer, core::Resource::STATE_COPY_DST);
  commandBundle->CopyBufferRegion(*vertexBuffer, 0, *uploadVertexBuffer, 0, grid.GetVertexDataSize());
  commandBundle->Barrier(*vertexBuffer, core::Resource::STATE_COMMON);

  commandBundle->Barrier(*indexBuffer, core::Resource::STATE_COPY_DST);
  commandBundle->CopyBufferRegion(*indexBuffer, 0, *uploadIndexBuffer, 0, grid.GetIndexDataSize());
  commandBundle->Barrier(*indexBuffer, core::Resource::STATE_COMMON);
  
  std::unique_ptr<core::TlasBuilder> tlasBuilder{device->CreateTlasBuilder(*resourceAllocator)};
  tlasBuilder->CreateResourcesAndWriteBuildCommands(*indexBuffer, grid.GetNumIndices(), *vertexBuffer, grid.GetNumVertices(), *commandBundle);

  std::unique_ptr<core::Image> image{resourceAllocator->CreateImage(wnd.GetWidth(), wnd.GetHeight(), true)};
  std::unique_ptr<core::ImageUav> imageUav{device->CreateImageUav(*image)};

  device->WriteImageUavToPool(*descriptorPool, *imageUav, 0);
  
  commandBundle->BindComputePso(*pso);
  commandBundle->SetDescriptorPoolAndTlas(*descriptorPool, tlasBuilder->GetTlas());
  
  commandBundle->Dispatch(wnd.GetWidth(), wnd.GetHeight(), 1);

  commandBundle->Barrier(swapchain->GetCurrentBackbufferResource(), core::Resource::STATE_COPY_DST);
  commandBundle->Barrier(*image, core::Resource::STATE_COPY_SRC);

  commandBundle->CopyResource(*image, swapchain->GetCurrentBackbufferResource());

  commandBundle->Barrier(swapchain->GetCurrentBackbufferResource(), core::Resource::STATE_PRESENT);

  commandBundle->End();

  commandQueue->Submit(*commandBundle, *fence);

  swapchain->Present();

  fence->Wait();

  //PIXEndCapture(false);
  //FreeLibrary(dll);

  while (wnd.ProcessEvents())
    ;

  commandQueue->WaitIdle();
}
