#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class TextAlign : uint8
  {
    Left,
    Center,
    Right,
    Justify
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::TextAlign>()
  {
    return 4;
  }
}