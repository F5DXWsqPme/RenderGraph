module;

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

#include <combaseapi.h>
#include <dxcapi.h>
#include <atlbase.h>
#include <d3d12shader.h>

#include <boost/assert.hpp>

export module engine.render.core.dx12.shader_compiler;

import engine.render.core.shader_compiler;
import engine.render.core.dx12.shader_bytecode;

namespace dx12
{
  export class ShaderCompiler final : public core::ShaderCompiler
  {
  public:
    ShaderCompiler()
    {
      DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
      DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
      dxcUtils->CreateDefaultIncludeHandler(&dxcIncludeHandler);
    }

    /*std::unique_ptr<core::ShaderBytecode> CompileToBytecodeFromFile(const std::string& fileName, const std::wstring& target) const override
    {
      std::wstring fileNameWs(fileName.size(), L' ');
      fileNameWs.resize(std::mbstowcs(&fileNameWs[0], fileName.c_str(), fileName.size()));
      std::wstring pdbFileName{fileNameWs + L".pdb"};
      std::wstring binFileName{fileNameWs + L".bin"};
      CComPtr<IDxcResult> compileResults = RunShaderCompiler(fileNameWs, target);

      CComPtr<IDxcBlob> shaderBlob = ExtractShaderBlob(compileResults);
      //CComPtr<IDxcBlob> pdbBlob = ExtractPdbBlob(compileResults);
      //CComPtr<IDxcBlob> reflectionBlob = ExtractReflectionBlob(compileResults);

      //CComPtr<ID3D12ShaderReflection> reflection = CreateShaderReflection(reflectionBlob);

      return std::make_unique<ShaderBytecode>(shaderBlob);//, pdbBlob, reflection);
    }*/

    std::unique_ptr<core::ShaderBytecode> LoadShaderBytecodeFromFile(const std::string& fileName) override final
    {
      std::wstring fileNameWs(fileName.size(), L' ');
      fileNameWs.resize(std::mbstowcs(&fileNameWs[0], fileName.c_str(), fileName.size()));

      CComPtr<IDxcBlobEncoding> shaderBlobEncoding = nullptr;
      BOOST_VERIFY(SUCCEEDED(dxcUtils->LoadFile(fileNameWs.c_str(), nullptr, &shaderBlobEncoding)));

      CComPtr<IDxcBlob> shaderBlob;
      BOOST_VERIFY(SUCCEEDED(shaderBlobEncoding->QueryInterface(IID_PPV_ARGS(&shaderBlob))));

      return std::make_unique<ShaderBytecode>(std::move(shaderBlob));
    }

  private:
    CComPtr<IDxcBlob> ExtractShaderBlob(const CComPtr<IDxcResult>& compileResults) const
    {
      CComPtr<IDxcBlob> shaderBlob = nullptr;
      CComPtr<IDxcBlobUtf16> shaderName = nullptr;
      BOOST_VERIFY(SUCCEEDED(compileResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), &shaderName)));
      BOOST_ASSERT(shaderBlob != nullptr);

      return std::move(shaderBlob);
    }

    CComPtr<IDxcBlob> ExtractReflectionBlob(const CComPtr<IDxcResult>& compileResults) const
    {
      CComPtr<IDxcBlob> reflectionBlob = nullptr;
      BOOST_VERIFY(SUCCEEDED(compileResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr)));
      BOOST_ASSERT(reflectionBlob != nullptr);
      return std::move(reflectionBlob);
    }

    CComPtr<IDxcBlob> ExtractPdbBlob(const CComPtr<IDxcResult>& compileResults) const
    {
      CComPtr<IDxcBlob> pdbBlob = nullptr;
      CComPtr<IDxcBlobUtf16> pdbName = nullptr;
      BOOST_VERIFY(SUCCEEDED(compileResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pdbBlob), &pdbName)));
      BOOST_ASSERT(pdbBlob != nullptr);
      return std::move(pdbBlob);
    }

    CComPtr<ID3D12ShaderReflection> CreateShaderReflection(const CComPtr<IDxcBlob>& reflectionBlob) const
    {
      CComPtr<ID3D12ShaderReflection> reflection;

      if (reflectionBlob != nullptr)
      {
        DxcBuffer reflectionDesc;
        reflectionDesc.Encoding = DXC_CP_ACP;
        reflectionDesc.Ptr = reflectionBlob->GetBufferPointer();
        reflectionDesc.Size = reflectionBlob->GetBufferSize();

        BOOST_VERIFY(SUCCEEDED(dxcUtils->CreateReflection(&reflectionDesc, IID_PPV_ARGS(&reflection))));
      }

      return std::move(reflection);
    }

    CComPtr<IDxcResult> RunShaderCompiler(const std::wstring& fileNameWs, const std::wstring& target) const
    {
      std::vector<LPCWSTR> compilerArgs =
      {
        fileNameWs.c_str(),
        L"-E", L"main",
        L"-T", target.c_str(),
        DXC_ARG_WARNINGS_ARE_ERRORS,
        //DXC_ARG_DEBUG,
        //L"-Fd", L"VertexShader.pdb"
        //L"-Qstrip_debug",
        //L"-Qstrip_reflect",
      };

      CComPtr<IDxcBlobEncoding> sourceBlobEncoding = nullptr;
      BOOST_VERIFY(SUCCEEDED(dxcUtils->LoadFile(fileNameWs.c_str(), nullptr, &sourceBlobEncoding)));

      DxcBuffer dxcSourceBuffer;
      dxcSourceBuffer.Ptr = sourceBlobEncoding->GetBufferPointer();
      dxcSourceBuffer.Size = sourceBlobEncoding->GetBufferSize();
      dxcSourceBuffer.Encoding = DXC_CP_ACP;

      CComPtr<IDxcResult> compileResults;
      BOOST_VERIFY(SUCCEEDED(dxcCompiler->Compile(&dxcSourceBuffer, compilerArgs.data(), static_cast<uint32_t>(compilerArgs.size()),
        dxcIncludeHandler, IID_PPV_ARGS(&compileResults))));

      return std::move(compileResults);
    }

    void CheckErrors(const CComPtr<IDxcResult>& results, const std::string& fileName) const
    {
      CComPtr<IDxcBlobUtf8> errors = nullptr;
      BOOST_VERIFY(SUCCEEDED(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)));
      if (errors != nullptr && errors->GetStringLength() != 0)
      {
        wprintf(L"Warnings and Errors:\n%S\n", errors->GetStringPointer());
      }

      HRESULT status;
      BOOST_VERIFY(SUCCEEDED(results->GetStatus(&status)));
      if (FAILED(status))
      {
        throw std::runtime_error(fileName + " compilation failed");
      }
    }

    CComPtr<IDxcCompiler3> dxcCompiler;
    CComPtr<IDxcUtils> dxcUtils;
    CComPtr<IDxcIncludeHandler> dxcIncludeHandler;
  };
}
