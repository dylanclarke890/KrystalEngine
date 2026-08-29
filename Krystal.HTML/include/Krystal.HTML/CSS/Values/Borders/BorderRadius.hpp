#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
  // https://drafts.csswg.org/css-backgrounds-3/#propdef-border-radius
  struct BorderRadius
  {
    using Axis = SpaceSeparatedArray<LengthPercentage<NonNegative>, 4>;
    using Corner = MinimallySerializingSpaceSeparatedSize<LengthPercentage<NonNegative>>;

    Axis Horizontal;
    Axis Vertical;

    KRYS_NODISCARD static BorderRadius DefaultValue() noexcept
    {
      return BorderRadius {
        .Horizontal = {0_css_px, 0_css_px, 0_css_px, 0_css_px},
        .Vertical = {0_css_px, 0_css_px, 0_css_px, 0_css_px},
      };
    }

    KRYS_NODISCARD Corner TopLeft() const noexcept
    {
      return {Horizontal.value[0], Vertical.value[0]};
    }

    KRYS_NODISCARD Corner TopRight() const noexcept
    {
      return {Horizontal.value[1], Vertical.value[1]};
    }

    KRYS_NODISCARD Corner BottomRight() const noexcept
    {
      return {Horizontal.value[2], Vertical.value[2]};
    }

    KRYS_NODISCARD Corner BottomLeft() const noexcept
    {
      return {Horizontal.value[3], Vertical.value[3]};
    }

    KRYS_NODISCARD bool operator==(const BorderRadius &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const BorderRadius &value) noexcept
  {
    if constexpr (I == 0uz)
    {
      return value.Horizontal;
    }
    else if constexpr (I == 1uz)
    {
      return value.Vertical;
    }
  }

  // Returns true if the provided `BorderRadius` contains the default value. This
  // is used to know ahead of time if serialization is needed.
  KRYS_NODISCARD bool HasDefaultValue(const BorderRadius &borderRadius) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::BorderRadius, 2)
