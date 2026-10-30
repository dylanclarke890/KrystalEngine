#pragma once

#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"

namespace krys::boo
{
  /// @brief A grid area that spans in both rows' and columns' direction.
  class GridArea
  {
  public:
    GridSpan Columns;
    GridSpan Rows;

    // HashMap requires a default constructor.
    GridArea() noexcept : Columns(GridSpan::IndefiniteGridSpan()), Rows(GridSpan::IndefiniteGridSpan())
    {
    }

    GridArea(const GridSpan &rows, const GridSpan &columns) noexcept : Columns(columns), Rows(rows)
    {
    }

    KRYS_NODISCARD bool operator==(const GridArea &) const noexcept = default;

    KRYS_NODISCARD const GridSpan &Span(style::GridTrackSizingDirection direction) const noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? Columns : Rows;
    }
  };
}
