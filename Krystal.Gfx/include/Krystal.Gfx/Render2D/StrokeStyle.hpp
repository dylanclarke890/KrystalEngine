#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::gfx
{
  /// @brief How a stroke's open ends are drawn.
  enum class LineCap : uint8
  {
    /// @brief The stroke stops at the end point.
    Butt,
    /// @brief A half circle centred on the end point.
    Round,
    /// @brief The stroke goes on for half its width past the end point.
    Square,
  };

  /// @brief How a stroke turns its corners.
  enum class LineJoin : uint8
  {
    /// @brief A sharp point, cut back to a bevel when it would be longer than the miter limit allows.
    Miter,
    /// @brief An arc centred on the corner.
    Round,
    /// @brief The outer edges joined by a straight line.
    Bevel,
  };

  /// @brief How a path is stroked, with the meanings canvas and SVG give these settings.
  struct StrokeStyle
  {
    /// @brief The stroke's width, in the path's coordinates. 0 strokes a hairline one target pixel wide.
    float Width {1.f};
    LineCap Cap {LineCap::Butt};
    LineJoin Join {LineJoin::Miter};
    /// @brief The longest a miter may be, in multiples of the width, before it's beveled.
    float MiterLimit {10.f};
    /// @brief Alternating dash and gap lengths, starting with a dash. An odd number of lengths is repeated to
    /// make it even. Empty, negative or all-zero patterns stroke solid.
    List<float> Dashes {};
    /// @brief How far into the pattern each contour starts.
    float DashOffset {0.f};
  };
}
