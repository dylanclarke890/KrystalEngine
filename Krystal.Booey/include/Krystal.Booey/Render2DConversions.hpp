#pragma once

#include "Krystal.Core/Geometry/AffineTransform.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Gfx/Render2D/AffineTransformFloats.hpp"
#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/RoundedRect2D.hpp"
#include "Krystal.Gfx/Render2D/StrokeStyle.hpp"
#include "Krystal.Web/Graphics/Color/Color.hpp"
#include "Krystal.Web/Graphics/Geometry/FloatRoundedRect.hpp"
#include "Krystal.Web/Graphics/GraphicsTypes.hpp"
#include "Krystal.Web/Graphics/Paths/WindRule.hpp"

namespace krys::web
{
  class Gradient;
  class Path;
}

namespace krys::boo
{
  KRYS_NODISCARD constexpr Vec2f ToVec2f(const PointF &point) noexcept
  {
    return {point.X(), point.Y()};
  }

  KRYS_NODISCARD constexpr gfx::FillRule ToFillRule(web::WindRule rule) noexcept
  {
    return rule == web::WindRule::EvenOdd ? gfx::FillRule::EvenOdd : gfx::FillRule::NonZero;
  }

  KRYS_NODISCARD constexpr gfx::LineCap ToLineCap(web::LineCap cap) noexcept
  {
    switch (cap)
    {
      case web::LineCap::Butt:   return gfx::LineCap::Butt;
      case web::LineCap::Round:  return gfx::LineCap::Round;
      case web::LineCap::Square: return gfx::LineCap::Square;
    }
    return gfx::LineCap::Butt;
  }

  KRYS_NODISCARD constexpr gfx::LineJoin ToLineJoin(web::LineJoin join) noexcept
  {
    switch (join)
    {
      case web::LineJoin::Miter: return gfx::LineJoin::Miter;
      case web::LineJoin::Round: return gfx::LineJoin::Round;
      case web::LineJoin::Bevel: return gfx::LineJoin::Bevel;
    }
    return gfx::LineJoin::Miter;
  }

  /// @brief A colour as Render2D paints it: 8-bit sRGB with straight alpha, multiplied by @p alpha. Colours
  /// in other spaces are converted, clamping to sRGB's gamut; an invalid colour is transparent.
  KRYS_NODISCARD SRGBA<uint8> ToColour(const web::Color &color, float alpha = 1.f) noexcept;

  /// @brief The rounded rect as Render2D's analytic shape, when every corner is circular, or within a
  /// sixteenth of a pixel of it.
  /// @note Snapping a box to device pixels scales its radii with it, so a box's circular corners come out a
  /// little elliptical (16 by 15.99, say); drawn as paths, each such box costs a coverage mask.
  KRYS_NODISCARD Maybe<gfx::RoundedRect2D> ToCircularRoundedRect(const web::FloatRoundedRect &rect) noexcept;

  /// @brief Adds a rounded rect, its corners elliptical, as a closed contour.
  void AddRoundedRect(gfx::Path2D &path, const web::FloatRoundedRect &rect) noexcept;

  /// @brief A path's segments as a `Path2D`: arcs, ellipses and rounded rects become curves.
  KRYS_NODISCARD gfx::Path2D ToPath2D(const web::Path &path) noexcept;

  /// @brief A gradient as Render2D paints it, its stops' alpha multiplied by @p alpha.
  KRYS_NODISCARD gfx::Gradient2D ToGradient2D(const web::Gradient &gradient, float alpha = 1.f) noexcept;
}
