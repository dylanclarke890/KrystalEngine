#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/WindRule.hpp"
#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "Krystal.Gfx/Render2D/Path2D.hpp"
#include "Krystal.Gfx/Render2D/Rect2D.hpp"
#include "Krystal.Gfx/Render2D/RoundedRect2D.hpp"
#include "Krystal.Gfx/Render2D/StrokeStyle.hpp"
#include "Krystal.Gfx/Render2D/Transform2D.hpp"

namespace krys::boo
{
  class Gradient;
  class Path;
}

namespace krys::ui
{
  KRYS_NODISCARD constexpr gfx::Rect2D ToRect2D(const boo::FloatRect &rect) noexcept
  {
    return {rect.X(), rect.Y(), rect.Width(), rect.Height()};
  }

  KRYS_NODISCARD constexpr Vec2f ToVec2f(const boo::FloatPoint &point) noexcept
  {
    return {point.X(), point.Y()};
  }

  KRYS_NODISCARD constexpr gfx::Transform2D ToTransform2D(const boo::AffineTransform &transform) noexcept
  {
    return {.A = static_cast<float>(transform.A()),
            .B = static_cast<float>(transform.B()),
            .C = static_cast<float>(transform.C()),
            .D = static_cast<float>(transform.D()),
            .E = static_cast<float>(transform.E()),
            .F = static_cast<float>(transform.F())};
  }

  KRYS_NODISCARD constexpr gfx::FillRule ToFillRule(boo::WindRule rule) noexcept
  {
    return rule == boo::WindRule::EvenOdd ? gfx::FillRule::EvenOdd : gfx::FillRule::NonZero;
  }

  KRYS_NODISCARD constexpr gfx::LineCap ToLineCap(boo::LineCap cap) noexcept
  {
    switch (cap)
    {
      case boo::LineCap::Butt:   return gfx::LineCap::Butt;
      case boo::LineCap::Round:  return gfx::LineCap::Round;
      case boo::LineCap::Square: return gfx::LineCap::Square;
    }
    return gfx::LineCap::Butt;
  }

  KRYS_NODISCARD constexpr gfx::LineJoin ToLineJoin(boo::LineJoin join) noexcept
  {
    switch (join)
    {
      case boo::LineJoin::Miter: return gfx::LineJoin::Miter;
      case boo::LineJoin::Round: return gfx::LineJoin::Round;
      case boo::LineJoin::Bevel: return gfx::LineJoin::Bevel;
    }
    return gfx::LineJoin::Miter;
  }

  /// @brief A colour as Render2D paints it: 8-bit sRGB with straight alpha, multiplied by @p alpha. Colours
  /// in other spaces are converted, clamping to sRGB's gamut; an invalid colour is transparent.
  KRYS_NODISCARD SRGBA<uint8> ToColour(const boo::Color &color, float alpha = 1.f) noexcept;

  /// @brief The rounded rect as Render2D's analytic shape, when every corner is circular.
  KRYS_NODISCARD Maybe<gfx::RoundedRect2D> ToCircularRoundedRect(const boo::FloatRoundedRect &rect) noexcept;

  /// @brief Adds a rounded rect, its corners elliptical, as a closed contour.
  void AddRoundedRect(gfx::Path2D &path, const boo::FloatRoundedRect &rect) noexcept;

  /// @brief A path's segments as a `Path2D`: arcs, ellipses and rounded rects become curves.
  KRYS_NODISCARD gfx::Path2D ToPath2D(const boo::Path &path) noexcept;

  /// @brief A gradient as Render2D paints it, its stops' alpha multiplied by @p alpha.
  KRYS_NODISCARD gfx::Gradient2D ToGradient2D(const boo::Gradient &gradient, float alpha = 1.f) noexcept;
}
