#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  constexpr size_t ValueListBuilderInlineCapacity = 4uz;
  using ValueListBuilder = SmallList<Ref<Value>, ValueListBuilderInlineCapacity>;
}
