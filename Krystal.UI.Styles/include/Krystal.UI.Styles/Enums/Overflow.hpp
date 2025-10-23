#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
{
  enum class Overflow : uint8
  {
    Visible,
    Hidden,
    Scroll
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Styles::Overflow>()
  {
    return 3;
  }
}