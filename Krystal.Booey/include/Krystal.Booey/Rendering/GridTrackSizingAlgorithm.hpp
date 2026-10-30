#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Rendering/Grid.hpp"
#include "Krystal.Booey/Rendering/GridBaselineAlignment.hpp"
#include "Krystal.Booey/Rendering/GridLayoutState.hpp"
#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <map>

namespace krys::boo
{
  class RenderBox;
  class RenderGrid;

  inline constexpr int infinity = -1;

  enum class SizingOperation : uint8
  {
    TrackSizing,
    IntrinsicSizeComputation
  };

  enum class TrackSizeComputationVariant : uint8
  {
    NotCrossingFlexibleTracks,
    CrossingFlexibleTracks,
  };

  enum class TrackSizeComputationPhase : uint8
  {
    ResolveIntrinsicMinimums,
    ResolveContentBasedMinimums,
    ResolveMaxContentMinimums,
    ResolveIntrinsicMaximums,
    ResolveMaxContentMaximums,
    MaximizeTracks,
  };

  enum class SpaceDistributionLimit : uint8
  {
    UpToGrowthLimit,
    BeyondGrowthLimit,
  };

  class GridTrackSizingAlgorithmStrategy;
  class GridItemWithSpan;

  /// @brief A grid track being sized: its base size, growth limit and the sizes the algorithm plans for it.
  class GridTrack final
  {
  private:
    LayoutUnit _baseSize {0};
    LayoutUnit _growthLimit {0};
    LayoutUnit _plannedSize {0};
    LayoutUnit _tempSize {0};
    Maybe<LayoutUnit> _growthLimitCap;
    bool _infinitelyGrowable {false};
    Maybe<style::GridTrackSize> _cachedTrackSize;

  public:
    GridTrack() noexcept = default;

    KRYS_NODISCARD LayoutUnit BaseSize() const noexcept;
    KRYS_NODISCARD LayoutUnit UnclampedBaseSize() const noexcept;
    void SetBaseSize(LayoutUnit baseSize) noexcept;

    KRYS_NODISCARD const LayoutUnit &GrowthLimit() const noexcept;
    KRYS_NODISCARD bool GrowthLimitIsInfinite() const noexcept
    {
      return _growthLimit == infinity;
    }
    void SetGrowthLimit(LayoutUnit growthLimit) noexcept;

    KRYS_NODISCARD bool InfiniteGrowthPotential() const noexcept
    {
      return GrowthLimitIsInfinite() || _infinitelyGrowable;
    }
    KRYS_NODISCARD LayoutUnit GrowthLimitIfNotInfinite() const noexcept;

    KRYS_NODISCARD const LayoutUnit &PlannedSize() const noexcept
    {
      return _plannedSize;
    }
    void SetPlannedSize(LayoutUnit plannedSize) noexcept
    {
      _plannedSize = plannedSize;
    }

    KRYS_NODISCARD const LayoutUnit &TempSize() const noexcept
    {
      return _tempSize;
    }
    void SetTempSize(const LayoutUnit &tempSize) noexcept;
    void GrowTempSize(const LayoutUnit &tempSize) noexcept;

    KRYS_NODISCARD bool InfinitelyGrowable() const noexcept
    {
      return _infinitelyGrowable;
    }
    void SetInfinitelyGrowable(bool infinitelyGrowable) noexcept
    {
      _infinitelyGrowable = infinitelyGrowable;
    }

    void SetGrowthLimitCap(Maybe<LayoutUnit> growthLimitCap) noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit> GrowthLimitCap() const noexcept
    {
      return _growthLimitCap;
    }

    KRYS_NODISCARD const style::GridTrackSize &CachedTrackSize() const noexcept;
    void SetCachedTrackSize(const style::GridTrackSize &cachedTrackSize) noexcept;

  private:
    KRYS_NODISCARD bool IsGrowthLimitBiggerThanBaseSize() const noexcept
    {
      return GrowthLimitIsInfinite() || _growthLimit >= std::max(_baseSize, 0_lu);
    }

    void EnsureGrowthLimitIsBiggerThanBaseSize() noexcept;
  };

