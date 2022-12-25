module;

#include <filesystem>

#include <stb_image.h>
#include <boost/assert.hpp>
#include <glm/vec2.hpp>

export module utils.picture;

import utils.noncopiable;

namespace utils
{
  export class Picture final : private Noncopiable
  {
  public:
    Picture(const std::filesystem::path& file)
    {
      std::filesystem::path path = std::filesystem::current_path() / file;
      stbImage = stbi_load(path.string().c_str(), &width, &height, &channels, 0);
      BOOST_ASSERT(stbImage != nullptr);
    }

    ~Picture()
    {
      stbi_image_free(stbImage);
    }

    float SampleR(float x, float y)
    {
      const glm::vec2 coords{x * width, y * height};
      const glm::ivec2 index{std::ceil(coords.x), std::ceil(coords.y)};
      const float p00 = GetRByClampedCoord(index.x + 0, index.y + 0);
      const float p01 = GetRByClampedCoord(index.x + 0, index.y + 1);
      const float p10 = GetRByClampedCoord(index.x + 1, index.y + 0);
      const float p11 = GetRByClampedCoord(index.x + 1, index.y + 1);
      const glm::vec2 coordsInPixel = coords - glm::vec2(index.x, index.y);
      return
        p00 * (1 - coordsInPixel.x) * (1 - coordsInPixel.y) +
        p00 * (1 - coordsInPixel.x) * coordsInPixel.y +
        p00 * coordsInPixel.x * (1 - coordsInPixel.y) +
        p00 * coordsInPixel.x * coordsInPixel.y;
    }

  private:
    float GetRByClampedCoord(int x, int y) const
    {
      return GetR(std::clamp(x, 0, width), std::clamp(y, 0, height));
    }

    float GetR(int x, int y) const
    {
      const unsigned char* const pixelPtr = GetPixelPtr(x, y);
      const float r = *(pixelPtr) / 255.f;
      return r;
    }

    const unsigned char* GetPixelPtr(int x, int y) const
    {
      BOOST_ASSERT(x >= 0 && y >= 0 && x < width && x < height);
      return stbImage + (y * width + x) * channels;
    }

    int channels = 0;
    int width = 0;
    int height = 0;
    unsigned char* stbImage = nullptr;
  };
}
