#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::Overflow>() noexcept
  {
    return 3;
  }
}