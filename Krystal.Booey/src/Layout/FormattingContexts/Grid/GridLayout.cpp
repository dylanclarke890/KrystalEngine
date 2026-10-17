#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/PlacedGridItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>

namespace krys::boo::layout
{
  namespace
  {
    KRYS_NODISCARD LayoutUnit FixedMargin(const style::MarginEdge &margin,
                                          const style::ZoomFactor &zoomFactor) noexcept
    {
      if (auto fixedMargin = margin.TryFixed())
      {
        return style::Evaluate<LayoutUnit>(*fixedMargin, zoomFactor);
      }

      // FIXME: Not implemented yet.
      krys_debug_assert(false);
      return {};
    }
  }

  GridLayout::GridLayout(const GridFormattingContext &gridFormattingContext) noexcept
      : _gridFormattingContext(gridFormattingContext)
  {
  }

  GridDimensions GridLayout::CalculateGridDimensions(const UnplacedGridItems &unplacedGridItems,
                                                     size_t explicitColumnsCount,
                                                     size_t explicitRowsCount) noexcept
  {
    int minimumRowIndex = 0;
    int minimumColumnIndex = 0;
    auto maximumRowIndex = static_cast<int>(explicitRowsCount);
    auto maximumColumnIndex = static_cast<int>(explicitColumnsCount);

    auto updateGridBounds = [&](const UnplacedGridItem &item)
    {
      if (item.HasDefiniteRowPosition())
      {
        auto [rowStart, rowEnd] = item.DefiniteRowStartEnd();
        minimumRowIndex = std::min({minimumRowIndex, rowStart, rowEnd});
        maximumRowIndex = std::max({maximumRowIndex, rowStart, rowEnd});
      }

      if (item.HasDefiniteColumnPosition())
      {
        auto [columnStart, columnEnd] = item.DefiniteColumnStartEnd();
        minimumColumnIndex = std::min({minimumColumnIndex, columnStart, columnEnd});
        maximumColumnIndex = std::max({maximumColumnIndex, columnStart, columnEnd});
      }
    };

    for (const auto &item : unplacedGridItems.NonAutoPositionedItems)
    {
      updateGridBounds(item);
    }
    for (const auto &item : unplacedGridItems.DefiniteRowPositionedItems)
    {
      updateGridBounds(item);
    }

    auto rowOffset = minimumRowIndex < 0 ? static_cast<size_t>(-minimumRowIndex) : 0;
    auto columnOffset = minimumColumnIndex < 0 ? static_cast<size_t>(-minimumColumnIndex) : 0;

    return {rowOffset, columnOffset, static_cast<size_t>(maximumColumnIndex) + columnOffset,
            static_cast<size_t>(maximumRowIndex) + rowOffset};
  }

  // 8.5. Grid Item Placement Algorithm.
  // https://drafts.csswg.org/css-grid-1/#auto-placement-algo
  GridLayout::PlacementResult
    GridLayout::PlaceGridItems(UnplacedGridItems &unplacedGridItems,
                               const List<style::GridTrackSize> &gridTemplateColumnsTrackSizes,
                               const List<style::GridTrackSize> &gridTemplateRowsTrackSizes,
                               GridAutoFlowOptions autoFlowOptions) noexcept
  {
    // Calculate grid dimensions (offsets and total size) for negative grid line positions
    auto gridDimensions = CalculateGridDimensions(unplacedGridItems, gridTemplateColumnsTrackSizes.size(),
                                                  gridTemplateRowsTrackSizes.size());

    // Normalize all grid item positions by applying the offsets
    for (auto &item : unplacedGridItems.NonAutoPositionedItems)
    {
      item.ApplyGridOffsets(gridDimensions.RowOffset, gridDimensions.ColumnOffset);
    }
    for (auto &item : unplacedGridItems.DefiniteRowPositionedItems)
    {
      item.ApplyGridOffsets(gridDimensions.RowOffset, gridDimensions.ColumnOffset);
    }
    for (auto &item : unplacedGridItems.AutoPositionedItems)
    {
      item.ApplyGridOffsets(gridDimensions.RowOffset, gridDimensions.ColumnOffset);
    }

    ImplicitGrid implicitGrid(gridDimensions.TotalColumns, gridDimensions.TotalRows);

    // 1. Position anything that's not auto-positioned.
    for (const auto &nonAutoPositionedItem : unplacedGridItems.NonAutoPositionedItems)
    {
      implicitGrid.InsertUnplacedGridItem(nonAutoPositionedItem);
    }

    // 2. Process the items locked to a given row.
    // Phase 1: Only single-cell items within explicit grid bounds
    HashMap<size_t, size_t> rowCursors;
    for (const auto &definiteRowPositionedItem : unplacedGridItems.DefiniteRowPositionedItems)
    {
      implicitGrid.InsertDefiniteRowItem(definiteRowPositionedItem, autoFlowOptions, rowCursors);
    }

    // FIXME: 3. Process auto-positioned items.
    krys_debug_assert(unplacedGridItems.AutoPositionedItems.empty());

    return {implicitGrid.GetGridAreas(), implicitGrid.ColumnsCount(), implicitGrid.RowsCount()};
  }

