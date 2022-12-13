export module engine.render.core.image;

import engine.render.core.resource;

namespace core
{
  export class Image : public core::Resource
  {
  public:
    virtual ~Image() = default;
  };
}
