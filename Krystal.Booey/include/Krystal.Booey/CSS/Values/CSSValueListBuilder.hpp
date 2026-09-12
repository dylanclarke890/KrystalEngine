#pragma once

#include "Krystal.Booey/CSS/Values/CSSValue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  constexpr size_t CSSValueListBuilderInlineCapacity = 4uz;
  using CSSValueListBuilder = SmallList<Ref<CSSValue>, CSSValueListBuilderInlineCapacity>;
}