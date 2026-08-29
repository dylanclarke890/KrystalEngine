#include "Krystal.HTML/CSS/Values/Primitives/CSSRatio.hpp"
#include "Krystal.HTML/CSS/Values/CSSRatioValue.hpp"

namespace Krys::HTML
{
  Ref<CSSValue> CSSValueCreation<Ratio>::operator()(CSSValuePool &, const Ratio &ratio) noexcept
  {
    return CSSRatioValue::Create(ratio);
  }
}