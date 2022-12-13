module;

#include <string>

export module engine.render.core.adapter_properties;

namespace core
{
  export struct AdapterProperties
  {
    std::wstring description{};
    size_t videoMemorySize{0};
    bool isSoftware{false};
  };
}
