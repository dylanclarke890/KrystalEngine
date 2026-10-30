#pragma once

#include "Krystal.Booey/Graphics/DashArray.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class Path;

  /// @brief How a path is stroked: what a stroke style applier set on a graphics context.
  struct PathStrokeStyle
  {
    float Thickness {1};
    LineCap Cap {LineCap::Butt};
    LineJoin Join {LineJoin::Miter};
    float MiterLimit {10};
    /// @brief Empty for a solid stroke.
    DashArray Dashes {};
    float DashOffset {0};
  };

  /// @brief How the host turns strokes into geometry, for hit testing strokes and measuring their bounds.
  class PathStrokeStrategy
  {
  public:
    virtual ~PathStrokeStrategy() noexcept = default;

    /// @brief Whether @p point is inside @p path stroked with @p style.
    KRYS_NODISCARD virtual bool StrokeContains(const Path &path, const FloatPoint &point,
                                               const PathStrokeStyle &style) noexcept = 0;
    /// @brief The bounds of @p path stroked with @p style.
    KRYS_NODISCARD virtual FloatRect StrokeBoundingRect(const Path &path,
                                                        const PathStrokeStyle &style) noexcept = 0;
  };
}
