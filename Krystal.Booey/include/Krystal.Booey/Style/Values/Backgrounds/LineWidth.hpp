#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include <cmath>

namespace krys::boo
{
  using FloatBoxExtent = RectEdges<float>;
  using LayoutBoxExtent = RectEdges<LayoutUnit>;

  namespace style
  {
    // <line-width> = <length [0,∞]> | thin | medium | thick
    // https://drafts.csswg.org/css-backgrounds/#typedef-line-width
    struct LineWidth
    {
      using Length = style::Length<css::NonNegative>;

      Length Value;

      constexpr LineWidth(Length length) noexcept : Value {length}
      {
      }
      constexpr LineWidth(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Value {literal}
      {
      }

      KRYS_NODISCARD constexpr bool IsZero() const noexcept
      {
        return Value.IsZero();
      }
      KRYS_NODISCARD constexpr bool IsPositive() const noexcept
      {
        return Value.IsPositive();
      }

      KRYS_NODISCARD explicit constexpr operator bool() const noexcept
      {
        return !IsZero();
      }

      KRYS_NODISCARD constexpr bool operator==(const LineWidth &) const noexcept = default;
      KRYS_NODISCARD constexpr auto operator<=>(const LineWidth &) const noexcept = default;
    };
    DEFINE_TYPE_WRAPPER_GET(LineWidth, Value);

    using LineWidthBox = css::MinimallySerialisingSpaceSeparatedRectEdges<style::LineWidth>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<LineWidth>
    {
      KRYS_NODISCARD LineWidth operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Evaluate

    template <typename Result>
    struct Evaluation<LineWidth, Result>
    {
      KRYS_NODISCARD constexpr Result operator()(const LineWidth &value, ZoomNeeded zoom) noexcept
      {
        return Result(value.Value.ResolveZoom(zoom));
      }
    };

    template <>
    struct Evaluation<LineWidthBox, FloatBoxExtent>
    {
      KRYS_NODISCARD FloatBoxExtent operator()(const LineWidthBox &, ZoomNeeded) noexcept;
    };
    template <>
    struct Evaluation<LineWidthBox, LayoutBoxExtent>
    {
      KRYS_NODISCARD LayoutBoxExtent operator()(const LineWidthBox &, ZoomNeeded) noexcept;
    };

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::LineWidth)
