#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <dasharray-value> = [ <length-percentage [0,∞]> | <number [0,∞]>@(converted-to-px) ]
  struct SVGStrokeDasharrayValueLength : LengthWrapperBase<LengthPercentage<css::NonNegative>>
  {
    using Base::Base;
  };
  struct SVGStrokeDasharrayValue
  {
    using Fixed = SVGStrokeDasharrayValueLength::Fixed;
    using Percentage = SVGStrokeDasharrayValueLength::Percentage;
    using Calc = SVGStrokeDasharrayValueLength::Calc;

    SVGStrokeDasharrayValueLength Value;

    SVGStrokeDasharrayValue(SVGStrokeDasharrayValueLength &&length) noexcept : Value {krys::move(length)}
    {
    }
    SVGStrokeDasharrayValue(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept : Value {literal}
    {
    }
    SVGStrokeDasharrayValue(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : Value {literal}
    {
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const SVGStrokeDasharrayValue &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(SVGStrokeDasharrayValue, Value);

  // <dasharray> = [ <dasharray-value>+ ]#
  using SVGStrokeDasharrayList = css::CommaSeparatedFixedList<SVGStrokeDasharrayValue>;

  // <'stroke-dasharray'> = none | <dasharray>
  // https://svgwg.org/svg2-draft/painting.html#StrokeDashing
  struct SVGStrokeDasharray : css::ListOrNone<SVGStrokeDasharrayList>
  {
    using css::ListOrNone<SVGStrokeDasharrayList>::ListOrNone;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGStrokeDasharrayValue>
  {
    KRYS_NODISCARD SVGStrokeDasharrayValue operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<SVGStrokeDasharray>
  {
    KRYS_NODISCARD SVGStrokeDasharray Blend(const SVGStrokeDasharray &, const SVGStrokeDasharray &,
                                            const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::SVGStrokeDasharrayValue)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGStrokeDasharrayValueLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGStrokeDasharray)
