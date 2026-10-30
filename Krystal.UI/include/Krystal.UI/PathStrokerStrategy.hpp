#pragma once

#include "Krystal.Booey/Platform/PathStrokeStrategy.hpp"
#include "Krystal.Gfx/Render2D/PathStroker.hpp"

namespace krys::ui
{
  /// @brief Turns Booey's strokes into geometry with Render2D's stroker, the one that draws them: a point is
  /// inside a stroke when it's inside the stroke's outline, and a stroke's bounds are its outline's.
  class PathStrokerStrategy final : public boo::PathStrokeStrategy
  {
  private:
    gfx::PathStroker _stroker;

  public:
    KRYS_NODISCARD bool StrokeContains(const boo::Path &path, const boo::FloatPoint &point,
                                       const boo::PathStrokeStyle &style) noexcept override;
    KRYS_NODISCARD boo::FloatRect StrokeBoundingRect(const boo::Path &path,
                                                     const boo::PathStrokeStyle &style) noexcept override;

  private:
    /// @brief The outline of @p path stroked with @p style, in the path's coordinates.
    KRYS_NODISCARD gfx::Path2D Outline(const boo::Path &path, const boo::PathStrokeStyle &style) noexcept;
  };
}
