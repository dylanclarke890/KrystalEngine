#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

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
  constexpr int32 OrdinalCount<UI::Gutter>()
  {
    return 3;
  }
}