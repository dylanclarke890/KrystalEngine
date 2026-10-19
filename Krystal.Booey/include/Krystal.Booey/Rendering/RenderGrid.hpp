#pragma once

#include "Krystal.Booey/Graphics/LayoutRange.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Rendering/Grid.hpp"
#include "Krystal.Booey/Rendering/GridMasonryLayout.hpp"
#include "Krystal.Booey/Rendering/GridTrackSizingAlgorithm.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include <functional>

namespace krys::boo
{
  class GridLayoutState;

  struct ContentAlignmentData
  {
    LayoutUnit PositionOffset;
    LayoutUnit DistributionOffset;
  };

  enum class GridAxisPosition : uint8
  {
    GridAxisStart,
    GridAxisEnd,
    GridAxisCenter
  };

  enum class SubgridDidChange : bool
  {
    No,
    Yes
  };

  /// @brief The logical heights of a grid's items, kept between layouts for the first row sizing pass.
  class GridItemSizeCache
  {
  private:
    // Keyed by address: the cache is reset when the grid's items change (SetNeedsItemPlacement).
    HashMap<const RenderBox *, Maybe<LayoutUnit>> _sizes;

  public:
    void SetSizeForGridItem(const RenderBox &gridItem, LayoutUnit size) noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> SizeForItem(const RenderBox &gridItem) const noexcept;
    void InvalidateSizeForItem(const RenderBox &gridItem) noexcept;
  };

