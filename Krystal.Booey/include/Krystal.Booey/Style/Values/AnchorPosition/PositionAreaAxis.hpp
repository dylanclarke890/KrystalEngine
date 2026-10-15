#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"

namespace krys::boo::style
{
  // The axis that the span specifies.
  // Represented as 3 bits: axis type, direction type, [Logical]BoxAxis value.
  enum class PositionAreaAxis : uint8
  {
    // Physical axes × Physical directions.
    Horizontal = 0b000,
    Vertical = 0b001,

    // Physical axes × Logical directions.
    X = 0b010,
    Y = 0b011,

    // Logical axes × Logical directions.
    Inline = 0b110,
    Block = 0b111,
  };

  // Get the opposite axis of a given axis.
  KRYS_NODISCARD constexpr PositionAreaAxis OppositePositionAreaAxis(PositionAreaAxis axis) noexcept
  {
    switch (axis)
    {
      case PositionAreaAxis::Horizontal: return PositionAreaAxis::Vertical;
      case PositionAreaAxis::Vertical:   return PositionAreaAxis::Horizontal;

      case PositionAreaAxis::X:          return PositionAreaAxis::Y;
      case PositionAreaAxis::Y:          return PositionAreaAxis::X;

      case PositionAreaAxis::Block:      return PositionAreaAxis::Inline;
      case PositionAreaAxis::Inline:     return PositionAreaAxis::Block;
    }

    krys_unreachable();
  }

  KRYS_NODISCARD constexpr bool IsPositionAreaAxisLogical(PositionAreaAxis positionAxis) noexcept
  {
    constexpr uint8 axisBit = 0b100;
    return static_cast<uint8>(positionAxis) & axisBit;
  }

  KRYS_NODISCARD constexpr bool IsPositionAreaDirectionLogical(PositionAreaAxis positionAxis) noexcept
  {
    constexpr uint8 directionBit = 0b010;
    return static_cast<uint8>(positionAxis) & directionBit;
  }

  KRYS_NODISCARD constexpr BoxAxis MapPositionAreaAxisToPhysicalAxis(PositionAreaAxis positionAxis,
                                                                     WritingMode writingMode) noexcept
  {
    constexpr uint8 orientationBit = 0b001;
    auto physicalAxis = static_cast<uint8>(positionAxis) & orientationBit;
    if (IsPositionAreaAxisLogical(positionAxis) && writingMode.IsVertical())
    {
      physicalAxis = !physicalAxis;
    }
    return static_cast<BoxAxis>(physicalAxis);
  }

  KRYS_NODISCARD constexpr LogicalBoxAxis MapPositionAreaAxisToLogicalAxis(PositionAreaAxis positionAxis,
                                                                           WritingMode writingMode) noexcept
  {
    constexpr uint8 orientationBit = 0b001;
    auto logicalAxis = static_cast<uint8>(positionAxis) & orientationBit;
    if (!IsPositionAreaAxisLogical(positionAxis) && writingMode.IsVertical())
    {
      logicalAxis = !logicalAxis;
    }
    return static_cast<LogicalBoxAxis>(logicalAxis);
  }
}
