#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class FlexDirection : uint8
  {
    Column,
    ColumnReverse,
    Row,
    RowReverse
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::FlexDirection>()
  {
    return 4;
  }
}