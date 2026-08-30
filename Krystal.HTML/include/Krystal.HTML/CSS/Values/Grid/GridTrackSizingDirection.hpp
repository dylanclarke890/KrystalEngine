#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"

namespace Krys::HTML
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