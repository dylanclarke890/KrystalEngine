#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct SVGStrokeDashoffsetLength : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  // <'stroke-dashoffset'> = <length-percentage> | <number>@(converted-to-px)
  // https://svgwg.org/svg2-draft/painting.html#StrokeDashoffsetProperty
  struct SVGStrokeDashoffset
  {
    using Fixed = SVGStrokeDashoffsetLength::Fixed;
    using Percentage = SVGStrokeDashoffsetLength::Percentage;
    using Calc = SVGStrokeDashoffsetLength::Calc;

    SVGStrokeDashoffsetLength Value;

    SVGStrokeDashoffset(SVGStrokeDashoffsetLength &&length) noexcept : Value {krys::move(length)}
    {
    }
    SVGStrokeDashoffset(css::ValueLiteral<css::NumberUnit::Number> literal) noexcept
        : Value {Fixed {static_cast<float>(literal.Value)}}
    {
    }
    SVGStrokeDashoffset(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Value {literal}
    {
    }
    SVGStrokeDashoffset(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept : Value {literal}
    {
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const SVGStrokeDashoffset &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(SVGStrokeDashoffset, Value);

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGStrokeDashoffset>
  {
    KRYS_NODISCARD SVGStrokeDashoffset operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGStrokeDashoffsetLength)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::SVGStrokeDashoffset)
