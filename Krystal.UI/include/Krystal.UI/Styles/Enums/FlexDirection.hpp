#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class FlexDirection : uint8
  {
    Column,
    ColumnReverse,
    Row,
    RowReverse
  };

  KRYS_NODISCARD constexpr inline bool IsRow(const FlexDirection flexDirection) noexcept
  {
    return flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse;
  }

  KRYS_NODISCARD constexpr inline bool IsColumn(const FlexDirection flexDirection) noexcept
  {
    return flexDirection == FlexDirection::Column || flexDirection == FlexDirection::ColumnReverse;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::FlexDirection, 4u)
