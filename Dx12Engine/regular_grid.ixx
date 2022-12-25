module;

#include <vector>

#include <boost/assert.hpp>
#include <glm/vec3.hpp>

export module engine.geometry.regular_grid;

import utils.noncopiable;
import engine.geometry.grid;

namespace geometry
{
  export class RegularGrid : public Grid
  {
  public:
    RegularGrid(int sizeX, int sizeY, float stepX, float stepY) :
      Grid(sizeX, sizeY)
    {
      for (int y = 0; y < sizeY; y++)
        for (int x = 0; x < sizeX; x++)
          SetPoint(x, y, {x * stepX, 0.f, y * stepY});
    }

    void SetHeight(int x, int y, float newHeight)
    {
      glm::vec3* pointPtr = GetPointPtr(x, y);
      pointPtr->y = newHeight;
    }
  };
}