  /// @brief The grid track sizing algorithm (css-grid 12), for both layout and intrinsic size computation.
  class GridTrackSizingAlgorithm final
  {
    KRYS_NON_COPY_MOVABLE(GridTrackSizingAlgorithm);
    friend class GridTrackSizingAlgorithmStrategy;
    friend class DefiniteSizeStrategy;
    friend class IndefiniteSizeStrategy;

  public:
    using Tracks = List<UniquePtr<GridTrack>>;

  private:
    using SpanLength = uint32;

    struct MasonryMinMaxTrackSize
    {
      LayoutUnit MinContentSize;
      LayoutUnit MaxContentSize;
      LayoutUnit MinSize;
    };

    struct MasonryMinMaxTrackSizeWithGridSpan
    {
      MasonryMinMaxTrackSize TrackSize;
      GridSpan Span;
    };

    enum class SizingState : uint8
    {
      ColumnSizingFirstIteration,
      RowSizingFirstIteration,
      RowSizingExtraIterationForSizeContainment,
      ColumnSizingSecondIteration,
      RowSizingSecondIteration
    };

    // Keyed by address: the caches are cleared at the end of each layout of the grid.
    using BaselineItemsCache = HashMap<const RenderBox *, bool>;
    using TrackIndexSet = HashSet<uint32>;

    // This is a RAII class used to ensure that the track sizing algorithm is
    // executed as it is supposed to be, i.e., first resolve columns and then
    // rows. Only if required a second iteration is run following the same order,
    // first columns and then rows.
    class StateMachine
    {
      KRYS_NON_COPY_MOVABLE(StateMachine);

    private:
      GridTrackSizingAlgorithm &_algorithm;

    public:
      explicit StateMachine(GridTrackSizingAlgorithm &algorithm) noexcept;
      ~StateMachine() noexcept;
    };

    // Data.
    bool _needsSetup {true};
    bool _hasPercentSizedRowsIndefiniteHeight {false};
    bool _hasFlexibleMaxTrackBreadth {false};
    Maybe<LayoutUnit> _availableSpaceRows;
    Maybe<LayoutUnit> _availableSpaceColumns;

    Maybe<LayoutUnit> _freeSpaceColumns;
    Maybe<LayoutUnit> _freeSpaceRows;

    // We need to keep both alive in order to properly size grids with orthogonal
    // writing modes.
    Tracks _columns;
    Tracks _rows;
    List<uint32> _contentSizedTracksIndex;
    List<uint32> _flexibleSizedTracksIndex;
    List<uint32> _autoSizedTracksForStretchIndex;

    style::GridTrackSizingDirection _direction {style::GridTrackSizingDirection::Columns};
    SizingOperation _sizingOperation {SizingOperation::TrackSizing};

    Grid &_grid;

    const RenderGrid *_renderGrid;
    UniquePtr<GridTrackSizingAlgorithmStrategy> _strategy;

    // The track sizing algorithm is used for both layout and intrinsic size
    // computation. We're normally just interested in intrinsic inline sizes
    // (a.k.a widths in most of the cases) for the computeIntrinsicLogicalWidths()
    // computations. That's why we don't need to keep around different values for
    // rows/columns.
    LayoutUnit _minContentSize;
    LayoutUnit _maxContentSize;

    SizingState _sizingState;

    GridBaselineAlignment _baselineAlignment;

    // Rows/columns here indicate the alignment context.
    BaselineItemsCache _baselineAlignmentItemsForRows;
    BaselineItemsCache _baselineAlignmentItemsForColumns;

    // Keyed by address: cleared with the baseline items cache.
    ListHashSet<const RenderGrid *> _rowSubgridsWithBaselineAlignedItems;

  public:
    GridTrackSizingAlgorithm(const RenderGrid *renderGrid, Grid &grid) noexcept;
    ~GridTrackSizingAlgorithm() noexcept;

    void Run(style::GridTrackSizingDirection direction, uint32 numTracks, SizingOperation sizingOperation,
             Maybe<LayoutUnit> availableSpace, GridLayoutState &gridLayoutState) noexcept;
    void Reset() noexcept;

    // Required by RenderGrid. Try to minimize the exposed surface.
    KRYS_NODISCARD const Grid &GetGrid() const noexcept
    {
      return _grid;
    }

    KRYS_NODISCARD const RenderGrid *GetRenderGrid() const noexcept
    {
      return _renderGrid;
    }

