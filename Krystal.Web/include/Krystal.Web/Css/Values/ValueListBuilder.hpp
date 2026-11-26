#pragma once

#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"

namespace krys::web::css
{
  constexpr size_t ValueListBuilderInlineCapacity = 4uz;
  using ValueListBuilder = SmallList<Ref<Value>, ValueListBuilderInlineCapacity>;
}
