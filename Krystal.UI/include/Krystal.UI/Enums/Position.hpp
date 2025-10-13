#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Position : uint8
  {
    Static,
    Relative,
    Absolute,
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::Position>()
  {
    return 3;
  }
}