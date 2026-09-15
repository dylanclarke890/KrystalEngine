#include "Krystal.Booey/CSS/Values/Primitives/Ratio.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"

namespace krys::boo::css
{
  Ref<Value> ValueCreation<Ratio>::operator()(ValuePool &, const Ratio &ratio) noexcept
  {
    return RatioValue::Create(ratio);
  }
}