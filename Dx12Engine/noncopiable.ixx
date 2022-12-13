export module utils.noncopiable;

namespace utils
{
  export class Noncopiable
  {
  public:
    Noncopiable(const Noncopiable&) = delete;
    Noncopiable& operator=(const Noncopiable&) = delete;

  protected:
    Noncopiable() = default;
    virtual ~Noncopiable() = default;
  };
}
