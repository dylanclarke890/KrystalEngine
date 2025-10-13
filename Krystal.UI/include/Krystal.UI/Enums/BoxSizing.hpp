#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class BoxSizing : uint8
  {
    BorderBox,
    ContentBox,
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::BoxSizing>()
  {
    return 9;
  }
}