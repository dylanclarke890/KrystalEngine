#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Gutter : uint8
  {
    Row,
    Column,
    All
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::Gutter> OrdinalCount<UI::Gutter> = 3;
}