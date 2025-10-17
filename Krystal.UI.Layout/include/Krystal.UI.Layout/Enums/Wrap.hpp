#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Layout
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
  constexpr uint32 OrdinalCount<UI::Layout::Wrap>()
  {
    return 3;
  }
}