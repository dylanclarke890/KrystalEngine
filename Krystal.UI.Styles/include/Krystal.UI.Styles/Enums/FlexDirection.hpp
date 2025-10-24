#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
{
  enum class FlexDirection : uint8
  {
    Column,
    ColumnReverse,
    Row,
    RowReverse
  };

  NO_DISCARD inline bool IsRow(const FlexDirection flexDirection) noexcept
  {
    return flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse;
  }

  NO_DISCARD inline bool IsColumn(const FlexDirection flexDirection) noexcept
  {
    return flexDirection == FlexDirection::Column || flexDirection == FlexDirection::ColumnReverse;
  }
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Styles::FlexDirection>()
  {
    return 4;
  }
}