  // https://drafts.csswg.org/css-grid-1/#layout-algorithm
  Pair<UsedTrackSizes, GridItemRects> GridLayout::Layout(GridLayoutConstraints,
                                                         UnplacedGridItems &unplacedGridItems) noexcept
  {
    const auto &gridContainerStyle = GridContainerStyle();
    const auto &gridTemplateColumnsTrackSizes = gridContainerStyle.GridTemplateColumns().Sizes;
    const auto &gridTemplateRowsTrackSizes = gridContainerStyle.GridTemplateRows().Sizes;

    // 1. Run the Grid Item Placement Algorithm to resolve the placement of all grid items in the grid.
    auto gridAutoFlow = gridContainerStyle.GetGridAutoFlow();
    auto autoFlowOptions =
      GridAutoFlowOptions {gridAutoFlow.IsDense() ? PackingStrategy::Dense : PackingStrategy::Sparse,
                           gridAutoFlow.IsRow() ? GridAutoFlowDirection::Row : GridAutoFlowDirection::Column};
    auto [gridAreas, columnsCount, rowsCount] = PlaceGridItems(
      unplacedGridItems, gridTemplateColumnsTrackSizes, gridTemplateRowsTrackSizes, autoFlowOptions);
    auto placedGridItems = FormattingContext().ConstructPlacedGridItems(gridAreas);

    auto columnTrackSizingFunctionsList =
      ComputeTrackSizingFunctions(columnsCount, gridTemplateColumnsTrackSizes);
    auto rowTrackSizingFunctionsList = ComputeTrackSizingFunctions(rowsCount, gridTemplateRowsTrackSizes);

    // 3. Given the resulting grid container size, run the Grid Sizing Algorithm to size the grid.
    auto usedTrackSizes = PerformGridSizingAlgorithm(placedGridItems, columnTrackSizingFunctionsList,
                                                     rowTrackSizingFunctionsList);

    // 4. Lay out the grid items into their respective containing blocks. Each grid area’s
    // width and height are considered definite for this purpose.
    auto [usedInlineSizes, usedBlockSizes] = LayoutGridItems(placedGridItems, usedTrackSizes);

    // https://drafts.csswg.org/css-grid-1/#alignment
    auto zoomFactor = gridContainerStyle.UsedZoomForLength();
    auto usedInlineMargins = ComputeInlineMargins(placedGridItems, zoomFactor);
    auto usedBlockMargins = ComputeBlockMargins(placedGridItems, zoomFactor);

    // https://drafts.csswg.org/css-grid-1/#alignment
    // After a grid container’s grid tracks have been sized, and the dimensions of all grid items
    // are finalized, grid items can be aligned within their grid areas.
    auto inlineAxisPositions = PerformInlineAxisSelfAlignment(placedGridItems, usedInlineMargins);
    auto blockAxisPositions = PerformBlockAxisSelfAlignment(placedGridItems, usedBlockMargins);

    GridItemRects gridItemRects;
    gridItemRects.reserve(placedGridItems.size());
    for (size_t gridItemIndex = 0; gridItemIndex < placedGridItems.size(); ++gridItemIndex)
    {
      auto borderBoxRect = LayoutRect {inlineAxisPositions[gridItemIndex], blockAxisPositions[gridItemIndex],
                                       usedInlineSizes[gridItemIndex], usedBlockSizes[gridItemIndex]};

      const auto &gridItemInlineMargins = usedInlineMargins[gridItemIndex];
      const auto &gridItemBlockMargins = usedBlockMargins[gridItemIndex];
      auto marginEdges =
        RectEdges<LayoutUnit> {gridItemBlockMargins.MarginStart, gridItemInlineMargins.MarginEnd,
                               gridItemBlockMargins.MarginEnd, gridItemInlineMargins.MarginStart};

      const auto &placedGridItem = placedGridItems[gridItemIndex];
      gridItemRects.push_back(
        {borderBoxRect, marginEdges, placedGridItem.GetGridAreaLines(), &placedGridItem.LayoutBox()});
    }

    return {krys::move(usedTrackSizes), krys::move(gridItemRects)};
  }

