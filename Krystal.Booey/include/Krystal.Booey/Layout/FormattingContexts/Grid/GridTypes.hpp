/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/Geometry/RectEdges.hpp"
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
