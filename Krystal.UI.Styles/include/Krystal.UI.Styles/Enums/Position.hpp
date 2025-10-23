#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
{
  enum class PositionType : uint8
  {
    Static,
    Relative,
    Absolute,
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Styles::PositionType>()
  {
    return 3;
  }
}