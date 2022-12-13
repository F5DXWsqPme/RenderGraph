module;

#include <d3d12.h>
#include <dxcapi.h>
#include <atlbase.h>
#include <d3d12shader.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.shader_bytecode;

import engine.render.core.shader_bytecode;

namespace dx12
{
  export class ShaderBytecode final : public core::ShaderBytecode
  {
  public:
    ShaderBytecode(const CComPtr<IDxcBlob>& shaderBlob/*, const CComPtr<IDxcBlob>& pdbBlob,
      const CComPtr<ID3D12ShaderReflection>& reflection*/) :
      shaderBlob(shaderBlob)/*, pdbBlob(pdbBlob), reflection(reflection)*/
    {
    }

    D3D12_SHADER_BYTECODE GetDx12ShaderBytecode() const
    {
      D3D12_SHADER_BYTECODE bytecode;

      bytecode.BytecodeLength = shaderBlob->GetBufferSize();
      bytecode.pShaderBytecode = shaderBlob->GetBufferPointer();

      return bytecode;
    }

  private:
    CComPtr<IDxcBlob> shaderBlob;
    /*CComPtr<IDxcBlob> pdbBlob;
    CComPtr<ID3D12ShaderReflection> reflection;*/
  };
}
