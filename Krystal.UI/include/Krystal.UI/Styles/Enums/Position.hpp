#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::PositionType>() noexcept
  {
    return 3;
  }
}