    KRYS_NODISCARD LayoutUnit MinContentSize() const noexcept
    {
      return _minContentSize;
    }
    KRYS_NODISCARD LayoutUnit MaxContentSize() const noexcept
    {
      return _maxContentSize;
    }

    KRYS_NODISCARD LayoutUnit BaselineOffsetForGridItem(
      const RenderBox &gridItem, style::GridTrackSizingDirection alignmentContextType) const noexcept;

    // The estimated grid area should be use pre-layout versus the grid area, which should be used once
    // layout is complete.
    KRYS_NODISCARD Maybe<LayoutUnit>
      GridAreaBreadthForGridItem(const RenderBox &gridItem,
                                 style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      EstimatedGridAreaBreadthForGridItem(const RenderBox &gridItem,
                                          style::GridTrackSizingDirection direction) const noexcept;

    void CacheBaselineAlignedItem(const RenderBox &item, style::GridTrackSizingDirection alignmentContextType,
                                  bool cachingRowSubgridsForRootGrid) noexcept;
    void CopyBaselineItemsCache(const GridTrackSizingAlgorithm &source,
                                style::GridTrackSizingDirection alignmentContextType) noexcept;
    void ClearBaselineItemsCache() noexcept;

    KRYS_NODISCARD Tracks &GetTracks(style::GridTrackSizingDirection direction) noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _columns : _rows;
    }
    KRYS_NODISCARD const Tracks &GetTracks(style::GridTrackSizingDirection direction) const noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _columns : _rows;
    }

    KRYS_NODISCARD Maybe<LayoutUnit> FreeSpace(style::GridTrackSizingDirection direction) const noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _freeSpaceColumns : _freeSpaceRows;
    }
    void SetFreeSpace(style::GridTrackSizingDirection direction, Maybe<LayoutUnit> freeSpace) noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> AvailableSpace(style::GridTrackSizingDirection direction) const noexcept
    {
      return direction == style::GridTrackSizingDirection::Columns ? _availableSpaceColumns
                                                                   : _availableSpaceRows;
    }
    void SetAvailableSpace(style::GridTrackSizingDirection direction,
                           Maybe<LayoutUnit> availableSpace) noexcept;

    KRYS_NODISCARD LayoutUnit ComputeTrackBasedSize() const noexcept;

    KRYS_NODISCARD bool HasAllLengthRowSizes() const noexcept;
    KRYS_NODISCARD bool HasAnyPercentSizedRowsIndefiniteHeight() const noexcept
    {
      return _hasPercentSizedRowsIndefiniteHeight;
    }
    KRYS_NODISCARD bool HasAnyFlexibleMaxTrackBreadth() const noexcept
    {
      return _hasFlexibleMaxTrackBreadth;
    }
    KRYS_NODISCARD bool HasAnyBaselineAlignmentItem() const noexcept
    {
      return !_baselineAlignmentItemsForRows.empty() || !_baselineAlignmentItemsForColumns.empty();
    }

#if KRYS_DEBUG
    KRYS_NODISCARD bool TracksAreWiderThanMinTrackBreadth() const noexcept;
