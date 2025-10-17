#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Layout
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
  constexpr uint32 OrdinalCount<UI::Layout::FlexDirection>()
  {
    return 4;
  }
}