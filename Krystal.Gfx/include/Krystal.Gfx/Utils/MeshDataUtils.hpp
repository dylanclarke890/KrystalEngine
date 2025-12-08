#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx/Resources/Mesh.hpp"
#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Gfx
{
  class MeshDataUtils
  {
    STATIC_CLASS(MeshDataUtils)

  public:
    static void GenerateQuad(MeshData &data, const Maths::Vec2 &origin, const Maths::Vec2 &dimensions,
                             const ColourbPremultiplied &colour) noexcept
    {
      GenerateQuad(data, origin, dimensions, colour, Maths::Vec2 {0.f, 0.f}, Maths::Vec2 {1.f, 1.f});
    }

    static void GenerateQuad(MeshData &data, const Maths::Vec2 &origin, const Maths::Vec2 &dimensions,
                             const ColourbPremultiplied &colour, const Maths::Vec2 &topLeftTexCoord,
                             const Maths::Vec2 &bottomRightTexCoord) noexcept
    {
      using Vertex = Vertex::Position2D_ColourbPremultiplied_UV;

      const size_t v0 = data.Vertices.size();
      const size_t i0 = data.Indices.size();
      const size_t verticesSize = 4 * sizeof(Vertex);
      const size_t indicesSize = 6 * sizeof(uint32);

      {
        Array<Vertex, 4> vertices = {};

        vertices[0].Position = origin;
        vertices[0].Colour = colour;
        vertices[0].TexCoord = topLeftTexCoord;

        vertices[1].Position = Maths::Vec2(origin.x + dimensions.x, origin.y);
        vertices[1].Colour = colour;
        vertices[1].TexCoord = Maths::Vec2(bottomRightTexCoord.x, topLeftTexCoord.y);

        vertices[2].Position = origin + dimensions;
        vertices[2].Colour = colour;
        vertices[2].TexCoord = bottomRightTexCoord;

        vertices[3].Position = Maths::Vec2(origin.x, origin.y + dimensions.y);
        vertices[3].Colour = colour;
        vertices[3].TexCoord = Maths::Vec2(topLeftTexCoord.x, bottomRightTexCoord.y);

        data.Vertices.resize(data.Vertices.size() + verticesSize);
        std::memcpy(data.Vertices.data() + v0, &vertices[0], verticesSize);
      }

      {
        Array<uint32, 6> indices {};

        indices[0] = static_cast<uint32>(v0 + 0u);
        indices[1] = static_cast<uint32>(v0 + 1u);
        indices[2] = static_cast<uint32>(v0 + 2u);
        indices[3] = static_cast<uint32>(v0 + 2u);
        indices[4] = static_cast<uint32>(v0 + 3u);
        indices[5] = static_cast<uint32>(v0 + 0u);

        data.Indices.resize(data.Indices.size() + indicesSize);
        std::memcpy(data.Indices.data() + i0, &indices[0], indicesSize);
      }
    }
  };
}