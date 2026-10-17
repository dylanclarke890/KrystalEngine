#include "Krystal.Booey/Rendering/GridMasonryLayout.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/GridTrackSizingAlgorithm.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPositionsResolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo
{
  using style::GridTrackSizingDirection;

  void GridMasonryLayout::InitializeMasonry(uint32 gridAxisTracks,
                                            GridTrackSizingDirection masonryAxisDirection) noexcept
  {
    // Reset global variables as they may contain state from previous runs of Masonry.
    _masonryAxisDirection = masonryAxisDirection;
    _masonryAxisGridGap = _renderGrid.GridGap(_masonryAxisDirection);
    _gridAxisTracksCount = gridAxisTracks;
    _gridContentSize = 0_lu;
    _itemOffsets.clear();

    _renderGrid.CurrentGrid().SetupGridForMasonryLayout();
    _renderGrid.PopulateExplicitGridAndOrderIterator();

    ResizeAndResetRunningPositions();
  }

  void GridMasonryLayout::PerformMasonryPlacement(const GridTrackSizingAlgorithm &algorithm,
                                                  uint32 gridAxisTracks,
                                                  GridTrackSizingDirection masonryAxisDirection,
                                                  MasonryLayoutPhase layoutPhase) noexcept
  {
    InitializeMasonry(gridAxisTracks, masonryAxisDirection);

    _renderGrid.PopulateGridPositionsForDirection(algorithm, GridTrackSizingDirection::Columns);
    _renderGrid.PopulateGridPositionsForDirection(algorithm, GridTrackSizingDirection::Rows);

    // 2.3 Masonry Layout Algorithm
    // https://drafts.csswg.org/css-grid-3/#masonry-layout-algorithm

    // the insertIntoGridAndLayoutItem() will modify the m_autoFlowNextCursor, so m_autoFlowNextCursor needs
    // to be reset.
    _autoFlowNextCursor = 0;

    PlaceMasonryItems(algorithm, layoutPhase);
  }

  void GridMasonryLayout::ResizeAndResetRunningPositions() noexcept
  {
    _runningPositions.assign(_gridAxisTracksCount, LayoutUnit());
  }

  void GridMasonryLayout::PlaceMasonryItems(const GridTrackSizingAlgorithm &algorithm,
                                            MasonryLayoutPhase layoutPhase) noexcept
  {
    if (!_gridAxisTracksCount)
    {
      return;
    }

    auto &grid = _renderGrid.CurrentGrid();
    for (auto *gridItem = grid.GetOrderIterator().First(); gridItem;
         gridItem = grid.GetOrderIterator().Next())
    {
      if (grid.GetOrderIterator().ShouldSkipChild(*gridItem))
      {
        continue;
      }

      auto gridArea = HasDefiniteGridAxisPosition(*gridItem, GridAxisDirection())
                        ? GridAreaForDefiniteGridAxisItem(*gridItem)
                        : GridAreaForIndefiniteGridAxisItem(*gridItem);
      InsertIntoGridAndLayoutItem(algorithm, *gridItem, gridArea, layoutPhase);
    }
  }

  GridArea GridMasonryLayout::GridAreaForDefiniteGridAxisItem(const RenderBox &gridItem) const noexcept
  {
    auto itemSpan = _renderGrid.CurrentGrid().GridItemSpan(gridItem, GridAxisDirection());
    krys_debug_assert(!itemSpan.IsIndefinite());
    itemSpan.Translate(_renderGrid.CurrentGrid().ExplicitGridStart(GridAxisDirection()));
    return MasonryGridAreaFromGridAxisSpan(itemSpan);
  }

  LayoutUnit GridMasonryLayout::CalculateMasonryIntrinsicLogicalWidth(RenderBox &gridItem,
                                                                      MasonryLayoutPhase layoutPhase) noexcept
  {
    switch (layoutPhase)
    {
      case MasonryLayoutPhase::MinContentPhase:
        return gridItem.ComputeIntrinsicLogicalWidthUsing(style::PreferredSize {css::keywords::MinContent {}},
                                                          0_lu, gridItem.BorderAndPaddingLogicalWidth());
      case MasonryLayoutPhase::MaxContentPhase:
        return gridItem.ComputeIntrinsicLogicalWidthUsing(style::PreferredSize {css::keywords::MaxContent {}},
                                                          0_lu, gridItem.BorderAndPaddingLogicalWidth());
      case MasonryLayoutPhase::LayoutPhase: krys_debug_assert(false); return {};
    }

    return {};
  }

  void GridMasonryLayout::SetItemGridAxisContainingBlockToGridArea(const GridTrackSizingAlgorithm &algorithm,
                                                                   RenderBox &gridItem) noexcept
  {
    if (auto direction = GridAxisDirection(); direction == GridTrackSizingDirection::Columns)
    {
      gridItem.SetGridAreaContentLogicalWidth(algorithm.GridAreaBreadthForGridItem(gridItem, direction));
    }
    else
    {
      gridItem.SetGridAreaContentLogicalHeight(algorithm.GridAreaBreadthForGridItem(gridItem, direction));
    }

    // FIXME(249230): Try to cache masonry layout sizes
    gridItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
  }

  void GridMasonryLayout::InsertIntoGridAndLayoutItem(const GridTrackSizingAlgorithm &algorithm,
                                                      RenderBox &gridItem, const GridArea &area,
                                                      MasonryLayoutPhase layoutPhase) noexcept
  {
    auto shouldOverrideLogicalWidth = [&](RenderBox &gridItem, MasonryLayoutPhase layoutPhase)
    {
      if (layoutPhase == MasonryLayoutPhase::LayoutPhase)
      {
        return false;
      }

      if (!(gridItem.Style().LogicalWidth().IsAuto() || gridItem.Style().LogicalWidth().IsPercent()))
      {
        return false;
      }

      krys_debug_assert(_renderGrid.IsMasonry(GridTrackSizingDirection::Columns));

      if (gridItem.Style().GetWritingMode().IsOrthogonal(_renderGrid.Style().GetWritingMode()))
      {
        return false;
      }

      if (auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem); renderGrid && renderGrid->IsSubgridRows())
      {
        return false;
      }

      return true;
    };

    if (shouldOverrideLogicalWidth(gridItem, layoutPhase))
    {
      gridItem.SetOverridingBorderBoxLogicalWidth(
        CalculateMasonryIntrinsicLogicalWidth(gridItem, layoutPhase));
    }

    _renderGrid.CurrentGrid().Insert(gridItem, area);
    SetItemGridAxisContainingBlockToGridArea(algorithm, gridItem);
    gridItem.LayoutIfNeeded();
    UpdateRunningPositions(gridItem, area);
    _autoFlowNextCursor = GridAxisSpanFromArea(area).EndLine() % _gridAxisTracksCount;
  }

  LayoutUnit GridMasonryLayout::MasonryAxisMarginBoxForItem(const RenderBox &gridItem) noexcept
  {
    LayoutUnit marginBoxSize;
    if (_masonryAxisDirection == GridTrackSizingDirection::Rows)
    {
      if (GridLayoutFunctions::IsOrthogonalGridItem(_renderGrid, gridItem))
      {
        marginBoxSize = gridItem.IsHorizontalWritingMode()
                          ? gridItem.Width() + gridItem.HorizontalMarginExtent()
                          : gridItem.Height() + gridItem.VerticalMarginExtent();
      }
      else
      {
        marginBoxSize = gridItem.LogicalHeight() + gridItem.MarginLogicalHeight();
      }
    }
    else
    {
      if (GridLayoutFunctions::IsOrthogonalGridItem(_renderGrid, gridItem))
      {
        marginBoxSize = gridItem.IsHorizontalWritingMode()
                          ? gridItem.Height() + gridItem.VerticalMarginExtent()
                          : gridItem.Width() + gridItem.HorizontalMarginExtent();
      }
      else
      {
        marginBoxSize = gridItem.LogicalWidth() + gridItem.MarginLogicalWidth();
      }
    }
    return marginBoxSize;
  }

  void GridMasonryLayout::UpdateRunningPositions(const RenderBox &gridItem, const GridArea &area) noexcept
  {
    auto gridAxisSpan = GridAxisSpanFromArea(area);
    krys_debug_assert(gridAxisSpan.StartLine() < _runningPositions.size()
                      && gridAxisSpan.EndLine() <= _runningPositions.size());
    gridAxisSpan.Clamp(static_cast<int>(_runningPositions.size()));

    LayoutUnit previousRunningPosition;
    for (auto line : gridAxisSpan)
    {
      previousRunningPosition = std::max(previousRunningPosition, _runningPositions[line]);
    }

    auto newRunningPosition =
      MasonryAxisMarginBoxForItem(gridItem) + previousRunningPosition + _masonryAxisGridGap;
    _gridContentSize = std::max(_gridContentSize, newRunningPosition - _masonryAxisGridGap);

    for (auto span : gridAxisSpan)
    {
      _runningPositions[span] = newRunningPosition;
    }

    UpdateItemOffset(gridItem, previousRunningPosition);
  }

  void GridMasonryLayout::UpdateItemOffset(const RenderBox &gridItem, LayoutUnit offset) noexcept
  {
    // We set() and not add() to update the value if the |gridItem| is already inserted
    _itemOffsets.insert_or_assign(&gridItem, offset);
  }

  LayoutUnit GridMasonryLayout::MaxRunningPositionForSpan(uint32 startLine, uint32 spanLength) const noexcept
  {
    LayoutUnit maxPosition;
    for (uint32 lineOffset = 0; lineOffset < spanLength; lineOffset++)
    {
      maxPosition = std::max(maxPosition, _runningPositions[startLine + lineOffset]);
    }
    return maxPosition;
  }

  GridArea GridMasonryLayout::GridAreaForIndefiniteGridAxisItem(const RenderBox &item) noexcept
  {
    // An item can't span more tracks than the grid axis has.
    auto itemSpanLength =
      std::min(style::GridPositionsResolver::SpanSizeForAutoPlacedItem(item, GridAxisDirection()),
               _gridAxisTracksCount);
    auto gridAxisLines = _gridAxisTracksCount + 1;

    // Get item-tolerance from the masonry container's style
    const auto &tolerance = _renderGrid.Style().GetItemTolerance();

    if (tolerance.IsInfinite())
    {
      // Infinite tolerance: place items strictly in order without considering track lengths
      // Use round-robin placement starting from the cursor position
      auto startingLine = _autoFlowNextCursor;

      // If the item doesn't fit at the cursor position, wrap to the beginning
      if (startingLine + itemSpanLength > _gridAxisTracksCount)
      {
        startingLine = 0;
      }

      auto gridAxisPosition =
        GridSpan::TranslatedDefiniteGridSpan(startingLine, startingLine + itemSpanLength);
      return MasonryGridAreaFromGridAxisSpan(gridAxisPosition);
    }

    // For normal and length-percentage tolerances, find positions within tolerance of the shortest track
    // Calculate tolerance value
    auto contentBoxSize = GridAxisDirection() == GridTrackSizingDirection::Columns
                            ? _renderGrid.ContentBoxLogicalHeight()
                            : _renderGrid.ContentBoxLogicalWidth();

    LayoutUnit toleranceValue = tolerance.SwitchOn(
      [&](const css::keywords::Normal &) -> LayoutUnit
      {
        // Normal resolves to 1em
        return LayoutUnit {_renderGrid.Style().ComputedFontSize()};
      },
      [&](const style::ItemTolerance::Fixed &fixed) -> LayoutUnit
      { return LayoutUnit {fixed.ResolveZoom(_renderGrid.Style().UsedZoomForLength())}; },
      [&](const style::ItemTolerance::Percentage &percentage) -> LayoutUnit
      { return style::Evaluate<LayoutUnit>(percentage, contentBoxSize); },
      [&](const style::ItemTolerance::Calc &calc) -> LayoutUnit
      { return style::Evaluate<LayoutUnit>(calc, contentBoxSize, _renderGrid.Style().UsedZoomForLength()); },
      [](const css::keywords::Infinite &) -> LayoutUnit
      {
        // This case shouldn't be reached due to the outer if check
        krys_debug_assert(false);
        return LayoutUnit {};
      });

    // Step 1: Find the absolute shortest position across all tracks
    auto maxStartingLine = gridAxisLines - itemSpanLength;
    LayoutUnit absoluteShortest = LayoutUnit::Max();
    for (uint32 i = 0; i < maxStartingLine; i++)
    {
      absoluteShortest = std::min(absoluteShortest, MaxRunningPositionForSpan(i, itemSpanLength));
    }

    // Step 2: Find first position within tolerance of shortest, starting from the cursor position.
    uint32 smallestMaxPosLine = 0;
    auto autoFlowNextCursorShift = (_autoFlowNextCursor > maxStartingLine) ? 0 : _autoFlowNextCursor;
    for (uint32 i = 0; i < maxStartingLine; i++)
    {
      auto startingLine = (autoFlowNextCursorShift + i) % maxStartingLine;

      auto maxPosForCurrentStartingLine = MaxRunningPositionForSpan(startingLine, itemSpanLength);

      // Accept first position within tolerance of the absolute shortest
      if (maxPosForCurrentStartingLine <= absoluteShortest + toleranceValue)
      {
        smallestMaxPosLine = startingLine;
        break;
      }
    }

    auto gridAxisPosition =
      GridSpan::TranslatedDefiniteGridSpan(smallestMaxPosLine, smallestMaxPosLine + itemSpanLength);
    return MasonryGridAreaFromGridAxisSpan(gridAxisPosition);
  }

  LayoutUnit GridMasonryLayout::OffsetForGridItem(const RenderBox &gridItem) const noexcept
  {
    const auto offsetIter = _itemOffsets.find(&gridItem);
    if (offsetIter == _itemOffsets.end())
    {
      return 0_lu;
    }
    return offsetIter->second;
  }

  GridTrackSizingDirection GridMasonryLayout::GridAxisDirection() const noexcept
  {
    // The masonry axis and grid axis can never be the same.
    // They are always perpendicular to each other.
    return style::OrthogonalDirection(_masonryAxisDirection);
  }

  bool
    GridMasonryLayout::HasDefiniteGridAxisPosition(const RenderBox &gridItem,
                                                   GridTrackSizingDirection gridAxisDirection) const noexcept
  {
    return !style::GridPositionsResolver::ResolveGridPositionsFromStyle(_renderGrid, gridItem,
                                                                        gridAxisDirection)
              .IsIndefinite();
  }

  GridSpan GridMasonryLayout::GridAxisSpanFromArea(const GridArea &gridArea) const noexcept
  {
    return gridArea.Span(GridAxisDirection());
  }

  GridArea GridMasonryLayout::MasonryGridAreaFromGridAxisSpan(const GridSpan &gridAxisSpan) const noexcept
  {
    return _masonryAxisDirection == GridTrackSizingDirection::Rows
             ? GridArea {_masonryAxisSpan, gridAxisSpan}
             : GridArea {gridAxisSpan, _masonryAxisSpan};
  }
}
