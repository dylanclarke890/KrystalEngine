#pragma once

#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'accent-color'> = auto | <color>
  // https://drafts.csswg.org/css-ui-4/#propdef-accent-color
  struct AccentColor : css::ValueOrKeyword<Color, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsColor() const noexcept
    {
      return IsValue();
    }
    KRYS_NODISCARD Maybe<Color> TryColor() const noexcept
    {
      return TryValue();
    }

    // Returns the color or a `currentColor` singleton if `auto`.
    const Color &ColorOrCurrentColor() const;
  };

  // MARK: - Blending

  template <>
  struct Blending<AccentColor>
  {
    KRYS_NODISCARD bool Equals(const AccentColor &, const AccentColor &, const RenderStyle &,
                               const RenderStyle &) noexcept;
    KRYS_NODISCARD bool CanBlend(const AccentColor &, const AccentColor &) noexcept;
    KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const AccentColor &,
                                                                                const AccentColor &) noexcept
    {
      return true;
    }
    KRYS_NODISCARD AccentColor Blend(const AccentColor &, const AccentColor &, const RenderStyle &,
                                     const RenderStyle &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::AccentColor);
