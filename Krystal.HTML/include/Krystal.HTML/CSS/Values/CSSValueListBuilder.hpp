#pragma once

#include "Krystal.HTML/CSS/Values/CSSValue.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  constexpr size_t CSSValueListBuilderInlineCapacity = 4uz;
  using CSSValueListBuilder = SmallList<Ref<CSSValue>, CSSValueListBuilderInlineCapacity>;
}