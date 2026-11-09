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

#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::layout
{
  class ElementBox;

  /// @brief A grid item before placement: its layout box and grid positions, normalised to non-negative
  /// matrix indices once the grid's offsets are known.
  class UnplacedGridItem
  {
    friend class GridFormattingContext;
    friend class GridLayout;
    friend class PlacedGridItem;

  private:
    const ElementBox *_layoutBox;
    // https://drafts.csswg.org/css-grid-1/#typedef-grid-row-start-grid-line
    Pair<style::GridPosition, style::GridPosition> _columnPosition;
    Pair<style::GridPosition, style::GridPosition> _rowPosition;

    // Offsets applied to normalize negative grid positions to non-negative matrix indices.
    size_t _rowNormalizationOffset {0};
    size_t _columnNormalizationOffset {0};

    // Flag to track whether applyGridOffsets() has been called.
    // This helps catch bugs where normalized methods are used before offsets are applied,
    // or where offsets are applied multiple times.
    bool _hasAppliedGridOffsets {false};

  public:
    UnplacedGridItem(const ElementBox &layoutBox, style::GridPosition columnStart,
                     style::GridPosition columnEnd, style::GridPosition rowStart,
                     style::GridPosition rowEnd) noexcept;

    KRYS_NODISCARD const ElementBox &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }

    KRYS_NODISCARD size_t NormalizedColumnStart() const noexcept;
    KRYS_NODISCARD size_t NormalizedColumnEnd() const noexcept;
    KRYS_NODISCARD size_t NormalizedRowStart() const noexcept;
    KRYS_NODISCARD size_t NormalizedRowEnd() const noexcept;

    KRYS_NODISCARD bool HasDefiniteRowPosition() const noexcept;
    KRYS_NODISCARD bool HasDefiniteColumnPosition() const noexcept;
    KRYS_NODISCARD bool HasAutoColumnPosition() const noexcept;

    KRYS_NODISCARD size_t ColumnSpanSize() const noexcept;

    KRYS_NODISCARD Pair<size_t, size_t> NormalizedRowStartEnd() const noexcept;
    KRYS_NODISCARD Pair<size_t, size_t> NormalizedColumnStartEnd() const noexcept;

  private:
    // The grammar for <grid-line>, which is used by the grid-{column, row}-{start-end}
    // placement properties is 1-index in regards to line numbers. To allow for easy
    // indexing from these line numbers into our structures we subtract 1 from them
    // into these helper functions to make them 0-index. For example, grid-column-start: 1
    // and grid-column-end: 2 would make to [0, 1] and place the grid item into
    // Grid[rowIndex][0].
    KRYS_NODISCARD int ExplicitColumnStart() const noexcept;
    KRYS_NODISCARD int ExplicitColumnEnd() const noexcept;
    KRYS_NODISCARD int ExplicitRowStart() const noexcept;
    KRYS_NODISCARD int ExplicitRowEnd() const noexcept;

    KRYS_NODISCARD Pair<int, int> DefiniteRowStartEnd() const noexcept;
    KRYS_NODISCARD Pair<int, int> DefiniteColumnStartEnd() const noexcept;

    void ApplyGridOffsets(size_t rowOffset, size_t columnOffset) noexcept;
  };

  // https://drafts.csswg.org/css-grid-1/#auto-placement-algo
  /// @brief A grid's items, grouped by the step of the auto-placement algorithm that places them.
  struct UnplacedGridItems
  {
    // 1. Position anything that's not auto-positioned.
    List<UnplacedGridItem> NonAutoPositionedItems;
    // 2. Process the items locked to a given row.
    List<UnplacedGridItem> DefiniteRowPositionedItems;
    // 4. Position the remaining grid items.
    List<UnplacedGridItem> AutoPositionedItems;
  };
}
