#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct StrokeWidthLength : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };

  // <'stroke-width'> = <length-percentage [0,∞]> | <number [0,∞]>@(converted-to-px)
  // FIXME: The current spec has the grammar as <'stroke-width'> = <length-percentage [0,∞]>#.
  // https://drafts.fxtf.org/fill-stroke-3/#propdef-stroke-width
  struct StrokeWidth
  {
    using Fixed = StrokeWidthLength::Fixed;
    using Percentage = StrokeWidthLength::Percentage;
    using Calc = StrokeWidthLength::Calc;

    StrokeWidthLength Value;

    StrokeWidth(StrokeWidthLength &&length) noexcept : Value {krys::move(length)}
    {
    }
    StrokeWidth(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Value {literal}
    {
    }
    StrokeWidth(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Value {literal}
    {
    }

    KRYS_NODISCARD bool IsPossiblyPositive() const noexcept
    {
      return Value.IsPossiblyPositive();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const StrokeWidth &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(StrokeWidth, Value)

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<StrokeWidth>
  {
    KRYS_NODISCARD StrokeWidth operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::StrokeWidthLength)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::StrokeWidth)
