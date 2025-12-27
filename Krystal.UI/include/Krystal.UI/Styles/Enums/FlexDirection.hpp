#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
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

namespace Krys
{
  template <>
  KRYS_NODISCARD constexpr uint32 OrdinalCount<UI::FlexDirection>() noexcept
  {
    return 4;
  }
}