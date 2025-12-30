#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Display : uint8
  {
    Flex,
    None,
    Contents
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::Display> OrdinalCount<UI::Display> = 3;
}