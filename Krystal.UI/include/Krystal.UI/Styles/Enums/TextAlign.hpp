#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
  constexpr UnderlyingType<UI::TextAlign> OrdinalCount<UI::TextAlign> = 4;
}