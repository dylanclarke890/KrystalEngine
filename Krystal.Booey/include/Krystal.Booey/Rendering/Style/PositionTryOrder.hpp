#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"

namespace krys::boo::style
{
  // https://drafts.csswg.org/css-anchor-position-1/#position-try-order-property
  enum class PositionTryOrder : uint8
  {
    Normal,
    MostWidth,
    MostHeight,
    MostBlockSize,
    MostInlineSize
  };

  KRYS_NODISCARD LogicalBoxAxis BoxAxisForPositionTryOrder(PositionTryOrder, WritingMode) noexcept;
}
