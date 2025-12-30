#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
  constexpr UnderlyingType<UI::Overflow> OrdinalCount<UI::Overflow> = 3;
}