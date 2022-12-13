module;

#include <memory>
#include <string>

export module engine.render.core.shader_compiler;

import engine.render.core.shader_bytecode;

namespace core
{
  export class ShaderCompiler
  {
  public:
    virtual ~ShaderCompiler() = default;

    //virtual std::unique_ptr<ShaderBytecode> CompileToBytecodeFromFile(const std::string& fileName, const std::wstring& target) const = 0;
  
    virtual std::unique_ptr<core::ShaderBytecode> LoadShaderBytecodeFromFile(const std::string& fileName) = 0;
  };
}
