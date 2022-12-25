module;

#include <vector>
#include <filesystem>

#include <boost/assert.hpp>
#include <glm/vec3.hpp>

export module engine.geometry.heightmap_regular_grid;

import utils.noncopiable;
import utils.picture;
import engine.geometry.regular_grid;

namespace geometry
{
  export class HeightmapRegularGrid final : public RegularGrid
  {
  public:
    HeightmapRegularGrid(int sizeX, int sizeY, float stepX, float stepY, const std::filesystem::path& heightmapFilename) :
      RegularGrid(sizeX, sizeY, stepX, stepY)
    {
      utils::Picture heightmap{heightmapFilename};
      for (int y = 0; y < sizeY; y++)
        for (int x = 0; x < sizeX; x++)
          SetHeight(x, y, heightmap.SampleR(x / float(sizeX), y / float(sizeY)));
    }
  };
}
