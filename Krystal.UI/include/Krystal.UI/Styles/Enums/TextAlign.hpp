#pragma once

#include "Krystal.Lib/Enum.hpp"
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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::TextAlign>() noexcept
  {
    return 4;
  }
}