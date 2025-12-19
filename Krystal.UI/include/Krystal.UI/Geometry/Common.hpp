#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Maths/Interpolate.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Gfx/Vertex.hpp"

namespace Krys::UI
{
  using Vertex = Gfx::Vertex::Position2D_ColourbPremultiplied_UV;
  using Index = uint32;
  using GeometryMeshWriter = Gfx::MeshDataWriter<Vertex, Index>;

  /// @brief Ordered by top, right, bottom, left.
  using EdgeSizes = Array<float, 4>;

  /// @brief Ordered by top-left, top-right, bottom-right, bottom-left.
  using CornerSizes = Array<float, 4>;
  using CornerSizes2 = Array<Maths::Vec2, 4>;
  using CornerPositions = Array<Maths::Vec2, 4>;

  enum class BoxEdge : uint8
  {
    Top,
    Right,
    Bottom,
    Left
  };

  constexpr auto operator+(BoxEdge c)
  {
    return static_cast<std::underlying_type_t<BoxEdge>>(c);
  }

  enum class BoxCorner : uint8
  {
    TopLeft,
    TopRight,
    BottomRight,
    BottomLeft
  };

  constexpr auto operator+(BoxCorner c)
  {
    return static_cast<std::underlying_type_t<BoxCorner>>(c);
  }

  constexpr int RoundToInteger(float value) noexcept
  {
    if (value > 0.0f)
      return int(value + 0.5f);

    return int(value - 0.5f);
  }

  constexpr Gfx::ColourbPremultiplied RoundedLerp(float t, const Gfx::ColourbPremultiplied v0,
                                                  const Gfx::ColourbPremultiplied v1) noexcept
  {
    using namespace Maths;
    return Gfx::ColourbPremultiplied {
      uchar(RoundToInteger(Lerp(float(v0[0]), float(v1[0]), t))),
      uchar(RoundToInteger(Lerp(float(v0[1]), float(v1[1]), t))),
      uchar(RoundToInteger(Lerp(float(v0[2]), float(v1[2]), t))),
      uchar(RoundToInteger(Lerp(float(v0[3]), float(v1[3]), t))),
    };
  }
}