/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Rendering/Style/GridArea.hpp"
#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class GridTrackSizingAlgorithm;
  class RenderBox;
  class RenderGrid;

  /// @brief Masonry layout (grid lanes): places a masonry container's items in its grid axis tracks, each in
  /// the track that has the least content so far.
  class GridMasonryLayout
  {
    KRYS_NON_COPY_MOVABLE(GridMasonryLayout);

  public:
    enum class MasonryLayoutPhase : uint8
    {
      LayoutPhase,
      MinContentPhase,
      MaxContentPhase
    };

  private:
    uint32 _gridAxisTracksCount {0};

    List<LayoutUnit> _runningPositions;
    // Keyed by address: the offsets are cleared each time the items are placed.
    HashMap<const RenderBox *, LayoutUnit> _itemOffsets;
    RenderGrid &_renderGrid;
    LayoutUnit _masonryAxisGridGap;
    LayoutUnit _gridContentSize;

    style::GridTrackSizingDirection _masonryAxisDirection {style::GridTrackSizingDirection::Rows};
    const GridSpan _masonryAxisSpan = GridSpan::MasonryAxisTranslatedDefiniteGridSpan();

    uint32 _autoFlowNextCursor {0};

  public:
    explicit GridMasonryLayout(RenderGrid &renderGrid) noexcept : _renderGrid(renderGrid)
    {
    }

    void InitializeMasonry(uint32 gridAxisTracks,
                           style::GridTrackSizingDirection masonryAxisDirection) noexcept;
    void PerformMasonryPlacement(const GridTrackSizingAlgorithm &algorithm, uint32 gridAxisTracks,
                                 style::GridTrackSizingDirection masonryAxisDirection,
                                 MasonryLayoutPhase layoutPhase) noexcept;
    KRYS_NODISCARD LayoutUnit OffsetForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD LayoutUnit GridContentSize() const noexcept
    {
      return _gridContentSize;
    }
    KRYS_NODISCARD LayoutUnit GridGap() const noexcept
    {
      return _masonryAxisGridGap;
    }

  private:
    KRYS_NODISCARD GridArea GridAreaForIndefiniteGridAxisItem(const RenderBox &item) noexcept;
    KRYS_NODISCARD GridArea GridAreaForDefiniteGridAxisItem(const RenderBox &gridItem) const noexcept;

    void PlaceMasonryItems(const GridTrackSizingAlgorithm &algorithm,
                           MasonryLayoutPhase layoutPhase) noexcept;
    void SetItemGridAxisContainingBlockToGridArea(const GridTrackSizingAlgorithm &algorithm,
                                                  RenderBox &gridItem) noexcept;
    void InsertIntoGridAndLayoutItem(const GridTrackSizingAlgorithm &algorithm, RenderBox &gridItem,
                                     const GridArea &area, MasonryLayoutPhase layoutPhase) noexcept;
    KRYS_NODISCARD LayoutUnit CalculateMasonryIntrinsicLogicalWidth(RenderBox &gridItem,
                                                                    MasonryLayoutPhase layoutPhase) noexcept;

    void ResizeAndResetRunningPositions() noexcept;
    KRYS_NODISCARD LayoutUnit MasonryAxisMarginBoxForItem(const RenderBox &gridItem) noexcept;
    void UpdateRunningPositions(const RenderBox &gridItem, const GridArea &area) noexcept;
    void UpdateItemOffset(const RenderBox &gridItem, LayoutUnit offset) noexcept;
    KRYS_NODISCARD LayoutUnit MaxRunningPositionForSpan(uint32 startLine, uint32 spanLength) const noexcept;
    KRYS_NODISCARD style::GridTrackSizingDirection GridAxisDirection() const noexcept;

    KRYS_NODISCARD bool
      HasDefiniteGridAxisPosition(const RenderBox &gridItem,
                                  style::GridTrackSizingDirection gridAxisDirection) const noexcept;
    KRYS_NODISCARD GridArea MasonryGridAreaFromGridAxisSpan(const GridSpan &gridAxisSpan) const noexcept;
    KRYS_NODISCARD GridSpan GridAxisSpanFromArea(const GridArea &gridArea) const noexcept;
  };
}
