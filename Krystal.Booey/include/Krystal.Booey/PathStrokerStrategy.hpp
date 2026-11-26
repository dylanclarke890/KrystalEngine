#pragma once

#include "Krystal.Gfx/Render2D/PathStroker.hpp"
#include "Krystal.Web/Platform/PathStrokeStrategy.hpp"

namespace krys::boo
{
  /// @brief Turns Krystal.Web's strokes into geometry with Render2D's stroker, the one that draws them: a point is
  /// inside a stroke when it's inside the stroke's outline, and a stroke's bounds are its outline's.
  class PathStrokerStrategy final : public web::PathStrokeStrategy
  {
  private:
    gfx::PathStroker _stroker;

  public:
    KRYS_NODISCARD bool StrokeContains(const web::Path &path, const PointF &point,
                                       const web::PathStrokeStyle &style) noexcept override;

    KRYS_NODISCARD RectF StrokeBoundingRect(const web::Path &path,
                                            const web::PathStrokeStyle &style) noexcept override;

  private:
    /// @brief The outline of @p path stroked with @p style, in the path's coordinates.
    KRYS_NODISCARD gfx::Path2D Outline(const web::Path &path, const web::PathStrokeStyle &style) noexcept;
  };
}
