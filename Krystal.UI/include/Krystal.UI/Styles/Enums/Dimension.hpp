#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Dimension : uint8
  {
    Width,
    Height
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::Dimension> OrdinalCount<UI::Dimension> = 2;
}