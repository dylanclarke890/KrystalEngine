#include "Krystal.Booey/CSS/Values/Primitives/CSSRatio.hpp"
#include "Krystal.Booey/CSS/Values/CSSRatioValue.hpp"

namespace krys::boo::css
{
  Ref<CSSValue> CSSValueCreation<Ratio>::operator()(CSSValuePool &, const Ratio &ratio) noexcept
  {
    return CSSRatioValue::Create(ratio);
  }
}