#endif

  private:
    void Setup(style::GridTrackSizingDirection direction, uint32 numTracks, SizingOperation sizingOperation,
               Maybe<LayoutUnit> availableSpace) noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> AvailableSpace() const noexcept;
    KRYS_NODISCARD style::GridTrackSize CalculateGridTrackSize(style::GridTrackSizingDirection direction,
                                                               uint32 translatedIndex) const noexcept;
    KRYS_NODISCARD const style::GridTrackSize &RawGridTrackSize(style::GridTrackSizingDirection direction,
                                                                uint32 translatedIndex) const noexcept;

    // Helper methods for step 1. initializeTrackSizes().
    KRYS_NODISCARD LayoutUnit InitialBaseSize(const style::GridTrackSize &trackSize) const noexcept;
    KRYS_NODISCARD LayoutUnit InitialGrowthLimit(const style::GridTrackSize &trackSize,
                                                 LayoutUnit baseSize) const noexcept;

    // Helper methods for step 2. resolveIntrinsicTrackSizes().
    void SizeTrackToFitNonSpanningItem(const GridSpan &span, RenderBox &gridItem, GridTrack &track,
                                       GridLayoutState &gridLayoutState) noexcept;
    void SizeTrackToFitSingleSpanMasonryGroup(const GridSpan &span,
                                              MasonryMinMaxTrackSize &masonryIndefiniteItems,
                                              GridTrack &track) noexcept;

    KRYS_NODISCARD bool SpanningItemCrossesFlexibleSizedTracks(const GridSpan &itemSpan) const noexcept;

    using GridItemsSpanGroupRange = Span<GridItemWithSpan>;
    template <TrackSizeComputationVariant variant, TrackSizeComputationPhase phase>
    void IncreaseSizesToAccommodateSpanningItems(GridItemsSpanGroupRange gridItemsWithSpan,
                                                 GridLayoutState &gridLayoutState) noexcept;
    template <TrackSizeComputationVariant variant>
    void IncreaseSizesToAccommodateSpanningItems(GridItemsSpanGroupRange gridItemsWithSpan,
                                                 GridLayoutState &gridLayoutState) noexcept;

    // 12.5 Resolve Intrinsic Track Sizing : Step 3
    // https://drafts.csswg.org/css-grid-2/#algo-spanning-items
    //
    // Take all grid items (definite and indefinite) that span 2 or more tracks, and distribute space to
    // intrinsic tracks (non-flex). The implementation diverges from
    // increaseSizesToAccommodateSpanningItems(), because we are grouping items together that are the same
    // span length. This function is divided into two main sections:
    //
    // 1. Constructing the track items
    // This step takes the definite and indefinite items, and merges them into one large map to send over to
    // the second step. Since the indefinite items are grouped together from a prior computation, this step
    // also need to create "fake" grid items that will be considered in each track.
    //
    // 2. Distribute space to intrinsic tracks
    // This step behaves similar to increaseSizesToAccommodateSpanningItems() where we start at the lowest
    // span length and distribute space to the tracks. Then look at the next smallest span length, and repeat
    // step 2 until we exhaust all grid items.
    template <TrackSizeComputationVariant variant>
    void IncreaseSizesToAccommodateSpanningItemsMasonry(
      std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> &definiteItemSizes) noexcept;

    // 12.5 Resolve Intrinsic Track Sizing : Step 4
    // https://drafts.csswg.org/css-grid-2/#algo-spanning-items
    //
    // Take all grid items (definite and indefinite) that span 1 or tracks, and distribute space to only flex
    // tracks. The implementation diverges from increaseSizesToAccommodateSpanningItems(), because we are
    // grouping items together that are the same span length. This function is divided into two main sections:
    //
    // 1. Constructing the track items
    // This step takes the definite and indefinite items, and merges them into one large map to send over to
    // the second step. Since the indefinite items are grouped together from a prior computation, this step
    // also need to create "fake" grid items that will be considered in each track.
    //
    // 2. Distribute space to intrinsic tracks
    // This step behaves similar to increaseSizesToAccommodateSpanningItems() where we consider all track
    // items at once instead of per span length.
    template <TrackSizeComputationVariant variant>
    void IncreaseSizesToAccommodateSpanningItemsMasonryWithFlex(
      List<MasonryMinMaxTrackSizeWithGridSpan> &definiteItemSizesSpanFlexTracks) noexcept;

    void ConvertIndefiniteItemsToDefiniteMasonry(
      const std::map<SpanLength, MasonryMinMaxTrackSize> &indefiniteSpanSizes,
      std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> &definiteItemSizes,
      List<MasonryMinMaxTrackSizeWithGridSpan> &definiteItemSizesSpanFlexTracks) noexcept;

    KRYS_NODISCARD LayoutUnit ItemSizeForTrackSizeComputationPhase(
      TrackSizeComputationPhase phase, RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept;
    KRYS_NODISCARD LayoutUnit ItemSizeForTrackSizeComputationPhaseMasonry(
      TrackSizeComputationPhase phase, const MasonryMinMaxTrackSize &trackSize) const noexcept;

    template <TrackSizeComputationVariant variant, TrackSizeComputationPhase phase>
    void DistributeSpaceToTracks(List<GridTrack *> &tracks, List<GridTrack *> *growBeyondGrowthLimitsTracks,
                                 LayoutUnit &freeSpace) const noexcept;

    void ComputeBaselineAlignmentContext() noexcept;
    void UpdateBaselineAlignmentContext(const RenderBox &gridItem,
                                        style::GridTrackSizingDirection alignmentContextType) noexcept;
    KRYS_NODISCARD bool
      CanParticipateInBaselineAlignment(const RenderBox &gridItem,
                                        style::GridTrackSizingDirection alignmentContextType) const noexcept;
    KRYS_NODISCARD bool
      ParticipateInBaselineAlignment(const RenderBox &gridItem,
                                     style::GridTrackSizingDirection alignmentContextType) const noexcept;

    KRYS_NODISCARD bool
      IsIntrinsicSizedGridArea(const RenderBox &gridItem,
                               style::GridTrackSizingDirection gridAreaDirection) const noexcept;
    void ComputeGridContainerIntrinsicSizes() noexcept;

    // Helper methods for step 4. Stretch flexible tracks.
    KRYS_NODISCARD double
      ComputeFlexFactorUnitSize(const Tracks &tracks, double flexFactorSum, LayoutUnit &leftOverSpace,
                                const List<uint32> &flexibleTracksIndexes,
                                UniquePtr<TrackIndexSet> tracksToTreatAsInflexible = nullptr) const noexcept;
    void ComputeFlexSizedTracksGrowth(double flexFraction, List<LayoutUnit> &increments,
                                      LayoutUnit &totalGrowth) const noexcept;
    KRYS_NODISCARD double FindFrUnitSize(const GridSpan &tracksSpan, LayoutUnit leftOverSpace) const noexcept;

    void HandleInfinityGrowthLimit() noexcept;

    // Build up a map of min/max sizes for each span length for use during resolving intrinsic track sizes.
    // We also need to keep track of definite items separately, since they do not contribute to every track
    // like indefinite items do.
    void ComputeDefiniteAndIndefiniteItemsForMasonry(
      std::map<SpanLength, MasonryMinMaxTrackSize> &indefiniteSpanSizes,
      std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> &definiteItemSizes,
      List<MasonryMinMaxTrackSizeWithGridSpan> &definiteItemSizesSpanFlexTrack,
      GridLayoutState &gridLayoutState) noexcept;
    KRYS_NODISCARD bool ShouldExcludeGridItemForMasonryTrackSizing(const RenderBox &gridItem,
                                                                   uint32 trackIndex,
                                                                   GridSpan itemSpan) const noexcept;
    // Track sizing algorithm steps. Note that the "Maximize Tracks" step is done
    // entirely inside the strategies, that's why we don't need an additional
    // method at this level.
    void InitializeTrackSizes() noexcept;
    void ResolveIntrinsicTrackSizes(GridLayoutState &gridLayoutState) noexcept;

    // Masonry Implementation of https://drafts.csswg.org/css-grid-2/#algo-content.
    // To implement Masonry performanently, we need to abandon the traditional Grid approach of treating
    // each item individually and start grouping items based on their span. A grid item has 3 major values we
    // care about the minContentSize, maxContentSize, and minSize. These values can be aggregated together and
    // then the max will be chosen. The main three scenarios we need to focus on are items that only span 1
    // track, items that span multiple tracks without crossing a flex track, and items that span multiple
    // tracks with crossing a flex track.
    //
    // Further details on the optimization can be found at
    // https://fantasai.inkedblade.net/style/specs/masonry/performance.
    void ResolveIntrinsicTrackSizesMasonry(GridLayoutState &gridLayoutState) noexcept;
    void StretchFlexibleTracks(Maybe<LayoutUnit> freeSpace, GridLayoutState &gridLayoutState) noexcept;
    void StretchAutoTracks() noexcept;

    void AccumulateIntrinsicSizesForTrack(GridTrack &track, uint32 trackIndex, GridIterator &iterator,
                                          List<GridItemWithSpan> &itemsSortedByIncreasingSpan,
                                          List<GridItemWithSpan> &itemsCrossingFlexibleTracks,
                                          HashSet<const RenderBox *> &itemsSet,
                                          LayoutUnit currentAccumulatedMbp,
                                          GridLayoutState &gridLayoutState) noexcept;

    bool CopyUsedTrackSizesForSubgrid() noexcept;

    // State machine.
    void AdvanceNextState() noexcept;
    KRYS_NODISCARD bool IsValidTransition() const noexcept;

    KRYS_NODISCARD bool IsDirectionInMasonryDirection() const noexcept;

    KRYS_NODISCARD bool WasSetup() const noexcept
    {
      return !!_strategy;
    }
  };

  /// @brief How the track sizing algorithm sizes items and maximizes tracks, for a definite or an indefinite
  /// size.
  class GridTrackSizingAlgorithmStrategy
  {
    KRYS_NON_COPY_MOVABLE(GridTrackSizingAlgorithmStrategy);

  protected:
    GridTrackSizingAlgorithm &_algorithm;

  public:
    virtual ~GridTrackSizingAlgorithmStrategy() noexcept = default;

    KRYS_NODISCARD virtual LayoutUnit
      MinContentContributionForGridItem(RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept;
    KRYS_NODISCARD LayoutUnit
      MaxContentContributionForGridItem(RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept;
    KRYS_NODISCARD LayoutUnit MinContributionForGridItem(RenderBox &gridItem,
                                                         GridLayoutState &gridLayoutState) const noexcept;

    virtual void MaximizeTracks(GridTrackSizingAlgorithm::Tracks &tracks,
                                Maybe<LayoutUnit> &freeSpace) noexcept = 0;
    KRYS_NODISCARD virtual double FindUsedFlexFraction(List<uint32> &flexibleSizedTracksIndex,
                                                       style::GridTrackSizingDirection direction,
                                                       Maybe<LayoutUnit> initialFreeSpace,
                                                       GridLayoutState &gridLayoutState) const noexcept = 0;
    virtual bool RecomputeUsedFlexFractionIfNeeded(double &flexFraction,
                                                   LayoutUnit &totalGrowth) const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit FreeSpaceForStretchAutoTracksStep() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsComputingSizeContainment() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsComputingInlineSizeContainment() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsComputingSizeOrInlineSizeContainment() const noexcept = 0;

  protected:
    explicit GridTrackSizingAlgorithmStrategy(GridTrackSizingAlgorithm &algorithm) noexcept
        : _algorithm(algorithm)
    {
    }

    KRYS_NODISCARD virtual LayoutUnit
      MinLogicalSizeForGridItem(RenderBox &gridItem, const style::MinimumSize &gridItemMinSize,
                                Maybe<LayoutUnit> availableSize) const noexcept;
    virtual void LayoutGridItemForMinSizeComputation(RenderBox &gridItem,
                                                     bool overrideSizeHasChanged) const noexcept = 0;

    KRYS_NODISCARD LayoutUnit LogicalHeightForGridItem(RenderBox &gridItem,
                                                       GridLayoutState &gridLayoutState) const noexcept;
    bool UpdateOverridingContainingBlockContentSizeForGridItem(
      RenderBox &gridItem, style::GridTrackSizingDirection direction,
      Maybe<LayoutUnit> overrideSize = null) const noexcept;

    // GridTrackSizingAlgorithm accessors for subclasses.
    KRYS_NODISCARD LayoutUnit ComputeTrackBasedSize() const noexcept
    {
      return _algorithm.ComputeTrackBasedSize();
    }
    KRYS_NODISCARD style::GridTrackSizingDirection Direction() const noexcept
    {
      return _algorithm._direction;
    }
    KRYS_NODISCARD GridTrackSizingAlgorithm::SizingState GetSizingState() const noexcept
    {
      return _algorithm._sizingState;
    }
    KRYS_NODISCARD double FindFrUnitSize(const GridSpan &tracksSpan, LayoutUnit leftOverSpace) const noexcept
    {
      return _algorithm.FindFrUnitSize(tracksSpan, leftOverSpace);
    }
    void DistributeSpaceToTracks(List<GridTrack *> &tracks, LayoutUnit &availableLogicalSpace) const noexcept
    {
      _algorithm.DistributeSpaceToTracks<TrackSizeComputationVariant::NotCrossingFlexibleTracks,
                                         TrackSizeComputationPhase::MaximizeTracks>(tracks, nullptr,
                                                                                    availableLogicalSpace);
    }
    KRYS_NODISCARD const RenderGrid *GetRenderGrid() const noexcept
    {
      return _algorithm._renderGrid;
    }
    KRYS_NODISCARD Maybe<LayoutUnit> AvailableSpace() const noexcept
    {
      return _algorithm.AvailableSpace();
    }
  };
}
