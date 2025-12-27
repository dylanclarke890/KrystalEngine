#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Wrap : uint8
  {
    NoWrap,
    Wrap,
    WrapReverse,
  };
}

namespace Krys
{
  template <>
  NO_DISCARD constexpr uint32 OrdinalCount<UI::Wrap>() noexcept
  {
    return 3;
  }
}