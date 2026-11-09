/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Text/WritingMode.hpp"

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
