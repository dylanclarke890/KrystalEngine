#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class DisplayType : uint8
  {
    Flex,
    None,
    Contents
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::DisplayType>()
  {
    return 3;
  }
}