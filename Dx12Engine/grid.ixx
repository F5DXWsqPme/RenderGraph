module;

#include <vector>
#include <array>
#include <algorithm>

#include <boost/assert.hpp>
#include <glm/vec3.hpp>

export module engine.geometry.grid;

import utils.noncopiable;

namespace geometry
{
  export class Grid : private utils::Noncopiable
  {
  public:
    virtual ~Grid() = default;

    Grid(int sizeX, int sizeY) :
      sizeX(sizeX),
      sizeY(sizeY),
      points{sizeX * sizeY}
    {
      GenerateIndices();
    }

    void SetPoint(int x, int y, const glm::vec3 point)
    {
      *GetPointPtr(x, y) = point;
    }

    glm::vec3* GetPointPtr(int x, int y)
    {
      return &points[GetPointOffset(x, y)];
    }

    const glm::vec3* GetVertexData() const
    {
      return points.data();
    }

    const uint32_t* GetIndexData() const
    {
      return indices.data();
    }

    int32_t GetVertexDataSize() const
    {
      return (int32_t)points.size() * sizeof(glm::vec3);
    }

    int32_t GetIndexDataSize() const
    {
      return (int32_t)indices.size() * sizeof(int32_t);
    }

    int32_t GetNumIndices() const
    {
      return (int32_t)indices.size();
    }

    int32_t GetNumVertices() const
    {
      return (int32_t)points.size();
    }

  private:
    void GenerateIndices()
    {
      BOOST_ASSERT(sizeX > 1 && sizeY > 1);

      constexpr int indicesPerBlock = 6;
      indices.resize((sizeX - 1) * (sizeY - 1) * indicesPerBlock);

      constexpr std::array<std::pair<int, int>, indicesPerBlock> blockIndices
        {{{0, 0}, {1, 0}, {0, 1}, {0, 1}, {1, 0}, {1, 1}}};

      for (int y = 0; y < sizeY - 1; y++)
        for (int x = 0; x < sizeX - 1; x++)
        {
          auto firstIndexIt = indices.begin() + (y * (sizeX - 1) + x) * indicesPerBlock;
          std::generate(firstIndexIt, firstIndexIt + indicesPerBlock, [=, i = 0]() mutable
            {
              const std::pair<int, int>& pairIndexInBlock = blockIndices[i++];
              return GetPointOffset(pairIndexInBlock.first + x, pairIndexInBlock.second + y);
            });
        }
    }

    int GetPointOffset(int x, int y)
    {
      BOOST_ASSERT(x >= 0 && y >= 0 && x < sizeX && x < sizeY);
      return y * sizeX + x;
    }

    std::vector<uint32_t> indices;
    std::vector<glm::vec3> points;
    int sizeX = 0;
    int sizeY = 0;
  };
}
