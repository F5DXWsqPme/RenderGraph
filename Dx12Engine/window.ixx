module;

#include <string_view>

#include <sdl2/sdl.h>
#include <sdl2/SDL_syswm.h>

#include <boost/assert.hpp>

export module engine.os.sdl.window;

import utils.noncopiable;
import engine.os.sdl.initializer;

namespace sdl
{
  export class Window final
  {
  public:
    Window(const std::string_view& name, int width, int height) :
      width(width), height(height)
    {
      Initializer{};

      windowPtr = SDL_CreateWindow(name.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_SHOWN);

      BOOST_ASSERT(windowPtr != nullptr);
    }

    ~Window()
    {
      SDL_DestroyWindow(windowPtr);
    }

    /**
     * Process currently pending events
     *
     * \returns false if window closed, true if otherwise
     */
    bool ProcessEvents() const
    {
      SDL_Event event;

      while (SDL_PollEvent(&event) != 0)
      {
        if (event.type == SDL_QUIT)
          return false;

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
          SDL_GetWindowID(windowPtr) == event.window.windowID)
        {
          return false;
        }
      }

      return true;
    }

    SDL_SysWMinfo GetWindowSystemInfo() const
    {
      SDL_SysWMinfo wmInfo;
      SDL_VERSION(&wmInfo.version);
      SDL_GetWindowWMInfo(windowPtr, &wmInfo);
      return wmInfo;
    }

    int GetWidth() const noexcept
    {
      return width;
    }

    int GetHeight() const noexcept
    {
      return height;
    }

  private:
    int width = 0;
    int height = 0;
    SDL_Window* windowPtr = nullptr;
  };
}
