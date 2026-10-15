#pragma once

#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Graphics/FloatRoundedRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo
{
  class LayoutSize;

  namespace style
  {
    using BorderRadiusValue = css::MinimallySerialisingSpaceSeparatedSize<LengthPercentage<css::NonNegative>>;

    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds-3/#propdef-border-radius
    struct BorderRadius : RectCorners<BorderRadiusValue>
    {
      BorderRadius(BorderRadiusValue value) noexcept
          : RectCorners<BorderRadiusValue> {value, value, value, value}
      {
      }

      BorderRadius(BorderRadiusValue topLeft, BorderRadiusValue topRight, BorderRadiusValue bottomLeft,
                   BorderRadiusValue bottomRight) noexcept
          : RectCorners<BorderRadiusValue> {krys::move(topLeft), krys::move(topRight), krys::move(bottomLeft),
                                            krys::move(bottomRight)}
      {
      }

      BorderRadius(RectCorners<BorderRadiusValue> &&corners) noexcept
          : RectCorners<BorderRadiusValue> {krys::move(corners)}
      {
      }

      KRYS_NODISCARD bool operator==(const BorderRadius &) const noexcept = default;
    };

    template <size_t I>
    const auto &get(const BorderRadius &value)
    {
      if constexpr (!I)
      {
        return value.TopLeft();
      }
      else if constexpr (I == 1)
      {
        return value.TopRight();
      }
      else if constexpr (I == 2)
      {
        return value.BottomLeft();
      }
      else if constexpr (I == 3)
      {
        return value.BottomRight();
      }
    }

    // MARK: - Conversion

    template <>
    struct ToCSS<BorderRadius>
    {
      KRYS_NODISCARD css::BorderRadius operator()(const BorderRadius &, const RenderStyle &) noexcept;
    };
    template <>
    struct ToStyle<css::BorderRadius>
    {
      KRYS_NODISCARD BorderRadius operator()(const css::BorderRadius &, const BuilderState &) noexcept;
    };

    template <>
    struct CSSValueConversion<BorderRadiusValue>
    {
      KRYS_NODISCARD BorderRadiusValue operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Evaluation

    template <>
    struct Evaluation<BorderRadius, FloatRoundedRect::Radii>
    {
      KRYS_NODISCARD FloatRoundedRect::Radii operator()(const BorderRadius &, FloatSize, ZoomNeeded) noexcept;
    };
    template <>
    struct Evaluation<BorderRadius, LayoutRoundedRect::Radii>
    {
      KRYS_NODISCARD LayoutRoundedRect::Radii operator()(const BorderRadius &, LayoutSize,
                                                         ZoomNeeded) noexcept;
    };

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::BorderRadius, 4)
