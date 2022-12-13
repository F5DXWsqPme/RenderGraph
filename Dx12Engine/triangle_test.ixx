module;

#include <memory>
#include <array>

#include <d3d12.h>
#include <pix3.h>

export module triangle_test;

import engine.os.sdl.window;
import engine.render.core.dx12.base;
import engine.render.core.dx12.shader_compiler;
import engine.render.core.resource;


export void RunTriangleTest()
{
  // auto dll = PIXLoadLatestWinPixGpuCapturerLibrary();
  // PIXCaptureParameters params = {};
  // params.GpuCaptureParameters.FileName = L"C:/oleg/Capture.wpix";
  // if (!SUCCEEDED(PIXBeginCapture(PIX_CAPTURE_GPU, &params)))
  // {
  //   return 0;
  // };

  sdl::Window wnd("Engn", 800, 600);
  dx12::Base base(true);
  std::unique_ptr<core::Device> device{base.CreateDevice(0, true)};
  std::unique_ptr<core::CommandQueue> commandQueue{device->CreateCommandQueue(core::CommandBuffer::TYPE_DIRECT,
    core::CommandQueue::PRIORITY_NORMAL, false)};
  std::unique_ptr<core::Swapchain> swapchain{base.CreateSwapchain(wnd, 2, *commandQueue, *device)};
  std::unique_ptr<core::CommandBundle> commandBundle{device->CreateCommandBundle(core::CommandBuffer::TYPE_DIRECT)};
  dx12::ShaderCompiler shaderCompiler{};
  //std::unique_ptr<core::ShaderBytecode> vs{shaderCompiler.CompileToBytecodeFromFile("shaders/VertexShader.hlsl", L"vs_6_0")};
  //std::unique_ptr<core::ShaderBytecode> ps{shaderCompiler.CompileToBytecodeFromFile("shaders/PixelShader.hlsl", L"ps_6_0")};
  std::unique_ptr<core::ShaderBytecode> vs{shaderCompiler.LoadShaderBytecodeFromFile("shaders-build/VertexShader.cso")};
  std::unique_ptr<core::ShaderBytecode> ps{shaderCompiler.LoadShaderBytecodeFromFile("shaders-build/PixelShader.cso")};
  std::unique_ptr<core::PipelineLayout> pipelineLayout{device->CreatePipelineLayout()};
  std::unique_ptr<core::Pso> pso{device->CreateGraphicsPso(*pipelineLayout, *vs, *ps)};
  std::unique_ptr<core::ResourceAllocator> resourceAllocator{device->CreateResourceAllocator()};
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
  std::unique_ptr<core::Buffer> uploadBuffer{resourceAllocator->CreateBuffer(sizeof(vertexData), true)};
  void* mappedMemory = uploadBuffer->Map(std::pair<size_t, size_t>(0, sizeof(vertexData)));
  memcpy(mappedMemory, vertexData, sizeof(vertexData));
  uploadBuffer->Unmap();

  commandBundle->Begin();
  commandBundle->Barrier(*vertexBuffer, core::Resource::STATE_COPY_DST);
  commandBundle->CopyBufferRegion(*vertexBuffer, 0, *uploadBuffer, 0, sizeof(vertexData));
  commandBundle->Barrier(*vertexBuffer, core::Resource::STATE_COMMON);

  std::unique_ptr<core::VertexBufferView> vertexBufferView{vertexBuffer->CreateVertexBufferView(
    sizeof(vertexData), 0, sizeof(vertexData) / 3)};

  commandBundle->Barrier(swapchain->GetCurrentBackbufferResource(), core::Resource::STATE_RTV);

  commandBundle->ClearRtv(swapchain->GetCurrentBackbufferRtv(), {0.1f, 0.1f, 0.1f, 1.f});

  commandBundle->BindVertexBuffer(*vertexBufferView);
  commandBundle->BindGraphicsPso(*pso);
  commandBundle->SetViewport(0, 1, 0, 0, (float)wnd.GetWidth(), (float)wnd.GetHeight());
  commandBundle->SetScissorRect(0, 0, wnd.GetWidth(), wnd.GetHeight());
  commandBundle->SetRtv(swapchain->GetCurrentBackbufferRtv());

  commandBundle->DrawInstanced(3, 0, 1, 0);

  commandBundle->Barrier(swapchain->GetCurrentBackbufferResource(), core::Resource::STATE_PRESENT);

  commandBundle->End();

  commandQueue->Submit(*commandBundle, *fence);

  swapchain->Present();

  fence->Wait();

  // PIXEndCapture(false);
  // FreeLibrary(dll);

  while (wnd.ProcessEvents())
    ;

  commandQueue->WaitIdle();
}
