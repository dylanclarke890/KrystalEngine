#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Grid/UnplacedGridItem.hpp"
#include "Krystal.Booey/Style/Values/Align/GapGutter.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderStyle;

  namespace style
  {
    struct ZoomFactor;
  }
}

namespace krys::boo::layout
{
  class GridFormattingContext;

  enum class PackingStrategy : bool
  {
    Sparse,
    Dense
  };

  enum class GridAutoFlowDirection : bool
  {
    Row,
    Column
  };

  struct GridAutoFlowOptions
  {
    PackingStrategy Strategy;
    GridAutoFlowDirection Direction;
  };

  struct GridLayoutConstraints
  {
    Maybe<LayoutUnit> InlineAxisAvailableSpace;
    Maybe<LayoutUnit> BlockAxisAvailableSpace;
  };

  /// @brief The size of the implicit grid and the offsets that turn negative lines into matrix indices.
  struct GridDimensions
  {
    size_t RowOffset {0};
    size_t ColumnOffset {0};
    size_t TotalColumns {0};
    size_t TotalRows {0};
  };

  // https://drafts.csswg.org/css-grid-1/#implicit-grids
  /// @brief The implicit grid: the explicit grid and the tracks placement adds, as a matrix of the items in
  /// each cell.
  class ImplicitGrid
  {
  private:
    using GridCell = List<UnplacedGridItem>;
    using GridMatrix = List<List<GridCell>>;

    GridMatrix _gridMatrix;

  public:
    ImplicitGrid(size_t totalColumnsCount, size_t totalRowsCount) noexcept;

    KRYS_NODISCARD size_t RowsCount() const noexcept
    {
      return _gridMatrix.size();
    }
    KRYS_NODISCARD size_t ColumnsCount() const noexcept
    {
      return RowsCount() ? _gridMatrix[0].size() : 0;
    }

    void InsertUnplacedGridItem(const UnplacedGridItem &unplacedGridItem) noexcept;
    void InsertDefiniteRowItem(const UnplacedGridItem &unplacedGridItem, GridAutoFlowOptions autoFlowOptions,
                               HashMap<size_t, size_t> &rowCursors) noexcept;

    KRYS_NODISCARD GridAreas GetGridAreas() const noexcept;

  private:
    KRYS_NODISCARD Maybe<size_t> FindFirstAvailableColumnPosition(size_t rowStart, size_t rowEnd,
                                                                  size_t columnSpan,
                                                                  size_t startSearchColumn) const noexcept;
    KRYS_NODISCARD bool IsCellRangeEmpty(size_t columnStart, size_t columnEnd, size_t rowStart,
                                         size_t rowEnd) const noexcept;
    void InsertItemInArea(const UnplacedGridItem &unplacedGridItem, size_t columnStart, size_t columnEnd,
                          size_t rowStart, size_t rowEnd) noexcept;
  };

  /// @brief The track sizing algorithm of the grid formatting context.
  // https://drafts.csswg.org/css-grid-1/#algo-track-sizing
  class TrackSizingAlgorithm
  {
  public:
    KRYS_NODISCARD static TrackSizes
      SizeTracks(const PlacedGridItems &placedGridItems,
                 const TrackSizingFunctionsList &trackSizingFunctions) noexcept;
  };

  namespace GridLayoutUtils
  {
    KRYS_NODISCARD LayoutUnit ComputeGapValue(const style::GapGutter &gap) noexcept;
    KRYS_NODISCARD LayoutUnit UsedInlineSizeForGridItem(const PlacedGridItem &placedGridItem,
                                                        LayoutUnit borderAndPadding,
                                                        const TrackSizes &usedColumnSizes,
                                                        LayoutUnit columnsGap) noexcept;
    KRYS_NODISCARD LayoutUnit UsedBlockSizeForGridItem(const PlacedGridItem &placedGridItem) noexcept;
    /// @brief The position of the start of the track after grid line @p gridLineIndex.
    KRYS_NODISCARD LayoutUnit ComputeGridLinePosition(size_t gridLineIndex, const TrackSizes &trackSizes,
                                                      LayoutUnit gap) noexcept;
    /// @brief The position of the end of the track before grid line @p gridLineIndex.
    KRYS_NODISCARD LayoutUnit ComputeGridLineEndPosition(size_t gridLineIndex, const TrackSizes &trackSizes,
                                                         LayoutUnit gap) noexcept;
  }

  /// @brief Runs the grid layout algorithm over a grid's items: placement, track sizing, item sizing and
  /// alignment.
  class GridLayout
  {
  private:
    struct UsedMargins
    {
      LayoutUnit MarginStart;
      LayoutUnit MarginEnd;
    };

    struct PlacementResult
    {
      GridAreas Areas;
      size_t ColumnsCount;
      size_t RowsCount;
    };

    const GridFormattingContext &_gridFormattingContext;

  public:
    explicit GridLayout(const GridFormattingContext &gridFormattingContext) noexcept;

    KRYS_NODISCARD Pair<UsedTrackSizes, GridItemRects> Layout(GridLayoutConstraints layoutConstraints,
                                                              UnplacedGridItems &unplacedGridItems) noexcept;

  private:
    KRYS_NODISCARD PlacementResult PlaceGridItems(
      UnplacedGridItems &unplacedGridItems, const List<style::GridTrackSize> &gridTemplateColumnsTrackSizes,
      const List<style::GridTrackSize> &gridTemplateRowsTrackSizes,
      GridAutoFlowOptions autoFlowOptions) noexcept;
    KRYS_NODISCARD static GridDimensions CalculateGridDimensions(const UnplacedGridItems &unplacedGridItems,
                                                                 size_t explicitColumnsCount,
                                                                 size_t explicitRowsCount) noexcept;
    KRYS_NODISCARD static TrackSizingFunctionsList
      ComputeTrackSizingFunctions(size_t implicitGridTracksCount,
                                  const List<style::GridTrackSize> &gridTemplateTrackSizes) noexcept;
    KRYS_NODISCARD static UsedTrackSizes
      PerformGridSizingAlgorithm(const PlacedGridItems &placedGridItems,
                                 const TrackSizingFunctionsList &columnTrackSizingFunctionsList,
                                 const TrackSizingFunctionsList &rowTrackSizingFunctionsList) noexcept;
    KRYS_NODISCARD Pair<UsedInlineSizes, UsedBlockSizes>
      LayoutGridItems(const PlacedGridItems &placedGridItems,
                      const UsedTrackSizes &usedTrackSizes) const noexcept;
    KRYS_NODISCARD static List<UsedMargins>
      ComputeInlineMargins(const PlacedGridItems &placedGridItems,
                           const style::ZoomFactor &zoomFactor) noexcept;
    KRYS_NODISCARD static List<UsedMargins> ComputeBlockMargins(const PlacedGridItems &placedGridItems,
                                                                const style::ZoomFactor &zoomFactor) noexcept;
    KRYS_NODISCARD static BorderBoxPositions
      PerformInlineAxisSelfAlignment(const PlacedGridItems &placedGridItems,
                                     const List<UsedMargins> &inlineMargins) noexcept;
    KRYS_NODISCARD static BorderBoxPositions
      PerformBlockAxisSelfAlignment(const PlacedGridItems &placedGridItems,
                                    const List<UsedMargins> &blockMargins) noexcept;

    KRYS_NODISCARD const GridFormattingContext &FormattingContext() const noexcept
    {
      return _gridFormattingContext;
    }
    KRYS_NODISCARD const ElementBox &GridContainer() const noexcept;
    KRYS_NODISCARD const RenderStyle &GridContainerStyle() const noexcept;
  };
}
