#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  enum class GridTrackSizingDirection : bool
  {
    Columns,
    Rows
  };

  KRYS_NODISCARD constexpr GridTrackSizingDirection
    OrthogonalDirection(GridTrackSizingDirection direction) noexcept
  {
    return direction == GridTrackSizingDirection::Columns ? GridTrackSizingDirection::Rows
                                                          : GridTrackSizingDirection::Columns;
  }
}