  /// @brief A grid container: places its items in a grid, sizes the grid's tracks and aligns the items in
  /// their areas. Also lays out subgrids and masonry (grid lanes) containers.
  // FIXME: Hit testing (hitTestChildren), once there is hit testing.
  class RenderGrid final : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderGrid);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderGrid);
    friend class GridTrackSizingAlgorithm;
    friend class GridTrackSizingAlgorithmStrategy;
    friend class GridMasonryLayout;
    friend class PositionedLayoutConstraints;

  private:
    enum class ShouldUpdateGridAreaLogicalSize : bool
    {
      No,
      Yes
    };

    using AutoPlacementCursor = Pair<uint32, uint32>;

    /// @brief The grid layout uses, which intrinsic size computation swaps for a temporary one.
    class GridWrapper
    {
      KRYS_NON_COPY_MOVABLE(GridWrapper);

    private:
      Grid _layoutGrid;

    public:
      explicit GridWrapper(RenderGrid &renderGrid) noexcept;
      void ResetCurrentGrid() const noexcept;
      mutable std::reference_wrapper<Grid> CurrentGrid {std::ref(_layoutGrid)};
    };

    GridWrapper _grid;

    // FIXME: Refactor _trackSizingAlgorithm to be inside of LayoutGrid and LayoutMasonry.
    // https://bugs.webkit.org/show_bug.cgi?id=277496
    GridTrackSizingAlgorithm _trackSizingAlgorithm;

    List<LayoutUnit> _columnPositions;
    List<LayoutUnit> _rowPositions;
    ContentAlignmentData _offsetBetweenColumns;
    ContentAlignmentData _offsetBetweenRows;

    mutable GridMasonryLayout _masonryLayout;

    bool _baselineItemsCached {false};

    mutable Maybe<GridItemSizeCache> _intrinsicLogicalHeightsForRowSizingFirstPass;

    Maybe<bool> _hasGridFormattingContextLayout;

    mutable bool _isComputingTrackSizes {false};

  public:
    RenderGrid(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderGrid() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;
    void LayoutBlock(RelayoutChildren relayoutChildren,
                     LayoutUnit pageLogicalHeight = 0_lu) noexcept override;

    KRYS_NODISCARD bool CanDropAnonymousBlockChild() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool IsComputingTrackSizes() const noexcept
    {
      return _isComputingTrackSizes;
    }

    KRYS_NODISCARD bool HasDefiniteLogicalHeight() const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> AvailableLogicalHeightForContentBox() const noexcept;

    void SetNeedsItemPlacement(
      SubgridDidChange descendantSubgridsNeedItemPlacement = SubgridDidChange::No) noexcept;
    KRYS_NODISCARD List<LayoutUnit>
      TrackSizesForComputedStyle(style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD const List<LayoutUnit> &ColumnPositions() const noexcept
    {
      return _columnPositions;
    }
    KRYS_NODISCARD const List<LayoutUnit> &RowPositions() const noexcept
    {
      return _rowPositions;
    }
    KRYS_NODISCARD const List<LayoutUnit> &Positions(style::GridTrackSizingDirection direction) const noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? ColumnPositions() : RowPositions();
    }

    KRYS_NODISCARD uint32
      AutoRepeatCountForDirection(style::GridTrackSizingDirection direction) const noexcept
    {
      return CurrentGrid().AutoRepeatTracks(direction);
    }
    KRYS_NODISCARD uint32
      ExplicitGridStartForDirection(style::GridTrackSizingDirection direction) const noexcept
    {
      return CurrentGrid().ExplicitGridStart(direction);
    }

    // Required by GridTrackSizingAlgorithm. Keep them under control.
    KRYS_NODISCARD LayoutUnit GuttersSize(style::GridTrackSizingDirection direction, uint32 startLine,
                                          uint32 span, Maybe<LayoutUnit> availableSize) const noexcept;
    KRYS_NODISCARD LayoutUnit GridItemOffset(style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit>
      ExplicitIntrinsicInnerLogicalSize(style::GridTrackSizingDirection direction) const noexcept;
    void UpdateGridAreaLogicalSize(RenderBox &gridItem, Maybe<LayoutUnit> width,
                                   Maybe<LayoutUnit> height) const noexcept;
    KRYS_NODISCARD bool IsBaselineAlignmentForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD bool
      IsBaselineAlignmentForGridItem(const RenderBox &gridItem,
                                     style::GridTrackSizingDirection alignmentContext) const noexcept;

    KRYS_NODISCARD StyleSelfAlignmentData
      SelfAlignmentForGridItem(style::GridTrackSizingDirection alignmentContext, const RenderBox &gridItem,
                               const RenderStyle *gridStyle = nullptr) const noexcept;

    KRYS_NODISCARD StyleContentAlignmentData
      ContentAlignment(style::GridTrackSizingDirection direction) const noexcept;

    // These functions handle the actual implementation of layoutBlock based on if
    // the grid is a standard grid or a masonry one. While masonry is an extension of grid,
    // keeping the logic in the same function was leading to a messy amount of if statements being added to
    // handle specific masonry cases.
    void LayoutGrid(RelayoutChildren relayoutChildren) noexcept;
    void LayoutMasonry(RelayoutChildren relayoutChildren) noexcept;

    // Computes the span relative to this RenderGrid, even if the RenderBox is a grid item
    // of a descendant subgrid.
    KRYS_NODISCARD GridSpan GridSpanForGridItem(const RenderBox &gridItem,
                                                style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD bool IsSubgrid() const noexcept;
    KRYS_NODISCARD bool IsSubgrid(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD bool IsSubgridRows() const noexcept
    {
      return IsSubgrid(style::GridTrackSizingDirection::Rows);
    }
    KRYS_NODISCARD bool IsSubgridColumns() const noexcept
    {
      return IsSubgrid(style::GridTrackSizingDirection::Columns);
    }
    KRYS_NODISCARD bool
      IsSubgridInParentDirection(style::GridTrackSizingDirection parentDirection) const noexcept;

    // Returns true if this grid is inheriting subgridded tracks for
    // the given direction from the specified ancestor. This handles
    // nested subgrids, where ancestor may not be our direct parent.
    KRYS_NODISCARD bool IsSubgridOf(style::GridTrackSizingDirection direction,
                                    const RenderGrid &ancestor) const noexcept;

    KRYS_NODISCARD bool IsMasonry() const noexcept;
    KRYS_NODISCARD bool IsMasonry(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD bool AreMasonryRows() const noexcept
    {
      return IsMasonry(style::GridTrackSizingDirection::Rows);
    }
    KRYS_NODISCARD bool AreMasonryColumns() const noexcept
    {
      return IsMasonry(style::GridTrackSizingDirection::Columns);
    }

    KRYS_NODISCARD const Grid &CurrentGrid() const noexcept;
    KRYS_NODISCARD Grid &CurrentGrid() noexcept;

    KRYS_NODISCARD uint32 NumTracks(style::GridTrackSizingDirection direction) const noexcept;

    void PlaceItems() noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit>
      AvailableSpaceForGutters(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD LayoutUnit GridGap(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD LayoutUnit GridGap(style::GridTrackSizingDirection direction,
                                      Maybe<LayoutUnit> availableSize) const noexcept;

    KRYS_NODISCARD LayoutUnit MasonryContentSize() const noexcept;

    void UpdateIntrinsicLogicalHeightsForRowSizingFirstPassCacheAvailability() noexcept;
    KRYS_NODISCARD Maybe<GridItemSizeCache> &IntrinsicLogicalHeightsForRowSizingFirstPass() const noexcept;

    KRYS_NODISCARD bool
      ShouldCheckExplicitIntrinsicInnerLogicalSize(style::GridTrackSizingDirection direction) const noexcept;

    // Checks both the grid container and the grid since the grid container is sized
    // according to the rules of the formatting context it lives in while the size of the
    // grid is determined by the lines/grid areas which come from track sizing.
    KRYS_NODISCARD bool IsExtrinsicallySized() const noexcept;

    KRYS_NODISCARD ItemPosition
      SelfAlignmentNormalBehavior(const RenderBox *gridItem = nullptr) const noexcept override
    {
      krys_debug_assert(gridItem);
      return gridItem->IsRenderReplaced() ? ItemPosition::Start : ItemPosition::Stretch;
    }

  protected:
    void PaintChildren(PaintInfo &forSelf, const LayoutPoint &, PaintInfo &forChild) noexcept override;

  private:
    void ComputeLayoutRequirementsForItemsBeforeLayout(GridLayoutState &gridLayoutState) const noexcept;
    KRYS_NODISCARD bool CanSetColumnAxisStretchRequirementForItem(const RenderBox &gridItem) const noexcept;

    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;

    KRYS_NODISCARD bool SelfAlignmentChangedToStretch(style::GridTrackSizingDirection alignmentContext,
                                                      const RenderStyle &oldStyle,
                                                      const RenderStyle &newStyle,
                                                      const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD bool SelfAlignmentChangedFromStretch(style::GridTrackSizingDirection alignmentContext,
                                                        const RenderStyle &oldStyle,
                                                        const RenderStyle &newStyle,
                                                        const RenderBox &gridItem) const noexcept;

    KRYS_NODISCARD SubgridDidChange GetSubgridDidChange(const RenderStyle &oldStyle) const noexcept;
    KRYS_NODISCARD bool ExplicitGridDidResize(const RenderStyle &oldStyle) const noexcept;
    KRYS_NODISCARD bool NamedGridLinesDefinitionDidChange(const RenderStyle &oldStyle) const noexcept;
    KRYS_NODISCARD bool ImplicitGridLinesDefinitionDidChange(const RenderStyle &oldStyle) const noexcept;

    KRYS_NODISCARD uint32 ComputeAutoRepeatTracksCount(style::GridTrackSizingDirection direction,
                                                       Maybe<LayoutUnit> availableSize) const noexcept;

    KRYS_NODISCARD uint32 ClampAutoRepeatTracks(style::GridTrackSizingDirection direction,
                                                uint32 autoRepeatTracks) const noexcept;

    KRYS_NODISCARD layout::IndexRange
      AutoRepeatTracksRange(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD UniquePtr<OrderedTrackIndexSet>
      ComputeEmptyTracksForAutoRepeat(style::GridTrackSizingDirection direction) const noexcept;

    void PerformPreLayoutForGridItems(
      const GridTrackSizingAlgorithm &algorithm,
      ShouldUpdateGridAreaLogicalSize shouldUpdateGridAreaLogicalSize) const noexcept;

    void PlaceItemsOnGrid(Maybe<LayoutUnit> availableLogicalWidth) noexcept;
    void PopulateExplicitGridAndOrderIterator() noexcept;
    KRYS_NODISCARD GridArea CreateEmptyGridAreaAtSpecifiedPositionsOutsideGrid(
      const RenderBox &gridItem, style::GridTrackSizingDirection specifiedDirection,
      const GridSpan &specifiedPositions) const noexcept;
    KRYS_NODISCARD bool
      IsPlacedWithinExtrinsicallySizedExplicitTracks(const RenderBox &gridItem) const noexcept;
    void PlaceSpecifiedMajorAxisItemsOnGrid(const List<RenderBox *> &autoGridItems) noexcept;
    void PlaceAutoMajorAxisItemsOnGrid(const List<RenderBox *> &autoGridItems) noexcept;
    void PlaceAutoMajorAxisItemOnGrid(RenderBox &gridItem, AutoPlacementCursor &autoPlacementCursor) noexcept;
    KRYS_NODISCARD style::GridTrackSizingDirection AutoPlacementMajorAxisDirection() const noexcept;
    KRYS_NODISCARD style::GridTrackSizingDirection AutoPlacementMinorAxisDirection() const noexcept;

    KRYS_NODISCARD bool CanPerformSimplifiedLayout() const noexcept final;
    void PrepareGridItemForPositionedLayout(RenderBox &gridItem) noexcept;
    KRYS_NODISCARD bool
      HasStaticPositionForGridItem(const RenderBox &gridItem,
                                   style::GridTrackSizingDirection direction) const noexcept;
    void LayoutOutOfFlowBox(RenderBox &gridItem, RelayoutChildren relayoutChildren,
                            bool fixedPositionObjectsOnly) noexcept override;

    void ComputeTrackSizesForDefiniteSize(style::GridTrackSizingDirection direction,
                                          LayoutUnit availableSpace,
                                          GridLayoutState &gridLayoutState) noexcept;
    void ComputeTrackSizesForIndefiniteSize(GridTrackSizingAlgorithm &algorithm,
                                            style::GridTrackSizingDirection direction,
                                            GridLayoutState &gridLayoutState,
                                            LayoutUnit *minIntrinsicSize = nullptr,
                                            LayoutUnit *maxIntrinsicSize = nullptr) const noexcept;

    void RepeatTracksSizingIfNeeded(LayoutUnit availableSpaceForColumns, LayoutUnit availableSpaceForRows,
                                    GridLayoutState &gridLayoutState) noexcept;

    void UpdateGridAreaForAspectRatioItems(const List<RenderBox *> &autoGridItems,
                                           GridLayoutState &gridLayoutState) noexcept;

    void LayoutGridItems(GridLayoutState &gridLayoutState) noexcept;
    void LayoutMasonryItems(GridLayoutState &gridLayoutState) noexcept;

    void PopulateGridPositionsForDirection(const GridTrackSizingAlgorithm &algorithm,
                                           style::GridTrackSizingDirection direction) noexcept;

    KRYS_NODISCARD LayoutRange GridAreaRangeForOutOfFlow(
      const RenderBox &gridItem, style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD Pair<LayoutUnit, LayoutUnit>
      GridAreaPositionForInFlowGridItem(const RenderBox &gridItem,
                                        style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD GridAxisPosition ColumnAxisPositionForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD GridAxisPosition RowAxisPositionForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD LayoutUnit ColumnAxisOffsetForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD LayoutUnit RowAxisOffsetForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD ContentAlignmentData ComputeContentPositionAndDistributionOffset(
      style::GridTrackSizingDirection direction, const LayoutUnit &availableFreeSpace,
      uint32 numberOfGridTracks) const noexcept;
    void SetLogicalPositionForGridItem(RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalOffsetForGridItem(
      const RenderBox &gridItem, style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD LayoutUnit GridAreaBreadthForGridItemIncludingAlignmentOffsets(
      const RenderBox &gridItem, style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD LayoutOptionalOutsets AllowedLayoutOverflow() const noexcept override;
    void ComputeOverflow(LayoutUnit oldClientAfterEdge,
                         EnumSet<ComputeOverflowOptions> options = {}) noexcept override;

    KRYS_NODISCARD StyleSelfAlignmentData
      JustifySelfForGridItem(const RenderBox &gridItem, StretchingMode stretchingMode = StretchingMode::Any,
                             const RenderStyle *gridStyle = nullptr) const noexcept;
    KRYS_NODISCARD StyleSelfAlignmentData
      AlignSelfForGridItem(const RenderBox &gridItem, StretchingMode stretchingMode = StretchingMode::Any,
                           const RenderStyle *gridStyle = nullptr) const noexcept;
    void ApplyStretchAlignmentToGridItemIfNeeded(RenderBox &gridItem,
                                                 GridLayoutState &gridLayoutState) noexcept;
    void ApplySubgridStretchAlignmentToGridItemIfNeeded(RenderBox &gridItem) noexcept;
    KRYS_NODISCARD bool IsChildEligibleForMarginTrim(style::MarginTrimSide marginTrimSide,
                                                     const RenderBox &gridItem) const noexcept final;

    KRYS_NODISCARD Maybe<LayoutUnit> FirstLineBaseline() const noexcept final;
    KRYS_NODISCARD Maybe<LayoutUnit> LastLineBaseline() const noexcept final;
    KRYS_NODISCARD const RenderBox *BaselineGridItem(ItemPosition alignment) const noexcept;

    KRYS_NODISCARD LayoutUnit ColumnAxisBaselineOffsetForGridItem(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD LayoutUnit RowAxisBaselineOffsetForGridItem(const RenderBox &gridItem) const noexcept;

    KRYS_NODISCARD uint32 NonCollapsedTracks(style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD LayoutUnit TranslateRTLCoordinate(LayoutUnit coordinate) const noexcept;

    KRYS_NODISCARD bool ShouldResetLogicalHeightBeforeLayout() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool AspectRatioPrefersInline(const RenderBox &gridItem,
                                                 bool blockFlowIsColumnAxis) noexcept;

    KRYS_NODISCARD List<RenderBox *>
      ComputeAspectRatioDependentAndBaselineItems(GridLayoutState &gridLayoutState) noexcept;

    KRYS_NODISCARD GridSpan GridSpanForOutOfFlowGridItem(
      const RenderBox &gridItem, style::GridTrackSizingDirection direction) const noexcept;

    bool ComputeGridPositionsForOutOfFlowGridItem(const RenderBox &gridItem,
                                                  style::GridTrackSizingDirection direction, int &startLine,
                                                  bool &startIsAuto, int &endLine,
                                                  bool &endIsAuto) const noexcept;

    KRYS_NODISCARD AutoRepeatType GetAutoRepeatType(style::GridTrackSizingDirection direction) const noexcept;

    KRYS_NODISCARD List<LayoutUnit> &MutablePositions(style::GridTrackSizingDirection direction) noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _columnPositions : _rowPositions;
    }

    KRYS_NODISCARD ContentAlignmentData &
      OffsetBetweenTracks(style::GridTrackSizingDirection direction) noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _offsetBetweenColumns
                                                                   : _offsetBetweenRows;
    }
    KRYS_NODISCARD const ContentAlignmentData &
      OffsetBetweenTracks(style::GridTrackSizingDirection direction) const noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _offsetBetweenColumns
                                                                   : _offsetBetweenRows;
    }

    KRYS_NODISCARD bool CanCreateIntrinsicLogicalHeightsForRowSizingFirstPassCache() const noexcept;

    bool LayoutUsingGridFormattingContext() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderGrid)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderGrid();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
