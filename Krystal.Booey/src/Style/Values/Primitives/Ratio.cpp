#include "Krystal.Booey/Style/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::style
{
  Ref<css::Value> CSSValueCreation<Ratio>::operator()(css::ValuePool &, const RenderStyle &style,
                                                      const Ratio &ratio) noexcept
  {
    return css::RatioValue::Create(ConvertToCSS(ratio, style));
  }
}
