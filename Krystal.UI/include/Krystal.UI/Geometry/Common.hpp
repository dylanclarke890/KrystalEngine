#pragma once

#include "Krystal.Gfx/Vertex.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Maths/Interpolate.hpp"
#include "Krystal.Maths/Vector.hpp"

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

  // TODO(fix): consider moving to Maths
  template <std::integral T>
  constexpr T RoundTo(float value) noexcept
  {
    return T(value + (value >= 0.0f ? 0.5f : -0.5f));
  }

  constexpr Gfx::ColourbPremultiplied RoundedLerp(const Gfx::ColourbPremultiplied from,
                                                  const Gfx::ColourbPremultiplied to, float factor) noexcept
  {
    const auto Float = [](uchar value) noexcept -> float
    {
      return static_cast<float>(value);
    };

    using namespace Maths;
    return Gfx::ColourbPremultiplied {
      RoundTo<uchar>(Lerp(Float(from[0]), Float(to[0]), factor)),
      RoundTo<uchar>(Lerp(Float(from[1]), Float(to[1]), factor)),
      RoundTo<uchar>(Lerp(Float(from[2]), Float(to[2]), factor)),
      RoundTo<uchar>(Lerp(Float(from[3]), Float(to[3]), factor)),
    };
  }
}