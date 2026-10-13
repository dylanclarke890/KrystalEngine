#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  template <Numeric StyleType>
  struct Serialiser<StyleType>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value) noexcept
    {
      // FIXME: Do this more efficiently without creating and destroying a css::Numeric object.
      css::Serialise(builder, context, ConvertToCSS(value, style));
    }
  };

  template <DimensionPercentageNumeric StyleType>
  struct Serialiser<StyleType>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value) noexcept
    {
      // FIXME: Do this more efficiently without creating and destroying a css::Numeric object.
      css::Serialise(builder, context, ConvertToCSS(value, style));
    }
  };

  template <Calc StyleType>
  struct Serialiser<StyleType>
  {
    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value) noexcept
    {
      // FIXME: Do this more efficiently without creating and destroying a css::UnevaluatedCalc object.
      css::Serialise(builder, context, ConvertToCSS(value, style));
    }
  };

  template <auto nR, auto pR, typename V>
  struct Serialiser<NumberOrPercentage<nR, pR, V>>
  {
    using StyleType = NumberOrPercentage<nR, pR, V>;

    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value) noexcept
    {
      // FIXME: Do this more efficiently without creating and destroying a css::UnevaluatedCalc object.
      css::Serialise(builder, context, ConvertToCSS(value, style));
    }
  };

  template <auto nR, auto pR, typename V>
  struct Serialiser<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using StyleType = NumberOrPercentageResolvedToNumber<nR, pR, V>;

    void operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                    const RenderStyle &style, const StyleType &value) noexcept
    {
      // FIXME: Do this more efficiently without creating and destroying a css::UnevaluatedCalc object.
      css::Serialise(builder, context, ConvertToCSS(value, style));
    }
  };
}
