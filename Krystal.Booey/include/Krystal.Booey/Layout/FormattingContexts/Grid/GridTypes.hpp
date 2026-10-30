#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackBreadth.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::layout
{
  class ElementBox;
  class PlacedGridItem;
  class UnplacedGridItem;

  // https://drafts.csswg.org/css-grid-1/#grid-area
  /// @brief The lines a grid item's area starts and ends at, in each axis.
  struct GridAreaLines
  {
    size_t ColumnStartLine;
    size_t ColumnEndLine;
    size_t RowStartLine;
    size_t RowEndLine;
  };

  /// @brief A grid item's border box and margins, in its grid area's coordinate space until mapped to the
  /// grid.
  struct GridItemRect
  {
    LayoutRect BorderBoxRect;
    RectEdges<LayoutUnit> Margins;
    GridAreaLines LineNumbersForGridArea;
    const ElementBox *LayoutBox;
  };

  /// @brief A track's min and max track sizing functions.
  struct TrackSizingFunctions
  {
    style::GridTrackBreadth Min {css::keywords::Auto {}};
    style::GridTrackBreadth Max {css::keywords::Auto {}};
  };

  using BorderBoxPositions = List<LayoutUnit>;
  // In the order the items are found in the grid, row by row.
  using GridAreas = List<Pair<UnplacedGridItem, GridAreaLines>>;
  using GridItemRects = List<GridItemRect>;
  using PlacedGridItems = List<PlacedGridItem>;
  using TrackSizes = List<LayoutUnit>;
  using TrackSizingFunctionsList = List<TrackSizingFunctions>;
  using UsedBlockSizes = List<LayoutUnit>;
  using UsedInlineSizes = List<LayoutUnit>;

  /// @brief The used sizes of a grid's columns and rows.
  struct UsedTrackSizes
  {
    TrackSizes ColumnSizes;
    TrackSizes RowSizes;
  };
}
