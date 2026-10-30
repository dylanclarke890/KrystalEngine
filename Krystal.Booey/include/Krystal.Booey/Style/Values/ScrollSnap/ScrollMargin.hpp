#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{
  class LayoutRect;
  class LayoutUnit;

  namespace style
  {
    // <'scroll-margin-*'> = <length>
    // https://drafts.csswg.org/css-scroll-snap-1/#margin-longhands-physical
    struct ScrollMarginEdge
    {
      using Fixed = Length<>;

      ScrollMarginEdge(Fixed &&fixed) noexcept : _value(fixed)
      {
      }
      ScrollMarginEdge(const Fixed &fixed) noexcept : _value(fixed)
      {
      }

      ScrollMarginEdge(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : _value(literal)
      {
      }

      KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsZero() const noexcept
      {
        return _value.IsZero();
      }
      KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsPositive() const noexcept
      {
        return _value.IsPositive();
      }
      KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsNegative() const noexcept
      {
        return _value.IsNegative();
      }

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

        return visitor(_value);
      }

      KRYS_NODISCARD bool operator==(const ScrollMarginEdge &) const noexcept = default;

    private:
      friend struct Evaluation<ScrollMarginEdge, LayoutUnit>;
      friend struct Evaluation<ScrollMarginEdge, float>;

      Length<> _value;
    };

    // <'scroll-margin'> = <length>{1,4}
    // https://drafts.csswg.org/css-scroll-snap-1/#propdef-scroll-margin
    using ScrollMarginBox = css::MinimallySerialisingSpaceSeparatedRectEdges<ScrollMarginEdge>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<ScrollMarginEdge>
    {
      KRYS_NODISCARD ScrollMarginEdge operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Evaluation

    template <>
    struct Evaluation<ScrollMarginEdge, LayoutUnit>
    {
      KRYS_NODISCARD LayoutUnit operator()(const ScrollMarginEdge &, LayoutUnit referenceLength,
                                           ZoomNeeded) noexcept;
      KRYS_NODISCARD LayoutUnit operator()(const ScrollMarginEdge &, ZoomNeeded) noexcept;
    };
    template <>
    struct Evaluation<ScrollMarginEdge, float>
    {
      KRYS_NODISCARD float operator()(const ScrollMarginEdge &, float referenceLength, ZoomNeeded) noexcept;
      KRYS_NODISCARD float operator()(const ScrollMarginEdge &, ZoomNeeded) noexcept;
    };

    // MARK: - Extent

    KRYS_NODISCARD LayoutBoxExtent ExtentForRect(const ScrollMarginBox &, const LayoutRect &) noexcept;

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollMarginEdge)
