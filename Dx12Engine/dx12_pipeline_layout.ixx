module;

#include <utility>

#include <d3d12.h>
#include <wrl.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.pipeline_layout;

import engine.render.core.pipeline_layout;

namespace dx12
{
  using namespace Microsoft::WRL;

  export class PipelineLayout final : public core::PipelineLayout
  {
  public:
    PipelineLayout(ComPtr<ID3D12RootSignature>&& rootSignature) :
      rootSignature(std::move(rootSignature))
    {
    }

    ComPtr<ID3D12RootSignature> GetD3d12RootSignature() const
    {
      return rootSignature;
    }

  private:
    ComPtr<ID3D12RootSignature> rootSignature;
  };
}
