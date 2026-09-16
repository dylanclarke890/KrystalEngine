#pragma once

#include "Krystal.Booey/CSS/Values/Grid/GridSpan.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTrackSizingDirection.hpp"

namespace krys::boo::css
{
  class GridArea
  {
  public:
    GridArea() noexcept : columns(GridSpan::indefiniteGridSpan()), rows(GridSpan::indefiniteGridSpan())
    {
    }

    GridArea(const GridSpan &r, const GridSpan &c) noexcept : columns(c), rows(r)
    {
    }

    KRYS_NODISCARD bool operator==(const GridArea &) const noexcept = default;

    GridSpan columns;
    GridSpan rows;

    KRYS_NODISCARD const GridSpan &span(GridTrackSizingDirection direction) const noexcept
    {
      return direction == GridTrackSizingDirection::Columns ? columns : rows;
    }
  };
}