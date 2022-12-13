export module engine.render.core.command_buffer;

namespace core
{
  export class CommandBuffer
  {
  public:
    virtual ~CommandBuffer() = default;

    enum TYPE
    {
      TYPE_DIRECT,
      TYPE_COPY,
      TYPE_COMPUTE,
      TYPE_COUNT
    };
  };
}
