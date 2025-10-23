#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
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
  constexpr uint32 OrdinalCount<UI::Styles::Display>()
  {
    return 3;
  }
}