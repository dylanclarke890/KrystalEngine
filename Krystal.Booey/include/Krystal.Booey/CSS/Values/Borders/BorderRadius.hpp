#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::css
{
  // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
  // https://drafts.csswg.org/css-backgrounds-3/#propdef-border-radius
  struct BorderRadius
  {
    using Axis = SpaceSeparatedArray<LengthPercentage<NonNegative>, 4>;
    using Corner = MinimallySerialisingSpaceSeparatedSize<LengthPercentage<NonNegative>>;

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
      return {Horizontal.Value[0], Vertical.Value[0]};
    }

    KRYS_NODISCARD Corner TopRight() const noexcept
    {
      return {Horizontal.Value[1], Vertical.Value[1]};
    }

    KRYS_NODISCARD Corner BottomRight() const noexcept
    {
      return {Horizontal.Value[2], Vertical.Value[2]};
    }

    KRYS_NODISCARD Corner BottomLeft() const noexcept
    {
      return {Horizontal.Value[3], Vertical.Value[3]};
    }

    KRYS_NODISCARD bool operator==(const BorderRadius &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const BorderRadius &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Horizontal;
    }
    else if constexpr (I == 1)
    {
      return value.Vertical;
    }
  }

  // Returns true if the provided `BorderRadius` contains the default value. This
  // is used to know ahead of time if serialization is needed.
  KRYS_NODISCARD bool HasDefaultValue(const BorderRadius &borderRadius) noexcept;

  template <>
  struct Serialiser<BorderRadius>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const BorderRadius &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::BorderRadius, 2)
