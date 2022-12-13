export module engine.render.core.tlas_builder;

import engine.render.core.buffer;
import engine.render.core.command_bundle;

namespace core
{
  export class TlasBuilder
  {
  public:
    virtual ~TlasBuilder() = default;

    virtual void CreateResourcesAndWriteBuildCommands(
      const core::Buffer& indexBuffer, int numIndices,
      const core::Buffer& vertexBuffer, int numVertices,
      core::CommandBundle& commandBundle) = 0;

    virtual core::Buffer& GetTlas() = 0;
  };
}