  BorderBoxPositions
    GridLayout::PerformInlineAxisSelfAlignment(const PlacedGridItems &placedGridItems,
                                               const List<UsedMargins> &inlineMargins) noexcept
  {
    BorderBoxPositions borderBoxPositions;
    borderBoxPositions.reserve(placedGridItems.size());

    auto computeMarginBoxPosition = [](const PlacedGridItem &placedGridItem) -> LayoutUnit
    {
      switch (placedGridItem.InlineAxisAlignment().Position())
      {
        case ItemPosition::FlexStart:
        case ItemPosition::SelfStart:
        case ItemPosition::Start:     return {};

        // https://www.w3.org/TR/css-align-3/#justify-grid
        // Sizes as either stretch (typical non-replaced elements) or start (typical replaced elements);
        // see Grid Item Sizing in [CSS-GRID-1]. The resulting box is then start-aligned.
        //
        // Stretching should be handled by GridLayout::layoutGridItems.
        case ItemPosition::Normal:
        case ItemPosition::Stretch:   return {};
        default:
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return {};
      }
    };

    for (size_t gridItemIndex = 0; gridItemIndex < placedGridItems.size(); ++gridItemIndex)
    {
      auto marginBoxPosition = computeMarginBoxPosition(placedGridItems[gridItemIndex]);
      borderBoxPositions.push_back(marginBoxPosition + inlineMargins[gridItemIndex].MarginStart);
    }

    return borderBoxPositions;
  }

  BorderBoxPositions GridLayout::PerformBlockAxisSelfAlignment(const PlacedGridItems &placedGridItems,
                                                               const List<UsedMargins> &blockMargins) noexcept
  {
    BorderBoxPositions borderBoxPositions;
    borderBoxPositions.reserve(placedGridItems.size());

    auto computeMarginBoxPosition = [](const PlacedGridItem &placedGridItem) -> LayoutUnit
    {
      switch (placedGridItem.BlockAxisAlignment().Position())
      {
        case ItemPosition::FlexStart:
        case ItemPosition::SelfStart:
        case ItemPosition::Start:     return {};

        // Items have a fixed block size, so normal and stretch size as start and the box is start-aligned.
        case ItemPosition::Normal:
        case ItemPosition::Stretch:   return {};
        default:
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return {};
      }
    };

    for (size_t gridItemIndex = 0; gridItemIndex < placedGridItems.size(); ++gridItemIndex)
    {
      auto marginBoxPosition = computeMarginBoxPosition(placedGridItems[gridItemIndex]);
      borderBoxPositions.push_back(marginBoxPosition + blockMargins[gridItemIndex].MarginStart);
    }

    return borderBoxPositions;
  }

  TrackSizingFunctionsList
    GridLayout::ComputeTrackSizingFunctions(size_t implicitGridTracksCount,
                                            const List<style::GridTrackSize> &gridTemplateTrackSizes) noexcept
  {
    // FIXME: Support implicit tracks (both before and after the explicit grid)
    krys_debug_assert_msg(implicitGridTracksCount == gridTemplateTrackSizes.size(),
                          "Currently only support mapping track sizes from explicit grid from "
                          "grid-template-{columns, rows}");
    (void)implicitGridTracksCount;

    // https://drafts.csswg.org/css-grid-1/#algo-terms
    TrackSizingFunctionsList trackSizingFunctionsList;
    trackSizingFunctionsList.reserve(gridTemplateTrackSizes.size());
    for (const auto &gridTrackSize : gridTemplateTrackSizes)
    {
      auto minTrackSizingFunction = [&]() -> style::GridTrackBreadth
      {
        // If the track was sized with a minmax() function, this is the first argument to that function.
        if (gridTrackSize.IsMinMax())
        {
          return gridTrackSize.MinTrackBreadth();
        }

        // If the track was sized with a <flex> value or fit-content() function, auto.
        if (gridTrackSize.IsFitContent() || gridTrackSize.MinTrackBreadth().IsFlex())
        {
          return css::keywords::Auto {};
        }

        // Otherwise, the track’s sizing function.
        return gridTrackSize.MinTrackBreadth();
      };

      auto maxTrackSizingFunction = [&]() -> style::GridTrackBreadth
      {
        // If the track was sized with a minmax() function, this is the second argument to that function.
        if (gridTrackSize.IsMinMax())
        {
          return gridTrackSize.MaxTrackBreadth();
        }

        // Otherwise, the track’s sizing function. In all cases, treat auto and fit-content() as max-content,
        // except where specified otherwise for fit-content().
        if (gridTrackSize.MaxTrackBreadth().IsAuto())
        {
          return css::keywords::MaxContent {};
        }

        if (gridTrackSize.IsFitContent())
        {
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return css::keywords::MaxContent {};
        }

        return gridTrackSize.MaxTrackBreadth();
      };

      trackSizingFunctionsList.push_back({minTrackSizingFunction(), maxTrackSizingFunction()});
    }
    return trackSizingFunctionsList;
  }

