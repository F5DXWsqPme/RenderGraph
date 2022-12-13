module;

#include <utility>
#include <variant>
#include <memory>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.pso;

import engine.render.core.pso;
import engine.render.core.resource_allocator;
import engine.render.core.buffer;
import engine.render.core.shaders_table;
import engine.render.core.dx12.shaders_table;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class Pso final : public core::Pso
  {
  public:
    Pso(ComPtr<ID3D12PipelineState>&& pso, const ComPtr<ID3D12RootSignature>& rootSignature) :
      pso(std::move(pso)),
      rootSignature(rootSignature)
    {
    }

    Pso(ComPtr<ID3D12StateObject>&& pso, const ComPtr<ID3D12RootSignature>& rootSignature) :
      pso(std::move(pso)),
      rootSignature(rootSignature)
    {
    }

    const ComPtr<ID3D12PipelineState>& GetD3d12Pso() const
    {
      return std::get<ComPtr<ID3D12PipelineState>>(pso);
    }

    const ComPtr<ID3D12StateObject>& GetD3d12RtPso() const
    {
      return std::get<ComPtr<ID3D12StateObject>>(pso);
    }

    std::unique_ptr<core::ShadersTable> CreateShadersTable(core::ResourceAllocator& resourceAllocator) const override final
    {
      ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
      BOOST_VERIFY(SUCCEEDED(GetD3d12RtPso().As(&stateObjectProperties)));

      std::unique_ptr<core::Buffer> raygen{resourceAllocator.CreateBuffer(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, true)};
      {
        void* shaderId = stateObjectProperties->GetShaderIdentifier(raygenShaderName);
        void* table = raygen->Map({0, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES});
        memcpy(table, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        raygen->Unmap();
      }

      std::unique_ptr<core::Buffer> miss{resourceAllocator.CreateBuffer(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, true)};
      {
        void* shaderId = stateObjectProperties->GetShaderIdentifier(missShaderName);
        void* table = miss->Map({0, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES});
        memcpy(table, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        miss->Unmap();
      }

      std::unique_ptr<core::Buffer> hit{resourceAllocator.CreateBuffer(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, true)};
      {
        void* shaderId = stateObjectProperties->GetShaderIdentifier(hitGroupName);
        void* table = hit->Map({0, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES});
        memcpy(table, shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
        hit->Unmap();
      }

      return std::make_unique<ShadersTable>(std::move(raygen), std::move(hit), std::move(miss));
    }

    const ComPtr<ID3D12RootSignature>& GetD3d12RootSignature() const
    {
      return rootSignature;
    }

    static constexpr const wchar_t* closestHitShaderName = L"ClosestHitShader";
    static constexpr const wchar_t* hitGroupName = L"HitGroup";
    static constexpr const wchar_t* missShaderName = L"MissShader";
    static constexpr const wchar_t* raygenShaderName = L"MissShader";

  private:
    std::variant<ComPtr<ID3D12PipelineState>, ComPtr<ID3D12StateObject>> pso;
    ComPtr<ID3D12RootSignature> rootSignature;
  };
}
