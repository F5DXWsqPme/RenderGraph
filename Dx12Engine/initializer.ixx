module;

#include <sdl2/sdl.h>
#include <boost/assert.hpp>

export module engine.os.sdl.initializer;

import utils.noncopiable;

export namespace sdl
{
  export class Initializer final : private utils::Noncopiable
  {
  public:
    Initializer()
    {
      static InitializerGuard guard{};
    }

  private:
    class InitializerGuard final : private utils::Noncopiable
    {
    public:
      InitializerGuard()
      {
        BOOST_VERIFY(SDL_Init(SDL_INIT_EVERYTHING) == 0);
      }

      ~InitializerGuard()
      {
        SDL_Quit();
      }
    };
  };
}
