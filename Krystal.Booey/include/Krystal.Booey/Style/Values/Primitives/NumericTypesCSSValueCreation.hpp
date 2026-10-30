#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  template <Numeric StyleType>
  struct CSSValueCreation<StyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <DimensionPercentageNumeric StyleType>
  struct CSSValueCreation<StyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <Calc StyleType>
  struct CSSValueCreation<StyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <css::Range nR, css::Range pR, typename V>
  struct CSSValueCreation<NumberOrPercentage<nR, pR, V>>
  {
    using StyleType = NumberOrPercentage<nR, pR, V>;

    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <css::Range nR, css::Range pR, typename V>
  struct CSSValueCreation<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using StyleType = NumberOrPercentageResolvedToNumber<nR, pR, V>;

    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };
}
