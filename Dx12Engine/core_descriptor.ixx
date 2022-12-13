export module engine.render.core.descriptor;

namespace core
{
  export class Descriptor
  {
  public:
    virtual ~Descriptor() = default;

    enum HEAP_TYPE
    {
      HEAP_TYPE_RTV = 0,
      HEAP_TYPE_CBV_SRV_UAV,
      HEAP_TYPE_COUNT
    };

    enum RANGE_TYPE
    {
      RANGE_TYPE_UAV,
      RANGE_TYPE_SRV,
      RANGE_TYPE_COUNT
    };
  };
}