  // https://www.w3.org/TR/css-grid-1/#algo-grid-sizing
  UsedTrackSizes GridLayout::PerformGridSizingAlgorithm(
    const PlacedGridItems &placedGridItems, const TrackSizingFunctionsList &columnTrackSizingFunctionsList,
    const TrackSizingFunctionsList &rowTrackSizingFunctionsList) noexcept
  {
    // 1. First, the track sizing algorithm is used to resolve the sizes of the grid columns.
    auto columnSizes = TrackSizingAlgorithm::SizeTracks(placedGridItems, columnTrackSizingFunctionsList);

    // 2. Next, the track sizing algorithm resolves the sizes of the grid rows.
    auto rowSizes = TrackSizingAlgorithm::SizeTracks(placedGridItems, rowTrackSizingFunctionsList);

    // FIXME: 3. Then, if the min-content contribution of any grid item has changed based on the
    // row sizes and alignment calculated in step 2, re-resolve the sizes of the grid
    // columns with the new min-content and max-content contributions (once only).

    // FIXME: 4. Next, if the min-content contribution of any grid item has changed based on the
    // column sizes and alignment calculated in step 3, re-resolve the sizes of the grid
    // rows with the new min-content and max-content contributions (once only).

    return {krys::move(columnSizes), krys::move(rowSizes)};
  }

  // https://drafts.csswg.org/css-grid-1/#auto-margins
  List<GridLayout::UsedMargins> GridLayout::ComputeInlineMargins(const PlacedGridItems &placedGridItems,
                                                                 const style::ZoomFactor &zoomFactor) noexcept
  {
    List<UsedMargins> usedMargins;
    usedMargins.reserve(placedGridItems.size());
    for (const auto &placedGridItem : placedGridItems)
    {
      const auto &inlineAxisSizes = placedGridItem.InlineAxisSizes();
      usedMargins.push_back({FixedMargin(inlineAxisSizes.MarginStart, zoomFactor),
                             FixedMargin(inlineAxisSizes.MarginEnd, zoomFactor)});
    }
    return usedMargins;
  }

  // https://drafts.csswg.org/css-grid-1/#auto-margins
  List<GridLayout::UsedMargins> GridLayout::ComputeBlockMargins(const PlacedGridItems &placedGridItems,
                                                                const style::ZoomFactor &zoomFactor) noexcept
  {
    List<UsedMargins> usedMargins;
    usedMargins.reserve(placedGridItems.size());
    for (const auto &placedGridItem : placedGridItems)
    {
      const auto &blockAxisSizes = placedGridItem.BlockAxisSizes();
      usedMargins.push_back({FixedMargin(blockAxisSizes.MarginStart, zoomFactor),
                             FixedMargin(blockAxisSizes.MarginEnd, zoomFactor)});
    }
    return usedMargins;
  }

  // https://drafts.csswg.org/css-grid-1/#grid-item-sizing
  Pair<UsedInlineSizes, UsedBlockSizes>
    GridLayout::LayoutGridItems(const PlacedGridItems &placedGridItems,
                                const UsedTrackSizes &usedTrackSizes) const noexcept
  {
    UsedInlineSizes usedInlineSizes;
    UsedBlockSizes usedBlockSizes;
    auto gridItemsCount = placedGridItems.size();
    usedInlineSizes.reserve(gridItemsCount);
    usedBlockSizes.reserve(gridItemsCount);

    const auto &formattingContext = FormattingContext();
    const auto &integrationUtils = formattingContext.GetIntegrationUtils();
    auto columnsGap = GridLayoutUtils::ComputeGapValue(formattingContext.Root().Style().ColumnGap());
    for (const auto &gridItem : placedGridItems)
    {
      const auto &gridItemBoxGeometry = formattingContext.GeometryForGridItem(gridItem.LayoutBox());

      auto usedInlineSizeForGridItem = GridLayoutUtils::UsedInlineSizeForGridItem(
        gridItem, gridItemBoxGeometry.HorizontalBorderAndPadding(), usedTrackSizes.ColumnSizes, columnsGap);
      usedInlineSizes.push_back(usedInlineSizeForGridItem);

      auto usedBlockSizeForGridItem =
        GridLayoutUtils::UsedBlockSizeForGridItem(gridItem) + gridItemBoxGeometry.VerticalBorderAndPadding();
      usedBlockSizes.push_back(usedBlockSizeForGridItem);

      integrationUtils.LayoutWithFormattingContextForBox(gridItem.LayoutBox(), usedInlineSizeForGridItem,
                                                         usedBlockSizeForGridItem);
    }
    return {krys::move(usedInlineSizes), krys::move(usedBlockSizes)};
  }

  const ElementBox &GridLayout::GridContainer() const noexcept
  {
    return _gridFormattingContext.Root();
  }

  const RenderStyle &GridLayout::GridContainerStyle() const noexcept
  {
    return GridContainer().Style();
  }
}
