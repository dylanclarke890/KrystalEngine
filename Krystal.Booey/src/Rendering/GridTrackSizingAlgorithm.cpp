#include "Krystal.Booey/Rendering/GridTrackSizingAlgorithm.hpp"
#include "Krystal.Booey/Rendering/AncestorSubgridIterator.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPositionsResolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  using style::GridTrackSizingDirection;

  GridTrackSizingAlgorithm::GridTrackSizingAlgorithm(const RenderGrid *renderGrid, Grid &grid) noexcept
      : _grid(grid), _renderGrid(renderGrid), _sizingState(SizingState::ColumnSizingFirstIteration)
  {
  }

  GridTrackSizingAlgorithm::~GridTrackSizingAlgorithm() noexcept = default;

  LayoutUnit GridTrack::BaseSize() const noexcept
  {
    krys_debug_assert(IsGrowthLimitBiggerThanBaseSize());
    return std::max(_baseSize, 0_lu);
  }

  LayoutUnit GridTrack::UnclampedBaseSize() const noexcept
  {
    krys_debug_assert(IsGrowthLimitBiggerThanBaseSize());
    return _baseSize;
  }

  const LayoutUnit &GridTrack::GrowthLimit() const noexcept
  {
    krys_debug_assert(IsGrowthLimitBiggerThanBaseSize());
    krys_debug_assert(!_growthLimitCap || _growthLimitCap.value() >= _growthLimit
                      || BaseSize() >= _growthLimitCap.value());
    return _growthLimit;
  }

  void GridTrack::SetBaseSize(LayoutUnit baseSize) noexcept
  {
    _baseSize = baseSize;
    EnsureGrowthLimitIsBiggerThanBaseSize();
  }

  void GridTrack::SetGrowthLimit(LayoutUnit growthLimit) noexcept
  {
    _growthLimit =
      growthLimit == infinity ? growthLimit : std::min(growthLimit, _growthLimitCap.value_or(growthLimit));
    EnsureGrowthLimitIsBiggerThanBaseSize();
  }

  LayoutUnit GridTrack::GrowthLimitIfNotInfinite() const noexcept
  {
    krys_debug_assert(IsGrowthLimitBiggerThanBaseSize());
    return _growthLimit == infinity ? BaseSize() : _growthLimit;
  }

  void GridTrack::SetTempSize(const LayoutUnit &tempSize) noexcept
  {
    krys_debug_assert(tempSize >= 0);
    krys_debug_assert(GrowthLimitIsInfinite() || GrowthLimit() >= tempSize);
    _tempSize = tempSize;
  }

  void GridTrack::GrowTempSize(const LayoutUnit &tempSize) noexcept
  {
    krys_debug_assert(tempSize >= 0);
    _tempSize += tempSize;
  }

  void GridTrack::SetGrowthLimitCap(Maybe<LayoutUnit> growthLimitCap) noexcept
  {
    krys_debug_assert(!growthLimitCap || growthLimitCap.value() >= 0);
    _growthLimitCap = growthLimitCap;
  }

  const style::GridTrackSize &GridTrack::CachedTrackSize() const noexcept
  {
    krys_assert(_cachedTrackSize);
    return *_cachedTrackSize;
  }

  void GridTrack::SetCachedTrackSize(const style::GridTrackSize &cachedTrackSize) noexcept
  {
    _cachedTrackSize = cachedTrackSize;
  }

  void GridTrack::EnsureGrowthLimitIsBiggerThanBaseSize() noexcept
  {
    if (_growthLimit != infinity && _growthLimit < std::max(_baseSize, 0_lu))
    {
      _growthLimit = std::max(_baseSize, 0_lu);
    }
  }

  namespace
  {
    KRYS_NODISCARD bool HasRelativeMarginOrPaddingForGridItem(const RenderBox &gridItem,
                                                              GridTrackSizingDirection direction) noexcept
    {
      if (direction == GridTrackSizingDirection::Columns)
      {
        return gridItem.Style().MarginStart().IsPercentOrCalculated()
               || gridItem.Style().MarginEnd().IsPercentOrCalculated()
               || gridItem.Style().PaddingStart().IsPercentOrCalculated()
               || gridItem.Style().PaddingEnd().IsPercentOrCalculated();
      }
      return gridItem.Style().MarginBefore().IsPercentOrCalculated()
             || gridItem.Style().MarginAfter().IsPercentOrCalculated()
             || gridItem.Style().PaddingBefore().IsPercentOrCalculated()
             || gridItem.Style().PaddingAfter().IsPercentOrCalculated();
    }

    KRYS_NODISCARD bool
      ShouldClearOverridingContainingBlockContentSizeForGridItem(const RenderBox &gridItem,
                                                                 GridTrackSizingDirection direction) noexcept
    {
      return GridLayoutFunctions::HasRelativeOrIntrinsicSizeForGridItem(gridItem, direction)
             || HasRelativeMarginOrPaddingForGridItem(gridItem, direction);
    }

    void SetOverridingContainingBlockContentSizeForGridItem(const RenderGrid &grid, RenderBox &gridItem,
                                                            GridTrackSizingDirection direction,
                                                            Maybe<LayoutUnit> size) noexcept
    {
      // This function sets the dimension based on the writing mode of the containing block.
      // For subgrids, this might not be the outermost grid, but could be a subgrid. If the
      // writing mode of the CB and the grid for which we're doing sizing don't match, swap
      // the directions.
      direction =
        GridLayoutFunctions::FlowAwareDirectionForGridItem(grid, *gridItem.ContainingBlock(), direction);
      if (direction == GridTrackSizingDirection::Columns)
      {
        gridItem.SetGridAreaContentLogicalWidth(size);
      }
      else
      {
        gridItem.SetGridAreaContentLogicalHeight(size);
      }
    }

    KRYS_NODISCARD LayoutUnit EvaluateTrackLength(const auto &length,
                                                  Maybe<LayoutUnit> availableSpace) noexcept
    {
      return style::EvaluateMinimum<LayoutUnit>(length, availableSpace.value_or(0_lu), style::ZoomNeeded {});
    }
  }

  // GridTrackSizingAlgorithm private.

  void GridTrackSizingAlgorithm::SetFreeSpace(GridTrackSizingDirection direction,
                                              Maybe<LayoutUnit> freeSpace) noexcept
  {
    if (direction == GridTrackSizingDirection::Columns)
    {
      _freeSpaceColumns = freeSpace;
    }
    else
    {
      _freeSpaceRows = freeSpace;
    }
  }

  Maybe<LayoutUnit> GridTrackSizingAlgorithm::AvailableSpace() const noexcept
  {
    krys_debug_assert(WasSetup());
    return AvailableSpace(_direction);
  }

  void GridTrackSizingAlgorithm::SetAvailableSpace(GridTrackSizingDirection direction,
                                                   Maybe<LayoutUnit> availableSpace) noexcept
  {
    if (direction == GridTrackSizingDirection::Columns)
    {
      _availableSpaceColumns = availableSpace;
    }
    else
    {
      _availableSpaceRows = availableSpace;
    }
  }

  const style::GridTrackSize &
    GridTrackSizingAlgorithm::RawGridTrackSize(GridTrackSizingDirection direction,
                                               uint32 translatedIndex) const noexcept
  {
    auto &renderStyle = _renderGrid->Style();
    auto &autoTrackStyles = renderStyle.GridAutoList(direction);
    auto &tracks = renderStyle.GridTemplateList(direction);
    auto &trackStyles = tracks.Sizes;
    auto &autoRepeatTrackStyles = tracks.AutoRepeatSizes;
    uint32 insertionPoint = tracks.AutoRepeatInsertionPoint;
    uint32 autoRepeatTracksCount = _grid.AutoRepeatTracks(direction);

    // We should not use Style::GridPositionsResolver::explicitGridXXXCount() for this because the
    // explicit grid might be larger than the number of tracks in grid-template-rows|columns (if
    // grid-template-areas is specified for example).
    uint32 explicitTracksCount = static_cast<uint32>(trackStyles.size()) + autoRepeatTracksCount;

    int untranslatedIndexAsInt =
      static_cast<int>(translatedIndex) - static_cast<int>(_grid.ExplicitGridStart(direction));
    auto autoTrackStylesSize = static_cast<uint32>(autoTrackStyles.size());
    if (untranslatedIndexAsInt < 0)
    {
      int index = untranslatedIndexAsInt % static_cast<int>(autoTrackStylesSize);
      // We need to transpose the index because the first negative implicit line will get the last defined
      // auto track and so on.
      index += index ? static_cast<int>(autoTrackStylesSize) : 0;
      krys_debug_assert(index >= 0);
      return autoTrackStyles[static_cast<size_t>(index)];
    }

    auto untranslatedIndex = static_cast<uint32>(untranslatedIndexAsInt);
    if (untranslatedIndex >= explicitTracksCount)
    {
      return autoTrackStyles[(untranslatedIndex - explicitTracksCount) % autoTrackStylesSize];
    }

    if (!autoRepeatTracksCount || untranslatedIndex < insertionPoint)
    {
      return trackStyles[untranslatedIndex];
    }

    if (untranslatedIndex < (insertionPoint + autoRepeatTracksCount))
    {
      uint32 autoRepeatLocalIndex = untranslatedIndex - insertionPoint;
      return autoRepeatTrackStyles[autoRepeatLocalIndex % autoRepeatTrackStyles.size()];
    }

    return trackStyles[untranslatedIndex - autoRepeatTracksCount];
  }

  LayoutUnit GridTrackSizingAlgorithm::ComputeTrackBasedSize() const noexcept
  {
    if (IsDirectionInMasonryDirection())
    {
      return _renderGrid->MasonryContentSize();
    }

    auto rangeToIgnore = layout::IndexRange {};
    if (_renderGrid->ShouldCheckExplicitIntrinsicInnerLogicalSize(_direction)
        && _renderGrid->GetAutoRepeatType(_direction) == AutoRepeatType::Fit)
    {
      rangeToIgnore = _renderGrid->AutoRepeatTracksRange(_direction);
    }

    auto &tracks = GetTracks(_direction);
    LayoutUnit size;
    for (size_t i = 0; i < tracks.size(); ++i)
    {
      if (rangeToIgnore.Distance() && i == rangeToIgnore.Begin())
      {
        i += rangeToIgnore.Distance() - 1;
        continue;
      }
      size += tracks[i]->BaseSize();
    }

    size += _renderGrid->GuttersSize(
      _direction, 0, static_cast<uint32>(tracks.size() - rangeToIgnore.Distance()), AvailableSpace());

    return size;
  }

  LayoutUnit GridTrackSizingAlgorithm::InitialBaseSize(const style::GridTrackSize &trackSize) const noexcept
  {
    auto &gridLength = trackSize.MinTrackBreadth();
    if (gridLength.IsFlex())
    {
      return 0_lu;
    }

    auto &trackLength = gridLength.Length();
    if (trackLength.IsSpecified())
    {
      return EvaluateTrackLength(trackLength, std::max(AvailableSpace().value_or(0_lu), 0_lu));
    }

    krys_debug_assert(trackLength.IsMinContent() || trackLength.IsAuto() || trackLength.IsMaxContent());
    return 0_lu;
  }

  LayoutUnit GridTrackSizingAlgorithm::InitialGrowthLimit(const style::GridTrackSize &trackSize,
                                                          LayoutUnit baseSize) const noexcept
  {
    auto &gridLength = trackSize.MaxTrackBreadth();
    if (gridLength.IsFlex())
    {
      return trackSize.MinTrackBreadth().IsContentSized() ? LayoutUnit(infinity) : baseSize;
    }

    auto &trackLength = gridLength.Length();
    if (trackLength.IsSpecified())
    {
      return EvaluateTrackLength(trackLength, std::max(AvailableSpace().value_or(0_lu), 0_lu));
    }

    krys_debug_assert(trackLength.IsMinContent() || trackLength.IsAuto() || trackLength.IsMaxContent());
    return LayoutUnit(infinity);
  }

  void GridTrackSizingAlgorithm::SizeTrackToFitSingleSpanMasonryGroup(
    const GridSpan &span, MasonryMinMaxTrackSize &masonryIndefiniteItems, GridTrack &track) noexcept
  {
    auto trackPosition = span.StartLine();
    const auto &trackSize = GetTracks(_direction)[trackPosition]->CachedTrackSize();

    if (trackSize.HasMinContentMinTrackBreadth())
    {
      track.SetBaseSize(std::max(track.BaseSize(), masonryIndefiniteItems.MinContentSize));
    }
    else if (trackSize.HasMaxContentMinTrackBreadth())
    {
      track.SetBaseSize(std::max(track.BaseSize(), masonryIndefiniteItems.MaxContentSize));
    }
    else if (trackSize.HasAutoMinTrackBreadth())
    {
      track.SetBaseSize(std::max(track.BaseSize(), masonryIndefiniteItems.MinSize));
    }

    if (trackSize.HasMinContentMaxTrackBreadth())
    {
      track.SetGrowthLimit(std::max(track.GrowthLimit(), masonryIndefiniteItems.MinContentSize));
    }
    else if (trackSize.HasMaxContentOrAutoMaxTrackBreadth())
    {
      auto growthLimit = masonryIndefiniteItems.MaxContentSize;
      if (trackSize.IsFitContent())
      {
        growthLimit =
          std::min(growthLimit, EvaluateTrackLength(trackSize.FitContentTrackLength(), AvailableSpace()));
      }
      track.SetGrowthLimit(std::max(track.GrowthLimit(), growthLimit));
    }
  }

  void GridTrackSizingAlgorithm::SizeTrackToFitNonSpanningItem(const GridSpan &span, RenderBox &gridItem,
                                                               GridTrack &track,
                                                               GridLayoutState &gridLayoutState) noexcept
  {
    uint32 trackPosition = span.StartLine();
    const auto &trackSize = GetTracks(_direction)[trackPosition]->CachedTrackSize();

    if (trackSize.HasMinContentMinTrackBreadth())
    {
      track.SetBaseSize(
        std::max(track.BaseSize(), _strategy->MinContentContributionForGridItem(gridItem, gridLayoutState)));
    }
    else if (trackSize.HasMaxContentMinTrackBreadth())
    {
      track.SetBaseSize(
        std::max(track.BaseSize(), _strategy->MaxContentContributionForGridItem(gridItem, gridLayoutState)));
    }
    else if (trackSize.HasAutoMinTrackBreadth())
    {
      track.SetBaseSize(
        std::max(track.BaseSize(), _strategy->MinContributionForGridItem(gridItem, gridLayoutState)));
    }

    if (trackSize.HasMinContentMaxTrackBreadth())
    {
      track.SetGrowthLimit(std::max(track.GrowthLimit(),
                                    _strategy->MinContentContributionForGridItem(gridItem, gridLayoutState)));
    }
    else if (trackSize.HasMaxContentOrAutoMaxTrackBreadth())
    {
      LayoutUnit growthLimit = _strategy->MaxContentContributionForGridItem(gridItem, gridLayoutState);
      if (trackSize.IsFitContent())
      {
        growthLimit =
          std::min(growthLimit, EvaluateTrackLength(trackSize.FitContentTrackLength(), AvailableSpace()));
      }
      track.SetGrowthLimit(std::max(track.GrowthLimit(), growthLimit));
    }
  }

  bool
    GridTrackSizingAlgorithm::SpanningItemCrossesFlexibleSizedTracks(const GridSpan &itemSpan) const noexcept
  {
    auto &trackList = GetTracks(_direction);
    for (auto trackPosition : itemSpan)
    {
      const auto &trackSize = trackList[trackPosition]->CachedTrackSize();
      if (trackSize.MinTrackBreadth().IsFlex() || trackSize.MaxTrackBreadth().IsFlex())
      {
        return true;
      }
    }

    return false;
  }

  /// @brief A grid item with its span in the direction being sized.
  class GridItemWithSpan
  {
  private:
    RenderBox *_gridItem;
    GridSpan _span;

  public:
    GridItemWithSpan(RenderBox &gridItem, GridSpan span) noexcept : _gridItem(&gridItem), _span(span)
    {
    }

    KRYS_NODISCARD RenderBox &GridItem() const noexcept
    {
      return *_gridItem;
    }
    KRYS_NODISCARD GridSpan Span() const noexcept
    {
      return _span;
    }

    KRYS_NODISCARD bool operator<(const GridItemWithSpan &other) const noexcept
    {
      return _span.IntegerSpan() < other._span.IntegerSpan();
    }
  };

  namespace
  {
    enum class TrackSizeRestriction : uint8
    {
      AllowInfinity,
      ForbidInfinity,
    };

    KRYS_NODISCARD bool
      ShouldProcessTrackForTrackSizeComputationPhase(TrackSizeComputationPhase phase,
                                                     const style::GridTrackSize &trackSize) noexcept
    {
      switch (phase)
      {
        case TrackSizeComputationPhase::ResolveIntrinsicMinimums:
          return trackSize.HasIntrinsicMinTrackBreadth();
        case TrackSizeComputationPhase::ResolveContentBasedMinimums:
          return trackSize.HasMinOrMaxContentMinTrackBreadth();
        case TrackSizeComputationPhase::ResolveMaxContentMinimums:
          return trackSize.HasMaxContentMinTrackBreadth();
        case TrackSizeComputationPhase::ResolveIntrinsicMaximums:
          return trackSize.HasIntrinsicMaxTrackBreadth();
        case TrackSizeComputationPhase::ResolveMaxContentMaximums:
          return trackSize.HasMaxContentOrAutoMaxTrackBreadth();
        case TrackSizeComputationPhase::MaximizeTracks: krys_debug_assert(false); return false;
      }

      krys_debug_assert(false);
      return false;
    }

    KRYS_NODISCARD LayoutUnit TrackSizeForTrackSizeComputationPhase(TrackSizeComputationPhase phase,
                                                                    const GridTrack &track,
                                                                    TrackSizeRestriction restriction) noexcept
    {
      switch (phase)
      {
        case TrackSizeComputationPhase::ResolveIntrinsicMinimums:
        case TrackSizeComputationPhase::ResolveContentBasedMinimums:
        case TrackSizeComputationPhase::ResolveMaxContentMinimums:
        case TrackSizeComputationPhase::MaximizeTracks:              return track.BaseSize();
        case TrackSizeComputationPhase::ResolveIntrinsicMaximums:
        case TrackSizeComputationPhase::ResolveMaxContentMaximums:
          return restriction == TrackSizeRestriction::AllowInfinity ? track.GrowthLimit()
                                                                    : track.GrowthLimitIfNotInfinite();
      }

      krys_debug_assert(false);
      return track.BaseSize();
    }

    void UpdateTrackSizeForTrackSizeComputationPhase(TrackSizeComputationPhase phase,
                                                     GridTrack &track) noexcept
    {
      switch (phase)
      {
        case TrackSizeComputationPhase::ResolveIntrinsicMinimums:
        case TrackSizeComputationPhase::ResolveContentBasedMinimums:
        case TrackSizeComputationPhase::ResolveMaxContentMinimums:
          track.SetBaseSize(track.PlannedSize());
          return;
        case TrackSizeComputationPhase::ResolveIntrinsicMaximums:
        case TrackSizeComputationPhase::ResolveMaxContentMaximums:
          track.SetGrowthLimit(track.PlannedSize());
          return;
        case TrackSizeComputationPhase::MaximizeTracks: krys_debug_assert(false); return;
      }

      krys_debug_assert(false);
    }

    KRYS_NODISCARD bool TrackShouldGrowBeyondGrowthLimitsForTrackSizeComputationPhase(
      TrackSizeComputationPhase phase, const style::GridTrackSize &trackSize) noexcept
    {
      switch (phase)
      {
        case TrackSizeComputationPhase::ResolveIntrinsicMinimums:
        case TrackSizeComputationPhase::ResolveContentBasedMinimums:
          return trackSize.HasAutoOrMinContentMinTrackBreadthAndIntrinsicMaxTrackBreadth();
        case TrackSizeComputationPhase::ResolveMaxContentMinimums:
          return trackSize.HasMaxContentMinTrackBreadthAndMaxContentMaxTrackBreadth();
        case TrackSizeComputationPhase::ResolveIntrinsicMaximums:
        case TrackSizeComputationPhase::ResolveMaxContentMaximums: return true;
        case TrackSizeComputationPhase::MaximizeTracks:            krys_debug_assert(false); return false;
      }

      krys_debug_assert(false);
      return false;
    }

    void MarkAsInfinitelyGrowableForTrackSizeComputationPhase(TrackSizeComputationPhase phase,
                                                              GridTrack &track) noexcept
    {
      switch (phase)
      {
        case TrackSizeComputationPhase::ResolveIntrinsicMinimums:
        case TrackSizeComputationPhase::ResolveContentBasedMinimums:
        case TrackSizeComputationPhase::ResolveMaxContentMinimums:   return;
        case TrackSizeComputationPhase::ResolveIntrinsicMaximums:
          if (TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::AllowInfinity)
                == infinity
              && track.PlannedSize() != infinity)
          {
            track.SetInfinitelyGrowable(true);
          }
          return;
        case TrackSizeComputationPhase::ResolveMaxContentMaximums:
          if (track.InfinitelyGrowable())
          {
            track.SetInfinitelyGrowable(false);
          }
          return;
        case TrackSizeComputationPhase::MaximizeTracks: krys_debug_assert(false); return;
      }

      krys_debug_assert(false);
    }

    template <TrackSizeComputationVariant variant>
    KRYS_NODISCARD double GetSizeDistributionWeight(const GridTrack &track) noexcept
    {
      if constexpr (variant != TrackSizeComputationVariant::CrossingFlexibleTracks)
      {
        return 0;
      }
      else
      {
        krys_debug_assert(track.CachedTrackSize().MaxTrackBreadth().IsFlex());
        return track.CachedTrackSize().MaxTrackBreadth().GetFlex().Value;
      }
    }

    KRYS_NODISCARD bool SortByGridTrackGrowthPotential(const GridTrack *track1,
                                                       const GridTrack *track2) noexcept
    {
      // This check ensures that we respect the irreflexivity property of the strict weak ordering required by
      // std::sort (forall x: NOT x < x).
      bool track1HasInfiniteGrowthPotentialWithoutCap =
        track1->InfiniteGrowthPotential() && !track1->GrowthLimitCap();
      bool track2HasInfiniteGrowthPotentialWithoutCap =
        track2->InfiniteGrowthPotential() && !track2->GrowthLimitCap();

      if (track1HasInfiniteGrowthPotentialWithoutCap && track2HasInfiniteGrowthPotentialWithoutCap)
      {
        return false;
      }

      if (track1HasInfiniteGrowthPotentialWithoutCap || track2HasInfiniteGrowthPotentialWithoutCap)
      {
        return track2HasInfiniteGrowthPotentialWithoutCap;
      }

      LayoutUnit track1Limit = track1->GrowthLimitCap().value_or(track1->GrowthLimit());
      LayoutUnit track2Limit = track2->GrowthLimitCap().value_or(track2->GrowthLimit());
      return (track1Limit - track1->BaseSize()) < (track2Limit - track2->BaseSize());
    }

    void ClampGrowthShareIfNeeded(TrackSizeComputationPhase phase, const GridTrack &track,
                                  LayoutUnit &growthShare) noexcept
    {
      if (phase != TrackSizeComputationPhase::ResolveMaxContentMaximums || !track.GrowthLimitCap())
      {
        return;
      }

      LayoutUnit distanceToCap = track.GrowthLimitCap().value() - track.TempSize();
      if (distanceToCap <= 0)
      {
        return;
      }

      growthShare = std::min(growthShare, distanceToCap);
    }

    template <TrackSizeComputationPhase phase, SpaceDistributionLimit limit>
    void DistributeItemIncurredIncreaseToTrack(GridTrack &track, LayoutUnit &freeSpace,
                                               double shareFraction) noexcept
    {
      LayoutUnit freeSpaceShare(freeSpace.ToDouble() / shareFraction);
      LayoutUnit growthShare =
        limit == SpaceDistributionLimit::BeyondGrowthLimit || track.InfiniteGrowthPotential()
          ? freeSpaceShare
          : std::min(freeSpaceShare, track.GrowthLimit()
                                       - TrackSizeForTrackSizeComputationPhase(
                                         phase, track, TrackSizeRestriction::ForbidInfinity));
      ClampGrowthShareIfNeeded(phase, track, growthShare);
      // We must never shrink any grid track or else we can't guarantee we abide by our min-sizing function.
      krys_debug_assert(growthShare >= 0);
      track.GrowTempSize(growthShare);
      freeSpace -= growthShare;
    }

    template <TrackSizeComputationVariant variant, TrackSizeComputationPhase phase,
              SpaceDistributionLimit limit>
    void DistributeItemIncurredIncreases(List<GridTrack *> &tracks, LayoutUnit &freeSpace) noexcept
    {
      auto tracksSize = static_cast<uint32>(tracks.size());
      if (!tracksSize)
      {
        return;
      }
      if constexpr (variant == TrackSizeComputationVariant::NotCrossingFlexibleTracks)
      {
        // We have to sort tracks according to their growth potential. This is necessary even when
        // distributing beyond growth limits, because there might be tracks with growth limit caps (like the
        // ones with fit-content()) which cannot indefinitely grow over the limits.
        std::ranges::sort(tracks, SortByGridTrackGrowthPotential);
        for (uint32 i = 0; i < tracksSize; ++i)
        {
          krys_debug_assert(!GetSizeDistributionWeight<variant>(*tracks[i]));
          DistributeItemIncurredIncreaseToTrack<phase, limit>(*tracks[i], freeSpace, tracksSize - i);
        }
        return;
      }
      else
      {
        // We never grow flex tracks beyond growth limits, since they are infinite.
        krys_debug_assert(limit != SpaceDistributionLimit::BeyondGrowthLimit);
        // For TrackSizeComputationVariant::CrossingFlexibleTracks we don't distribute equally, we need to
        // take the weights into account.
        List<double> fractionsOfRemainingSpace(tracksSize);
        double weightSum = 0;
        for (int32 i = static_cast<int32>(tracksSize) - 1; i >= 0; --i)
        {
          double weight = GetSizeDistributionWeight<variant>(*tracks[i]);
          weightSum += weight;
          fractionsOfRemainingSpace[i] = weightSum > 0 ? weightSum / weight : tracksSize - i;
        }
        for (uint32 i = 0; i < tracksSize; ++i)
        {
          // Sorting is not needed for TrackSizeComputationVariant::CrossingFlexibleTracks, since all tracks
          // have an infinite growth potential.
          krys_debug_assert(tracks[i]->GrowthLimitIsInfinite());
          DistributeItemIncurredIncreaseToTrack<phase, limit>(*tracks[i], freeSpace,
                                                              fractionsOfRemainingSpace[i]);
        }
      }
    }

    KRYS_NODISCARD LayoutUnit ComputeGridSpanSize(const GridTrackSizingAlgorithm::Tracks &tracks,
                                                  const GridSpan &gridSpan,
                                                  const Maybe<LayoutUnit> gridItemOffset,
                                                  const LayoutUnit totalGuttersSize) noexcept
    {
      LayoutUnit totalTracksSize;
      for (auto trackPosition : gridSpan)
      {
        totalTracksSize += tracks[trackPosition]->BaseSize();
      }
      return totalTracksSize + totalGuttersSize
             + static_cast<int>(gridSpan.IntegerSpan() - 1) * gridItemOffset.value_or(0_lu);
    }
  }

  LayoutUnit GridTrackSizingAlgorithm::ItemSizeForTrackSizeComputationPhase(
    TrackSizeComputationPhase phase, RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept
  {
    switch (phase)
    {
      case TrackSizeComputationPhase::ResolveIntrinsicMinimums:
        return _strategy->MinContributionForGridItem(gridItem, gridLayoutState);
      case TrackSizeComputationPhase::ResolveContentBasedMinimums:
      case TrackSizeComputationPhase::ResolveIntrinsicMaximums:
        return _strategy->MinContentContributionForGridItem(gridItem, gridLayoutState);
      case TrackSizeComputationPhase::ResolveMaxContentMinimums:
      case TrackSizeComputationPhase::ResolveMaxContentMaximums:
        return _strategy->MaxContentContributionForGridItem(gridItem, gridLayoutState);
      case TrackSizeComputationPhase::MaximizeTracks: krys_debug_assert(false); return 0_lu;
    }

    krys_debug_assert(false);
    return 0_lu;
  }

  LayoutUnit GridTrackSizingAlgorithm::ItemSizeForTrackSizeComputationPhaseMasonry(
    TrackSizeComputationPhase phase, const MasonryMinMaxTrackSize &trackSize) const noexcept
  {
    switch (phase)
    {
      case TrackSizeComputationPhase::ResolveIntrinsicMinimums:    return trackSize.MinSize;
      case TrackSizeComputationPhase::ResolveContentBasedMinimums:
      case TrackSizeComputationPhase::ResolveIntrinsicMaximums:    return trackSize.MinContentSize;
      case TrackSizeComputationPhase::ResolveMaxContentMinimums:
      case TrackSizeComputationPhase::ResolveMaxContentMaximums:   return trackSize.MaxContentSize;
      case TrackSizeComputationPhase::MaximizeTracks:              krys_debug_assert(false); return 0_lu;
    }

    krys_debug_assert(false);
    return 0_lu;
  }

  template <TrackSizeComputationVariant variant, TrackSizeComputationPhase phase>
  void GridTrackSizingAlgorithm::IncreaseSizesToAccommodateSpanningItems(
    GridItemsSpanGroupRange gridItemsWithSpan, GridLayoutState &gridLayoutState) noexcept
  {
    auto &allTracks = GetTracks(_direction);
    for (const auto &trackIndex : _contentSizedTracksIndex)
    {
      auto &track = *allTracks[trackIndex];
      track.SetPlannedSize(
        TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::AllowInfinity));
    }

    List<GridTrack *> growBeyondGrowthLimitsTracks;
    List<GridTrack *> filteredTracks;
    for (auto &gridItemWithSpan : gridItemsWithSpan)
    {
      const GridSpan &itemSpan = gridItemWithSpan.Span();
      krys_debug_assert(variant == TrackSizeComputationVariant::CrossingFlexibleTracks
                        || itemSpan.IntegerSpan() > 1u);

      filteredTracks.clear();
      growBeyondGrowthLimitsTracks.clear();
      LayoutUnit spanningTracksSize;
      for (auto trackPosition : itemSpan)
      {
        auto &track = *allTracks[trackPosition];
        const auto &trackSize = track.CachedTrackSize();
        spanningTracksSize +=
          TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::ForbidInfinity);
        if (variant == TrackSizeComputationVariant::CrossingFlexibleTracks
            && !trackSize.MaxTrackBreadth().IsFlex())
        {
          continue;
        }
        if (!ShouldProcessTrackForTrackSizeComputationPhase(phase, trackSize))
        {
          continue;
        }

        filteredTracks.push_back(&track);

        if (TrackShouldGrowBeyondGrowthLimitsForTrackSizeComputationPhase(phase, trackSize))
        {
          growBeyondGrowthLimitsTracks.push_back(&track);
        }
      }

      if (filteredTracks.empty())
      {
        continue;
      }

      spanningTracksSize +=
        _renderGrid->GuttersSize(_direction, itemSpan.StartLine(), itemSpan.IntegerSpan(), AvailableSpace());

      LayoutUnit extraSpace =
        ItemSizeForTrackSizeComputationPhase(phase, gridItemWithSpan.GridItem(), gridLayoutState)
        - spanningTracksSize;
      extraSpace = std::max<LayoutUnit>(extraSpace, 0_lu);
      auto &tracksToGrowBeyondGrowthLimits =
        growBeyondGrowthLimitsTracks.empty() ? filteredTracks : growBeyondGrowthLimitsTracks;
      DistributeSpaceToTracks<variant, phase>(filteredTracks, &tracksToGrowBeyondGrowthLimits, extraSpace);
    }

    for (const auto &trackIndex : _contentSizedTracksIndex)
    {
      auto &track = *allTracks[trackIndex];
      MarkAsInfinitelyGrowableForTrackSizeComputationPhase(phase, track);
      UpdateTrackSizeForTrackSizeComputationPhase(phase, track);
    }
  }

  template <TrackSizeComputationVariant variant>
  void GridTrackSizingAlgorithm::IncreaseSizesToAccommodateSpanningItems(
    GridItemsSpanGroupRange gridItemsWithSpan, GridLayoutState &gridLayoutState) noexcept
  {
    IncreaseSizesToAccommodateSpanningItems<variant, TrackSizeComputationPhase::ResolveIntrinsicMinimums>(
      gridItemsWithSpan, gridLayoutState);
    IncreaseSizesToAccommodateSpanningItems<variant, TrackSizeComputationPhase::ResolveContentBasedMinimums>(
      gridItemsWithSpan, gridLayoutState);
    IncreaseSizesToAccommodateSpanningItems<variant, TrackSizeComputationPhase::ResolveMaxContentMinimums>(
      gridItemsWithSpan, gridLayoutState);
    IncreaseSizesToAccommodateSpanningItems<variant, TrackSizeComputationPhase::ResolveIntrinsicMaximums>(
      gridItemsWithSpan, gridLayoutState);
    IncreaseSizesToAccommodateSpanningItems<variant, TrackSizeComputationPhase::ResolveMaxContentMaximums>(
      gridItemsWithSpan, gridLayoutState);
  }

  template <TrackSizeComputationVariant variant>
  void GridTrackSizingAlgorithm::IncreaseSizesToAccommodateSpanningItemsMasonry(
    std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> &definiteItemSizes) noexcept
  {
    auto increaseSizes =
      [&]<TrackSizeComputationPhase phase>(List<MasonryMinMaxTrackSizeWithGridSpan> &itemSizes)
    {
      auto &allTracks = GetTracks(_direction);
      for (const auto &trackIndex : _contentSizedTracksIndex)
      {
        auto &track = *allTracks[trackIndex];
        track.SetPlannedSize(
          TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::AllowInfinity));
      }

      List<GridTrack *> growBeyondGrowthLimitsTracks;
      List<GridTrack *> filteredTracks;

      for (auto &definiteItem : itemSizes)
      {
        const auto &itemSpan = definiteItem.Span;
        krys_debug_assert(variant == TrackSizeComputationVariant::CrossingFlexibleTracks
                          || itemSpan.IntegerSpan() > 1u);

        filteredTracks.clear();
        growBeyondGrowthLimitsTracks.clear();
        LayoutUnit spanningTracksSize;
        for (auto trackPosition : itemSpan)
        {
          auto &track = *allTracks[trackPosition];
          const auto &trackSize = track.CachedTrackSize();
          spanningTracksSize +=
            TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::ForbidInfinity);

          if (!ShouldProcessTrackForTrackSizeComputationPhase(phase, trackSize))
          {
            continue;
          }

          filteredTracks.push_back(&track);

          if (TrackShouldGrowBeyondGrowthLimitsForTrackSizeComputationPhase(phase, trackSize))
          {
            growBeyondGrowthLimitsTracks.push_back(&track);
          }
        }

        if (filteredTracks.empty())
        {
          continue;
        }

        spanningTracksSize += _renderGrid->GuttersSize(_direction, itemSpan.StartLine(),
                                                       itemSpan.IntegerSpan(), AvailableSpace());

        auto extraSpace =
          ItemSizeForTrackSizeComputationPhaseMasonry(phase, definiteItem.TrackSize) - spanningTracksSize;
        extraSpace = std::max<LayoutUnit>(extraSpace, 0_lu);
        auto &tracksToGrowBeyondGrowthLimits =
          growBeyondGrowthLimitsTracks.empty() ? filteredTracks : growBeyondGrowthLimitsTracks;
        DistributeSpaceToTracks<variant, phase>(filteredTracks, &tracksToGrowBeyondGrowthLimits, extraSpace);
      }

      for (const auto &trackIndex : _contentSizedTracksIndex)
      {
        auto &track = *allTracks[trackIndex];
        MarkAsInfinitelyGrowableForTrackSizeComputationPhase(phase, track);
        UpdateTrackSizeForTrackSizeComputationPhase(phase, track);
      }
    };

    for (auto &definiteItemSpanGroup : definiteItemSizes)
    {
      increaseSizes.template operator()<TrackSizeComputationPhase::ResolveIntrinsicMinimums>(
        definiteItemSpanGroup.second);
      increaseSizes.template operator()<TrackSizeComputationPhase::ResolveContentBasedMinimums>(
        definiteItemSpanGroup.second);
      increaseSizes.template operator()<TrackSizeComputationPhase::ResolveMaxContentMinimums>(
        definiteItemSpanGroup.second);
      increaseSizes.template operator()<TrackSizeComputationPhase::ResolveIntrinsicMaximums>(
        definiteItemSpanGroup.second);
      increaseSizes.template operator()<TrackSizeComputationPhase::ResolveMaxContentMaximums>(
        definiteItemSpanGroup.second);
    }
  }

  template <TrackSizeComputationVariant variant>
  void GridTrackSizingAlgorithm::IncreaseSizesToAccommodateSpanningItemsMasonryWithFlex(
    List<MasonryMinMaxTrackSizeWithGridSpan> &definiteItemSizesSpanFlexTracks) noexcept
  {
    auto increaseSizes =
      [&]<TrackSizeComputationPhase phase>(List<MasonryMinMaxTrackSizeWithGridSpan> &itemSizes)
    {
      auto &allTracks = GetTracks(_direction);
      for (const auto &trackIndex : _contentSizedTracksIndex)
      {
        auto &track = *allTracks[trackIndex];
        track.SetPlannedSize(
          TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::AllowInfinity));
      }

      List<GridTrack *> growBeyondGrowthLimitsTracks;
      List<GridTrack *> filteredTracks;

      for (auto &item : itemSizes)
      {
        const auto &itemSpan = item.Span;
        krys_debug_assert(variant == TrackSizeComputationVariant::CrossingFlexibleTracks
                          || itemSpan.IntegerSpan() > 1u);

        filteredTracks.clear();
        growBeyondGrowthLimitsTracks.clear();
        LayoutUnit spanningTracksSize;
        for (auto trackPosition : itemSpan)
        {
          auto &track = *allTracks[trackPosition];
          const auto &trackSize = track.CachedTrackSize();
          spanningTracksSize +=
            TrackSizeForTrackSizeComputationPhase(phase, track, TrackSizeRestriction::ForbidInfinity);
          if (!trackSize.MaxTrackBreadth().IsFlex())
          {
            continue;
          }
          if (!ShouldProcessTrackForTrackSizeComputationPhase(phase, trackSize))
          {
            continue;
          }

          filteredTracks.push_back(&track);

          if (TrackShouldGrowBeyondGrowthLimitsForTrackSizeComputationPhase(phase, trackSize))
          {
            growBeyondGrowthLimitsTracks.push_back(&track);
          }
        }

        if (filteredTracks.empty())
        {
          continue;
        }

        spanningTracksSize += _renderGrid->GuttersSize(_direction, itemSpan.StartLine(),
                                                       itemSpan.IntegerSpan(), AvailableSpace());

        auto extraSpace =
          ItemSizeForTrackSizeComputationPhaseMasonry(phase, item.TrackSize) - spanningTracksSize;
        extraSpace = std::max<LayoutUnit>(extraSpace, 0_lu);
        auto &tracksToGrowBeyondGrowthLimits =
          growBeyondGrowthLimitsTracks.empty() ? filteredTracks : growBeyondGrowthLimitsTracks;
        DistributeSpaceToTracks<variant, phase>(filteredTracks, &tracksToGrowBeyondGrowthLimits, extraSpace);
      }

      for (const auto &trackIndex : _contentSizedTracksIndex)
      {
        auto &track = *allTracks[trackIndex];
        MarkAsInfinitelyGrowableForTrackSizeComputationPhase(phase, track);
        UpdateTrackSizeForTrackSizeComputationPhase(phase, track);
      }
    };

    increaseSizes.template operator()<TrackSizeComputationPhase::ResolveIntrinsicMinimums>(
      definiteItemSizesSpanFlexTracks);
    increaseSizes.template operator()<TrackSizeComputationPhase::ResolveContentBasedMinimums>(
      definiteItemSizesSpanFlexTracks);
    increaseSizes.template operator()<TrackSizeComputationPhase::ResolveMaxContentMinimums>(
      definiteItemSizesSpanFlexTracks);
    increaseSizes.template operator()<TrackSizeComputationPhase::ResolveIntrinsicMaximums>(
      definiteItemSizesSpanFlexTracks);
    increaseSizes.template operator()<TrackSizeComputationPhase::ResolveMaxContentMaximums>(
      definiteItemSizesSpanFlexTracks);
  }

  void GridTrackSizingAlgorithm::ConvertIndefiniteItemsToDefiniteMasonry(
    const std::map<SpanLength, MasonryMinMaxTrackSize> &indefiniteSpanSizes,
    std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> &definiteItemSizes,
    List<MasonryMinMaxTrackSizeWithGridSpan> &definiteItemSizesSpanFlexTracks) noexcept
  {
    auto &allTracks = GetTracks(_direction);

    for (auto &indefiniteItem : indefiniteSpanSizes)
    {
      for (size_t trackIndex = 0; trackIndex < allTracks.size(); trackIndex++)
      {
        auto endLine = trackIndex + indefiniteItem.first;
        if (endLine > allTracks.size())
        {
          continue;
        }

        auto itemSpan =
          GridSpan::TranslatedDefiniteGridSpan(static_cast<uint32>(trackIndex), static_cast<uint32>(endLine));

        // The spec requires items with a span of 1 to be handled earlier.
        auto crossesFlexibleTracks = SpanningItemCrossesFlexibleSizedTracks(itemSpan);
        if (itemSpan.IntegerSpan() != 1 && !crossesFlexibleTracks)
        {
          definiteItemSizes[itemSpan.IntegerSpan()].push_back({indefiniteItem.second, itemSpan});
        }

        if (crossesFlexibleTracks)
        {
          definiteItemSizesSpanFlexTracks.push_back({indefiniteItem.second, itemSpan});
        }
      }
    }
  }

  template <TrackSizeComputationVariant variant, TrackSizeComputationPhase phase>
  void GridTrackSizingAlgorithm::DistributeSpaceToTracks(List<GridTrack *> &tracks,
                                                         List<GridTrack *> *growBeyondGrowthLimitsTracks,
                                                         LayoutUnit &freeSpace) const noexcept
  {
    krys_debug_assert(freeSpace >= 0);

    for (auto *track : tracks)
    {
      track->SetTempSize(
        TrackSizeForTrackSizeComputationPhase(phase, *track, TrackSizeRestriction::ForbidInfinity));
    }

    if (freeSpace > 0)
    {
      DistributeItemIncurredIncreases<variant, phase, SpaceDistributionLimit::UpToGrowthLimit>(tracks,
                                                                                               freeSpace);
    }

    if (freeSpace > 0 && growBeyondGrowthLimitsTracks)
    {
      DistributeItemIncurredIncreases<variant, phase, SpaceDistributionLimit::BeyondGrowthLimit>(
        *growBeyondGrowthLimitsTracks, freeSpace);
    }

    for (auto *track : tracks)
    {
      track->SetPlannedSize(track->PlannedSize() == infinity
                              ? track->TempSize()
                              : std::max(track->PlannedSize(), track->TempSize()));
    }
  }

  Maybe<LayoutUnit> GridTrackSizingAlgorithm::EstimatedGridAreaBreadthForGridItem(
    const RenderBox &gridItem, GridTrackSizingDirection direction) const noexcept
  {
    const GridSpan &span = _renderGrid->GridSpanForGridItem(gridItem, direction);
    LayoutUnit gridAreaSize;
    bool gridAreaIsIndefinite = false;
    Maybe<LayoutUnit> availableSize = AvailableSpace(direction);
    for (auto trackPosition : span)
    {
      // We may need to estimate the grid area size before running the track sizing algorithm in order to
      // perform the pre-layout of orthogonal items. We cannot use
      // tracks(direction)[trackPosition].cachedTrackSize() because tracks(direction) is empty, since we are
      // either performing pre-layout or are running the track sizing algorithm in the opposite direction and
      // haven't run it in the desired direction yet.
      const auto trackSize = WasSetup() ? CalculateGridTrackSize(direction, trackPosition)
                                        : RawGridTrackSize(direction, trackPosition);
      auto &maxTrackSize = trackSize.MaxTrackBreadth();
      if (maxTrackSize.IsContentSized() || maxTrackSize.IsFlex()
          || GridLayoutFunctions::IsRelativeGridTrackBreadthAsAuto(maxTrackSize, AvailableSpace(direction)))
      {
        gridAreaIsIndefinite = true;
      }
      else
      {
        gridAreaSize += EvaluateTrackLength(maxTrackSize.Length(), availableSize);
      }
    }

    gridAreaSize += _renderGrid->GuttersSize(direction, span.StartLine(), span.IntegerSpan(), availableSize);

    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *_renderGrid, gridItem, GridTrackSizingDirection::Columns);
    if (gridAreaIsIndefinite)
    {
      return direction == gridItemInlineDirection
               ? Maybe<LayoutUnit>(std::max(gridItem.MaxPreferredLogicalWidth(), gridAreaSize))
               : null;
    }
    return gridAreaSize;
  }

  Maybe<LayoutUnit>
    GridTrackSizingAlgorithm::GridAreaBreadthForGridItem(const RenderBox &gridItem,
                                                         GridTrackSizingDirection direction) const noexcept
  {
    // FIXME: These checks only works if we have precomputed logical width/height of the grid, which is not
    // guaranteed.
    if (_renderGrid->AreMasonryColumns() && direction == GridTrackSizingDirection::Columns)
    {
      return _renderGrid->ContentBoxLogicalWidth();
    }

    if (_renderGrid->AreMasonryRows() && direction == GridTrackSizingDirection::Rows
        && !GridLayoutFunctions::IsOrthogonalGridItem(*_renderGrid, gridItem))
    {
      return _renderGrid->ContentBoxLogicalHeight();
    }

    bool addContentAlignmentOffset =
      direction == GridTrackSizingDirection::Columns
      && (_sizingState == SizingState::RowSizingFirstIteration
          || _sizingState == SizingState::RowSizingExtraIterationForSizeContainment);
    // To determine the column track's size based on an orthogonal grid item we need it's logical
    // height, which may depend on the row track's size. It's possible that the row tracks sizing
    // logic has not been performed yet, so we will need to do an estimation.
    if (direction == GridTrackSizingDirection::Rows
        && (_sizingState == SizingState::ColumnSizingFirstIteration
            || _sizingState == SizingState::ColumnSizingSecondIteration)
        && !_renderGrid->AreMasonryColumns())
    {
      if (_sizingState == SizingState::ColumnSizingFirstIteration)
      {
        auto spannedRowsSize = EstimatedGridAreaBreadthForGridItem(gridItem, GridTrackSizingDirection::Rows);
        if (spannedRowsSize)
        {
          if (auto availableLogicalHeight = _renderGrid->AvailableLogicalHeightForContentBox();
              availableLogicalHeight && HasAllLengthRowSizes())
          {
            auto contentDistributionForRows = _renderGrid->ComputeContentPositionAndDistributionOffset(
              GridTrackSizingDirection::Rows, *availableLogicalHeight - *spannedRowsSize,
              _renderGrid->NumTracks(GridTrackSizingDirection::Rows));
            auto rowSpanForGridItem =
              _renderGrid->GridSpanForGridItem(gridItem, GridTrackSizingDirection::Rows);
            return *spannedRowsSize
                   + contentDistributionForRows.DistributionOffset
                       * static_cast<int>(rowSpanForGridItem.IntegerSpan() - 1);
          }
        }
        return spannedRowsSize;
      }
      addContentAlignmentOffset = true;
    }

    const GridSpan &span = _renderGrid->GridSpanForGridItem(gridItem, direction);
    return ComputeGridSpanSize(
      GetTracks(direction), span,
      addContentAlignmentOffset ? Maybe<LayoutUnit>(_renderGrid->GridItemOffset(direction)) : null,
      _renderGrid->GuttersSize(direction, span.StartLine(), span.IntegerSpan(), AvailableSpace(direction)));
  }

  bool GridTrackSizingAlgorithm::IsIntrinsicSizedGridArea(
    const RenderBox &gridItem, GridTrackSizingDirection gridAreaDirection) const noexcept
  {
    krys_debug_assert(WasSetup());
    const GridSpan &span = _renderGrid->GridSpanForGridItem(gridItem, gridAreaDirection);
    for (auto trackPosition : span)
    {
      const auto &trackSize = RawGridTrackSize(gridAreaDirection, trackPosition);
      // We consider fr units as 'auto' for the min sizing function.
      // FIXME(jfernandez): https://github.com/w3c/csswg-drafts/issues/2611
      //
      // The use of AvailableSize function may imply different results
      // for the same item when assuming indefinite or definite size
      // constraints depending on the phase we evaluate the item's
      // baseline participation.
      // FIXME(jfernandez): https://github.com/w3c/csswg-drafts/issues/3046
      if (trackSize.IsContentSized() || trackSize.IsFitContent() || trackSize.MinTrackBreadth().IsFlex()
          || (trackSize.MaxTrackBreadth().IsFlex() && !AvailableSpace(gridAreaDirection)))
      {
        return true;
      }
    }
    return false;
  }

  style::GridTrackSize GridTrackSizingAlgorithm::CalculateGridTrackSize(GridTrackSizingDirection direction,
                                                                        uint32 translatedIndex) const noexcept
  {
    krys_debug_assert(WasSetup());
    // Collapse empty auto repeat tracks if auto-fit.
    if (_grid.HasAutoRepeatEmptyTracks(direction) && _grid.IsEmptyAutoRepeatTrack(direction, translatedIndex))
    {
      return style::GridTrackSize {0_css_px};
    }

    auto &trackSize = RawGridTrackSize(direction, translatedIndex);
    if (trackSize.IsFitContent())
    {
      if (GridLayoutFunctions::IsRelativeGridTrackBreadthAsAuto(trackSize.FitContentTrackLength(),
                                                                AvailableSpace(direction)))
      {
        return style::GridTrackSize::MinMax {{style::GridTrackBreadth {css::keywords::Auto {}},
                                              style::GridTrackBreadth {css::keywords::MaxContent {}}}};
      }
      return trackSize;
    }

    auto minTrackBreadth = trackSize.MinTrackBreadth();
    auto maxTrackBreadth = trackSize.MaxTrackBreadth();
    // If the logical width/height of the grid container is indefinite, percentage
    // values are treated as <auto>.
    if (GridLayoutFunctions::IsRelativeGridTrackBreadthAsAuto(trackSize.MinTrackBreadth(),
                                                              AvailableSpace(direction)))
    {
      minTrackBreadth = css::keywords::Auto {};
    }
    if (GridLayoutFunctions::IsRelativeGridTrackBreadthAsAuto(trackSize.MaxTrackBreadth(),
                                                              AvailableSpace(direction)))
    {
      maxTrackBreadth = css::keywords::Auto {};
    }

    // Flex sizes are invalid as a min sizing function. However we still can have a flexible |minTrackBreadth|
    // if the track size is just a flex size (e.g. "1fr"), the spec says that in this case it implies an
    // automatic minimum.
    if (minTrackBreadth.IsFlex())
    {
      minTrackBreadth = css::keywords::Auto {};
    }

    return style::GridTrackSize::MinMax {{krys::move(minTrackBreadth), krys::move(maxTrackBreadth)}};
  }

  double GridTrackSizingAlgorithm::ComputeFlexFactorUnitSize(
    const Tracks &tracks, double flexFactorSum, LayoutUnit &leftOverSpace,
    const List<uint32> &flexibleTracksIndexes,
    UniquePtr<TrackIndexSet> tracksToTreatAsInflexible) const noexcept
  {
    // We want to avoid the effect of flex factors sum below 1 making the factor unit size to grow
    // exponentially.
    double hypotheticalFactorUnitSize = leftOverSpace.ToDouble() / std::max<double>(1, flexFactorSum);

    // product of the hypothetical "flex factor unit" and any flexible track's "flex factor" must be grater
    // than such track's "base size".
    bool validFlexFactorUnit = true;
    for (auto index : flexibleTracksIndexes)
    {
      if (tracksToTreatAsInflexible && tracksToTreatAsInflexible->contains(index))
      {
        continue;
      }
      auto baseSize = tracks[index]->BaseSize();
      auto flexFactor = tracks[index]->CachedTrackSize().MaxTrackBreadth().GetFlex().Value;
      // treating all such tracks as inflexible.
      if (baseSize.ToDouble() > hypotheticalFactorUnitSize * flexFactor)
      {
        leftOverSpace -= baseSize;
        flexFactorSum -= flexFactor;
        if (!tracksToTreatAsInflexible)
        {
          tracksToTreatAsInflexible = CreateUnique<TrackIndexSet>();
        }
        tracksToTreatAsInflexible->insert(index);
        validFlexFactorUnit = false;
      }
    }
    if (!validFlexFactorUnit)
    {
      return ComputeFlexFactorUnitSize(tracks, flexFactorSum, leftOverSpace, flexibleTracksIndexes,
                                       krys::move(tracksToTreatAsInflexible));
    }
    return hypotheticalFactorUnitSize;
  }

  void GridTrackSizingAlgorithm::ComputeFlexSizedTracksGrowth(double flexFraction,
                                                              List<LayoutUnit> &increments,
                                                              LayoutUnit &totalGrowth) const noexcept
  {
    size_t numFlexTracks = _flexibleSizedTracksIndex.size();
    krys_debug_assert(increments.size() == numFlexTracks);
    auto &allTracks = GetTracks(_direction);
    // The flexFraction multiplied by the flex factor can result in a non-integer size. Since we floor the
    // stretched size to fit in a LayoutUnit, we may lose the fractional part of the computation which can
    // cause the entire free space not being distributed evenly. The leftover fractional part from every
    // flexible track are accumulated here to avoid this issue.
    double leftOverSize = 0;
    for (size_t i = 0; i < numFlexTracks; ++i)
    {
      uint32 trackIndex = _flexibleSizedTracksIndex[i];
      const auto &trackSize = allTracks[trackIndex]->CachedTrackSize();
      krys_debug_assert(trackSize.MaxTrackBreadth().IsFlex());
      LayoutUnit oldBaseSize = allTracks[trackIndex]->BaseSize();
      auto frShare = flexFraction * trackSize.MaxTrackBreadth().GetFlex().Value + leftOverSize;
      auto stretchedSize = LayoutUnit(frShare);
      LayoutUnit newBaseSize = std::max(oldBaseSize, stretchedSize);
      increments[i] = newBaseSize - oldBaseSize;
      totalGrowth += increments[i];
      // In the case that stretchedSize is greater than frShare, we floor it to 0 to avoid a negative
      // leftover.
      leftOverSize = std::max(frShare - stretchedSize.ToDouble(), 0.0);
    }
  }

  double GridTrackSizingAlgorithm::FindFrUnitSize(const GridSpan &tracksSpan,
                                                  LayoutUnit leftOverSpace) const noexcept
  {
    if (leftOverSpace <= 0)
    {
      return 0;
    }

    auto &allTracks = GetTracks(_direction);
    double flexFactorSum = 0;
    List<uint32> flexibleTracksIndexes;
    for (auto trackIndex : tracksSpan)
    {
      const auto &trackSize = allTracks[trackIndex]->CachedTrackSize();
      if (!trackSize.MaxTrackBreadth().IsFlex())
      {
        leftOverSpace -= allTracks[trackIndex]->BaseSize();
      }
      else
      {
        auto flexFactor = trackSize.MaxTrackBreadth().GetFlex().Value;
        flexibleTracksIndexes.push_back(trackIndex);
        flexFactorSum += flexFactor;
      }
    }
    // We don't remove the gutters from left_over_space here, because that was already done before.

    // The function is not called if we don't have <flex> grid tracks.
    krys_debug_assert(!flexibleTracksIndexes.empty());

    return ComputeFlexFactorUnitSize(allTracks, flexFactorSum, leftOverSpace, flexibleTracksIndexes);
  }

  void GridTrackSizingAlgorithm::ComputeGridContainerIntrinsicSizes() noexcept
  {
    if (_direction == GridTrackSizingDirection::Columns
        && _strategy->IsComputingSizeOrInlineSizeContainment())
    {
      if (auto size = _renderGrid->ExplicitIntrinsicInnerLogicalSize(_direction))
      {
        _minContentSize = size.value();
        _maxContentSize = size.value();
        return;
      }
    }

    _minContentSize = _maxContentSize = 0_lu;

    bool hasAutoRepeatFitTracks = _renderGrid->GetAutoRepeatType(_direction) == AutoRepeatType::Fit;
    auto autoRepeatTracksRange = _renderGrid->AutoRepeatTracksRange(_direction);
    auto &allTracks = GetTracks(_direction);
    for (size_t trackIndex = 0; trackIndex < allTracks.size(); ++trackIndex)
    {
      auto &track = *allTracks[trackIndex];
      krys_debug_assert(_strategy->IsComputingSizeOrInlineSizeContainment()
                        || !track.InfiniteGrowthPotential());

      if (_strategy->IsComputingSizeOrInlineSizeContainment() && hasAutoRepeatFitTracks
          && trackIndex >= autoRepeatTracksRange.Begin() && trackIndex < autoRepeatTracksRange.End())
      {
        continue;
      }

      _minContentSize += track.BaseSize();
      _maxContentSize += track.GrowthLimitIsInfinite() ? track.BaseSize() : track.GrowthLimit();
      // The growth limit caps must be cleared now in order to properly sort
      // tracks by growth potential on an eventual "Maximize Tracks".
      track.SetGrowthLimitCap(null);
    }
  }

  // GridTrackSizingAlgorithmStrategy.
  LayoutUnit GridTrackSizingAlgorithmStrategy::LogicalHeightForGridItem(
    RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept
  {
    auto gridItemBlockDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Rows);

    auto &intrinsicLogicalHeightsForRowSizingFirstPass =
      GetRenderGrid()->IntrinsicLogicalHeightsForRowSizingFirstPass();
    if (intrinsicLogicalHeightsForRowSizingFirstPass && !gridItem.NeedsLayout()
        && GetSizingState() == GridTrackSizingAlgorithm::SizingState::RowSizingFirstIteration)
    {
      if (auto cachedLogicalHeight = intrinsicLogicalHeightsForRowSizingFirstPass->SizeForItem(gridItem))
      {
        return *cachedLogicalHeight;
      }
    }

    // If |gridItem| has a relative logical height, we shouldn't let it override its intrinsic height, which
    // is what we are interested in here. Thus we need to set the block-axis override size to nullopt (no
    // possible resolution).
    auto hasOverridingContainingBlockContentSizeForGridItem = [&]
    {
      auto overridingSize = GridLayoutFunctions::OverridingContainingBlockContentSizeForGridItem(
        gridItem, GridTrackSizingDirection::Rows);
      return overridingSize && *overridingSize;
    };
    if (hasOverridingContainingBlockContentSizeForGridItem()
        && ShouldClearOverridingContainingBlockContentSizeForGridItem(gridItem,
                                                                      GridTrackSizingDirection::Rows))
    {
      SetOverridingContainingBlockContentSizeForGridItem(*GetRenderGrid(), gridItem, gridItemBlockDirection,
                                                         null);
      gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);

      if (GetRenderGrid()->CanSetColumnAxisStretchRequirementForItem(gridItem))
      {
        gridLayoutState.SetLayoutRequirementForGridItem(
          gridItem, ItemLayoutRequirement::NeedsColumnAxisStretchAlignment);
      }
    }

    // We need to clear the stretched content size to properly compute logical height during layout.
    if (gridItem.NeedsLayout())
    {
      gridItem.ClearOverridingSize();
    }

    gridItem.LayoutIfNeeded();

    auto gridItemLogicalHeight =
      gridItem.LogicalHeight()
      + GridLayoutFunctions::MarginLogicalSizeForGridItem(*GetRenderGrid(), gridItemBlockDirection, gridItem)
      + _algorithm.BaselineOffsetForGridItem(gridItem, Direction());
    if (intrinsicLogicalHeightsForRowSizingFirstPass
        && GetSizingState() == GridTrackSizingAlgorithm::SizingState::RowSizingFirstIteration)
    {
      intrinsicLogicalHeightsForRowSizingFirstPass->SetSizeForGridItem(gridItem, gridItemLogicalHeight);
    }

    return gridItemLogicalHeight;
  }

  LayoutUnit GridTrackSizingAlgorithmStrategy::MinContentContributionForGridItem(
    RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept
  {
    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Columns);
    if (Direction() == gridItemInlineDirection)
    {
      if (IsComputingInlineSizeContainment())
      {
        return {};
      }

      bool needsGridItemMinContentContributionForSecondColumnPass =
        GetSizingState() == GridTrackSizingAlgorithm::SizingState::ColumnSizingSecondIteration
        && gridLayoutState.ContainsLayoutRequirementForGridItem(
          gridItem, ItemLayoutRequirement::MinContentContributionForSecondColumnPass);

      // FIXME: It's unclear if we should return the intrinsic width or the preferred width.
      // See http://lists.w3.org/Archives/Public/www-style/2013Jan/0245.html
      if (gridItem.ShouldInvalidatePreferredWidths()
          || needsGridItemMinContentContributionForSecondColumnPass)
      {
        gridItem.SetNeedsPreferredWidthsUpdate();
      }

      if (needsGridItemMinContentContributionForSecondColumnPass)
      {
        auto rowSize = GetRenderGrid()->GridAreaBreadthForGridItemIncludingAlignmentOffsets(
          gridItem, GridTrackSizingDirection::Rows);
        auto stretchedSize = !GridLayoutFunctions::IsOrthogonalGridItem(*GetRenderGrid(), gridItem)
                               ? gridItem.ConstrainLogicalHeightByMinMax(rowSize, null)
                               : gridItem.ConstrainLogicalWidthByMinMax(
                                   rowSize, GetRenderGrid()->ContentBoxWidth(), *GetRenderGrid());
        GridLayoutFunctions::SetOverridingContentSizeForGridItem(*GetRenderGrid(), gridItem, stretchedSize,
                                                                 GridTrackSizingDirection::Rows);
      }

      auto minContentLogicalWidth = gridItem.MinPreferredLogicalWidth();

      if (needsGridItemMinContentContributionForSecondColumnPass)
      {
        GridLayoutFunctions::ClearOverridingContentSizeForGridItem(*GetRenderGrid(), gridItem,
                                                                   GridTrackSizingDirection::Rows);
      }

      auto minLogicalWidth = [&]
      {
        auto &gridItemLogicalMinWidth = gridItem.Style().LogicalMinWidth();

        if (auto fixedGridItemLogicalMinWidth = gridItemLogicalMinWidth.TryFixed())
        {
          return LayoutUnit {fixedGridItemLogicalMinWidth->ResolveZoom(gridItem.Style().UsedZoomForLength())};
        }
        if (gridItemLogicalMinWidth.IsMaxContent())
        {
          return gridItem.MaxPreferredLogicalWidth();
        }

        // FIXME: We should be able to handle other values for the logical min width.
        return 0_lu;
      }();

      return std::max(minContentLogicalWidth, minLogicalWidth)
             + GridLayoutFunctions::MarginLogicalSizeForGridItem(*GetRenderGrid(), gridItemInlineDirection,
                                                                 gridItem)
             + _algorithm.BaselineOffsetForGridItem(gridItem, Direction());
    }

    if (UpdateOverridingContainingBlockContentSizeForGridItem(gridItem, gridItemInlineDirection))
    {
      gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);

      if (auto &intrinsicLogicalHeightsForRowSizingFirstPass =
            GetRenderGrid()->IntrinsicLogicalHeightsForRowSizingFirstPass())
      {
        intrinsicLogicalHeightsForRowSizingFirstPass->InvalidateSizeForItem(gridItem);
      }

      // For a grid item with relative width constraints to the grid area, such as percentaged paddings, we
      // reset the overridingContainingBlockContentSizeForGridItem value for columns when we are executing a
      // definite strategy for columns. Since we have updated the
      // overridingContainingBlockContentSizeForGridItem inline-axis/width value here, we might need to
      // recompute the grid item's relative width. For some cases, we probably will not be able to do it
      // during the RenderGrid::layoutGridItems() function as the grid area does't change there any more.
      // Also, as we are doing a layout inside GridTrackSizingAlgorithmStrategy::logicalHeightForGridItem()
      // function, let's take the advantage and set it here.
      if (ShouldClearOverridingContainingBlockContentSizeForGridItem(gridItem, gridItemInlineDirection))
      {
        gridItem.SetNeedsPreferredWidthsUpdate();
      }
    }
    return LogicalHeightForGridItem(gridItem, gridLayoutState);
  }

  LayoutUnit GridTrackSizingAlgorithmStrategy::MaxContentContributionForGridItem(
    RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept
  {
    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Columns);
    if (Direction() == gridItemInlineDirection)
    {
      if (IsComputingInlineSizeContainment())
      {
        return {};
      }
      // FIXME: It's unclear if we should return the intrinsic width or the preferred width.
      // See http://lists.w3.org/Archives/Public/www-style/2013Jan/0245.html
      if (gridItem.ShouldInvalidatePreferredWidths())
      {
        gridItem.SetNeedsPreferredWidthsUpdate();
      }
      return gridItem.MaxPreferredLogicalWidth()
             + GridLayoutFunctions::MarginLogicalSizeForGridItem(*GetRenderGrid(), gridItemInlineDirection,
                                                                 gridItem)
             + _algorithm.BaselineOffsetForGridItem(gridItem, Direction());
    }

    if (UpdateOverridingContainingBlockContentSizeForGridItem(gridItem, gridItemInlineDirection))
    {
      if (auto &intrinsicLogicalHeightsForRowSizingFirstPass =
            GetRenderGrid()->IntrinsicLogicalHeightsForRowSizingFirstPass())
      {
        intrinsicLogicalHeightsForRowSizingFirstPass->InvalidateSizeForItem(gridItem);
      }
      gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }
    return LogicalHeightForGridItem(gridItem, gridLayoutState);
  }

  LayoutUnit GridTrackSizingAlgorithmStrategy::MinContributionForGridItem(
    RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept
  {
    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Columns);
    bool isRowAxis = Direction() == gridItemInlineDirection;
    if (isRowAxis && IsComputingInlineSizeContainment())
    {
      return {};
    }

    auto &gridItemSize = isRowAxis ? gridItem.Style().LogicalWidth() : gridItem.Style().LogicalHeight();
    if (!gridItemSize.IsAuto() && !gridItemSize.IsPercentOrCalculated())
    {
      return MinContentContributionForGridItem(gridItem, gridLayoutState);
    }

    auto &gridItemMinSize =
      isRowAxis ? gridItem.Style().LogicalMinWidth() : gridItem.Style().LogicalMinHeight();

    auto computedOverflowIsNotScrollable = [&gridItem, isRowAxis]()
    {
      auto overflow =
        isRowAxis ? gridItem.EffectiveOverflowInlineDirection() : gridItem.EffectiveOverflowBlockDirection();
      return overflow == Overflow::Visible || overflow == Overflow::Clip;
    };

    LayoutUnit baselineShim = _algorithm.BaselineOffsetForGridItem(gridItem, Direction());

    if (gridItemMinSize.IsAuto())
    {
      if (!computedOverflowIsNotScrollable())
      {
        return {};
      }

      auto minSize = MinContentContributionForGridItem(gridItem, gridLayoutState);
      const GridSpan &span = _algorithm._renderGrid->GridSpanForGridItem(gridItem, Direction());

      LayoutUnit maxBreadth;
      const auto &allTracks = _algorithm.GetTracks(Direction());
      bool allFixed = true;
      for (auto trackPosition : span)
      {
        const auto &trackSize = allTracks[trackPosition]->CachedTrackSize();
        if (trackSize.MaxTrackBreadth().IsFlex() && span.IntegerSpan() > 1)
        {
          return {};
        }
        if (!trackSize.HasFixedMaxTrackBreadth())
        {
          allFixed = false;
        }
        else if (allFixed)
        {
          maxBreadth += EvaluateTrackLength(trackSize.MaxTrackBreadth().Length(), AvailableSpace());
        }
      }
      if (!allFixed)
      {
        return minSize;
      }
      if (minSize > maxBreadth)
      {
        auto marginAndBorderAndPadding =
          GridLayoutFunctions::MarginLogicalSizeForGridItem(*GetRenderGrid(), Direction(), gridItem);
        marginAndBorderAndPadding +=
          isRowAxis ? gridItem.BorderAndPaddingLogicalWidth() : gridItem.BorderAndPaddingLogicalHeight();
        minSize = std::max(maxBreadth, marginAndBorderAndPadding + baselineShim);
      }
      return minSize;
    }

    Maybe<LayoutUnit> gridAreaSize = _algorithm.GridAreaBreadthForGridItem(gridItem, gridItemInlineDirection);
    return MinLogicalSizeForGridItem(gridItem, gridItemMinSize, gridAreaSize) + baselineShim;
  }

  bool GridTrackSizingAlgorithm::CanParticipateInBaselineAlignment(
    const RenderBox &gridItem, GridTrackSizingDirection alignmentContextType) const noexcept
  {
    krys_debug_assert(alignmentContextType == GridTrackSizingDirection::Rows
                        ? _baselineAlignmentItemsForRows.contains(&gridItem)
                        : _baselineAlignmentItemsForColumns.contains(&gridItem));

    // Baseline cyclic dependencies only happen with synthesized
    // baselines. These cases include orthogonal or empty grid items
    // and replaced elements.
    bool isGridItemBlockDirectionParallelToAlignmentAxis =
      alignmentContextType == GridTrackSizingDirection::Rows
        ? !GridLayoutFunctions::IsOrthogonalGridItem(*_renderGrid, gridItem)
        : GridLayoutFunctions::IsOrthogonalGridItem(*_renderGrid, gridItem);
    if (isGridItemBlockDirectionParallelToAlignmentAxis && gridItem.FirstLineBaseline())
    {
      return true;
    }

    // FIXME: We don't currently allow items within subgrids that need to
    // synthesize a baseline, since we need a layout to have been completed
    // and performPreLayoutForGridItems on the outer grid doesn't layout subgrid
    // items.
    if (gridItem.Parent() != _renderGrid)
    {
      return false;
    }

    // Baseline cyclic dependencies only happen in grid areas with
    // intrinsically-sized tracks.
    if (!IsIntrinsicSizedGridArea(gridItem, alignmentContextType))
    {
      return true;
    }

    return isGridItemBlockDirectionParallelToAlignmentAxis ? !gridItem.HasRelativeLogicalHeight()
                                                           : !gridItem.HasRelativeLogicalWidth();
  }

  bool GridTrackSizingAlgorithm::ParticipateInBaselineAlignment(
    const RenderBox &gridItem, GridTrackSizingDirection alignmentContextType) const noexcept
  {
    auto &cache = alignmentContextType == GridTrackSizingDirection::Rows ? _baselineAlignmentItemsForRows
                                                                         : _baselineAlignmentItemsForColumns;
    auto entry = cache.find(&gridItem);
    return entry != cache.end() && entry->second;
  }

  void GridTrackSizingAlgorithm::UpdateBaselineAlignmentContext(
    const RenderBox &gridItem, GridTrackSizingDirection alignmentContextType) noexcept
  {
    krys_debug_assert(WasSetup());
    krys_debug_assert(CanParticipateInBaselineAlignment(gridItem, alignmentContextType));

    auto align = _renderGrid->SelfAlignmentForGridItem(alignmentContextType, gridItem).Position();
    const auto &span = _renderGrid->GridSpanForGridItem(gridItem, alignmentContextType);
    auto alignmentContext = GridLayoutFunctions::AlignmentContextForBaselineAlignment(span, align);
    _baselineAlignment.UpdateBaselineAlignmentContext(align, alignmentContext, gridItem,
                                                      alignmentContextType);
  }

  LayoutUnit GridTrackSizingAlgorithm::BaselineOffsetForGridItem(
    const RenderBox &gridItem, GridTrackSizingDirection alignmentContextType) const noexcept
  {
    // If we haven't yet initialized this axis (which can be the case if we're doing
    // prelayout of a subgrid), then we can't know the baseline offset.
    if (GetTracks(alignmentContextType).empty())
    {
      return LayoutUnit();
    }

    if (!ParticipateInBaselineAlignment(gridItem, alignmentContextType))
    {
      return LayoutUnit();
    }

    krys_debug_assert(alignmentContextType != GridTrackSizingDirection::Rows
                      || !_renderGrid->IsSubgridRows());
    auto align = _renderGrid->SelfAlignmentForGridItem(alignmentContextType, gridItem).Position();
    const auto &span = _renderGrid->GridSpanForGridItem(gridItem, alignmentContextType);
    auto alignmentContext = GridLayoutFunctions::AlignmentContextForBaselineAlignment(span, align);
    return _baselineAlignment.BaselineOffsetForGridItem(align, alignmentContext, gridItem,
                                                        alignmentContextType);
  }

  void GridTrackSizingAlgorithm::ClearBaselineItemsCache() noexcept
  {
    _baselineAlignmentItemsForRows.clear();
    _baselineAlignmentItemsForColumns.clear();
    _rowSubgridsWithBaselineAlignedItems.clear();
  }

  void GridTrackSizingAlgorithm::CacheBaselineAlignedItem(const RenderBox &item,
                                                          GridTrackSizingDirection alignmentContextType,
                                                          bool cachingRowSubgridsForRootGrid) noexcept
  {
    krys_debug_assert(
      Downcast<RenderGrid>(item.Parent())->IsBaselineAlignmentForGridItem(item, alignmentContextType));

    if (GridLayoutFunctions::IsOrthogonalParent(*_renderGrid, *item.Parent()))
    {
      alignmentContextType = alignmentContextType == GridTrackSizingDirection::Rows
                               ? GridTrackSizingDirection::Columns
                               : GridTrackSizingDirection::Rows;
    }

    if (alignmentContextType == GridTrackSizingDirection::Rows)
    {
      _baselineAlignmentItemsForRows.try_emplace(&item, true);
    }
    else
    {
      _baselineAlignmentItemsForColumns.try_emplace(&item, true);
    }

    if (const auto *gridItemParent = DynamicDowncast<RenderGrid>(item.Parent()))
    {
      auto gridItemParentIsSubgridRowsOfRootGrid =
        GridLayoutFunctions::IsOrthogonalGridItem(*_renderGrid, *gridItemParent)
          ? gridItemParent->IsSubgridColumns()
          : gridItemParent->IsSubgridRows();
      if (cachingRowSubgridsForRootGrid && gridItemParentIsSubgridRowsOfRootGrid)
      {
        _rowSubgridsWithBaselineAlignedItems.add(gridItemParent);
      }
    }
  }

  void
    GridTrackSizingAlgorithm::CopyBaselineItemsCache(const GridTrackSizingAlgorithm &source,
                                                     GridTrackSizingDirection alignmentContextType) noexcept
  {
    if (alignmentContextType == GridTrackSizingDirection::Rows)
    {
      _baselineAlignmentItemsForRows = source._baselineAlignmentItemsForRows;
    }
    else
    {
      _baselineAlignmentItemsForColumns = source._baselineAlignmentItemsForColumns;
    }
  }

  bool GridTrackSizingAlgorithmStrategy::UpdateOverridingContainingBlockContentSizeForGridItem(
    RenderBox &gridItem, GridTrackSizingDirection direction, Maybe<LayoutUnit> overrideSize) const noexcept
  {
    if (!overrideSize)
    {
      overrideSize = _algorithm.GridAreaBreadthForGridItem(gridItem, direction);
    }

    if (GetRenderGrid() != gridItem.Parent())
    {
      // If |gridItem| is part of a subgrid, find the nearest ancestor this is directly part of this grid
      // (either by being a child of the grid, or via being subgridded in this dimension.
      auto *grid = Downcast<RenderGrid>(gridItem.Parent());
      auto subgridDirection =
        GridLayoutFunctions::FlowAwareDirectionForGridItem(*GetRenderGrid(), *grid, direction);
      while (grid->Parent() != GetRenderGrid() && !grid->IsSubgridOf(subgridDirection, *GetRenderGrid()))
      {
        grid = Downcast<RenderGrid>(grid->Parent());
        subgridDirection =
          GridLayoutFunctions::FlowAwareDirectionForGridItem(*GetRenderGrid(), *grid, direction);
      }

      if (grid == gridItem.Parent() && grid->IsSubgrid(subgridDirection))
      {
        // If the item is subgridded in this direction (and thus the tracks it covers are tracks
        // owned by this sizing algorithm), then we want to take the breadth of the tracks we occupy,
        // and subtract any space occupied by the subgrid itself (and any ancestor subgrids).
        *overrideSize -=
          GridLayoutFunctions::ExtraMarginForSubgridAncestors(subgridDirection, gridItem).ExtraTotalMargin();
      }
      else
      {
        // Otherwise the tracks that this grid item covers (in this non-subgridded axis) are owned
        // by one of the intermediate RenderGrids (which are subgrids in the other axis), which may
        // be |grid| or a descendent.
        // Set the override size for |grid| (which is part of the outer grid), and force a layout
        // so that it computes the track sizes for the non-subgridded dimension and makes the size
        // of |gridItem| available.
        bool overrideSizeHasChanged = UpdateOverridingContainingBlockContentSizeForGridItem(*grid, direction);
        LayoutGridItemForMinSizeComputation(*grid, overrideSizeHasChanged);
        return overrideSizeHasChanged;
      }
    }

    if (auto overridingContainingBlockContentSizeForGridItem =
          GridLayoutFunctions::OverridingContainingBlockContentSizeForGridItem(gridItem, direction);
        overridingContainingBlockContentSizeForGridItem
        && *overridingContainingBlockContentSizeForGridItem == overrideSize)
    {
      return false;
    }

    SetOverridingContainingBlockContentSizeForGridItem(*GetRenderGrid(), gridItem, direction, overrideSize);
    return true;
  }

  LayoutUnit GridTrackSizingAlgorithmStrategy::MinLogicalSizeForGridItem(
    RenderBox &gridItem, const style::MinimumSize &gridItemMinSize,
    Maybe<LayoutUnit> availableSize) const noexcept
  {
    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Columns);
    bool isRowAxis = Direction() == gridItemInlineDirection;
    if (isRowAxis)
    {
      return IsComputingInlineSizeContainment()
               ? 0_lu
               : gridItem.ComputeLogicalWidthUsing(gridItemMinSize, availableSize.value_or(0_lu),
                                                   *GetRenderGrid())
                   + GridLayoutFunctions::MarginLogicalSizeForGridItem(*GetRenderGrid(),
                                                                       gridItemInlineDirection, gridItem);
    }
    bool overrideSizeHasChanged =
      UpdateOverridingContainingBlockContentSizeForGridItem(gridItem, gridItemInlineDirection, availableSize);
    LayoutGridItemForMinSizeComputation(gridItem, overrideSizeHasChanged);
    auto gridItemBlockDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Rows);
    return gridItem.ComputeLogicalHeightUsing(gridItemMinSize, null).value_or(0_lu)
           + GridLayoutFunctions::MarginLogicalSizeForGridItem(*GetRenderGrid(), gridItemBlockDirection,
                                                               gridItem);
  }

  /// @brief Track sizing for an indefinite size: the intrinsic size computation.
  class IndefiniteSizeStrategy final : public GridTrackSizingAlgorithmStrategy
  {
  public:
    explicit IndefiniteSizeStrategy(GridTrackSizingAlgorithm &algorithm) noexcept
        : GridTrackSizingAlgorithmStrategy(algorithm)
    {
    }

  private:
    void LayoutGridItemForMinSizeComputation(RenderBox &gridItem,
                                             bool overrideSizeHasChanged) const noexcept override;
    void MaximizeTracks(GridTrackSizingAlgorithm::Tracks &tracks,
                        Maybe<LayoutUnit> &freeSpace) noexcept override;
    KRYS_NODISCARD double FindUsedFlexFraction(List<uint32> &flexibleSizedTracksIndex,
                                               GridTrackSizingDirection direction,
                                               Maybe<LayoutUnit> freeSpace,
                                               GridLayoutState &gridLayoutState) const noexcept override;
    bool RecomputeUsedFlexFractionIfNeeded(double &flexFraction,
                                           LayoutUnit &totalGrowth) const noexcept override;
    KRYS_NODISCARD LayoutUnit FreeSpaceForStretchAutoTracksStep() const noexcept override;
    KRYS_NODISCARD bool IsComputingSizeContainment() const noexcept override
    {
      return GetRenderGrid()->ShouldApplySizeContainment();
    }
    KRYS_NODISCARD bool IsComputingInlineSizeContainment() const noexcept override
    {
      return GetRenderGrid()->ShouldApplyInlineSizeContainment();
    }
    KRYS_NODISCARD bool IsComputingSizeOrInlineSizeContainment() const noexcept override
    {
      return GetRenderGrid()->ShouldApplySizeOrInlineSizeContainment();
    }
    void AccumulateFlexFraction(double &flexFraction, GridIterator &iterator,
                                GridTrackSizingDirection outermostDirection,
                                HashSet<const RenderBox *> &itemsSet,
                                GridLayoutState &gridLayoutState) const noexcept;
    void AccumulateFlexFractionMasonry(double &flexFraction, GridIterator &iterator,
                                       GridTrackSizingDirection direction, uint32 flexTrackIndex,
                                       HashSet<const RenderBox *> &itemsSet,
                                       const List<RenderBox *> &indefiniteItems,
                                       GridLayoutState &gridLayoutState) const noexcept;
  };

  void IndefiniteSizeStrategy::LayoutGridItemForMinSizeComputation(RenderBox &gridItem,
                                                                   bool overrideSizeHasChanged) const noexcept
  {
    if (overrideSizeHasChanged && Direction() != GridTrackSizingDirection::Columns)
    {
      gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }
    gridItem.LayoutIfNeeded();
  }

  void IndefiniteSizeStrategy::MaximizeTracks(GridTrackSizingAlgorithm::Tracks &tracks,
                                              Maybe<LayoutUnit> &) noexcept
  {
    for (auto &track : tracks)
    {
      track->SetBaseSize(track->GrowthLimit());
    }
  }

  namespace
  {
    KRYS_NODISCARD inline double NormalizedFlexFraction(const GridTrack &track) noexcept
    {
      auto flexFactor = track.CachedTrackSize().MaxTrackBreadth().GetFlex().Value;
      return track.BaseSize().ToDouble() / std::max<double>(1, flexFactor);
    }
  }

  void IndefiniteSizeStrategy::AccumulateFlexFractionMasonry(double &flexFraction, GridIterator &iterator,
                                                             GridTrackSizingDirection direction,
                                                             uint32 flexTrackIndex,
                                                             HashSet<const RenderBox *> &itemsSet,
                                                             const List<RenderBox *> &indefiniteItems,
                                                             GridLayoutState &gridLayoutState) const noexcept
  {
    // Definite Items
    while (auto *gridItem = iterator.NextGridItem())
    {
      if (!style::GridPositionsResolver::ResolveGridPositionsFromStyle(*_algorithm.GetRenderGrid(), *gridItem,
                                                                       direction)
             .IsIndefinite())
      {
        // Do not include already processed items.
        if (!itemsSet.insert(gridItem).second)
        {
          continue;
        }

        auto span = _algorithm.GetRenderGrid()->GridSpanForGridItem(*gridItem, direction);

        // Removing gutters from the max-content contribution of the item, so they are not taken into account
        // in FindFrUnitSize().
        auto leftOverSpace =
          MaxContentContributionForGridItem(*gridItem, gridLayoutState)
          - GetRenderGrid()->GuttersSize(direction, span.StartLine(), span.IntegerSpan(), AvailableSpace());
        flexFraction = std::max(flexFraction, FindFrUnitSize(span, leftOverSpace));
      }
    }

    auto numTracks = _algorithm.GetTracks(direction).size();

    // Indefinite Items
    for (auto *gridItem : indefiniteItems)
    {
      auto span = _algorithm.GetRenderGrid()->GridSpanForGridItem(*gridItem, direction);
      auto spanLength = span.IntegerSpan();

      auto endLine = flexTrackIndex + spanLength;
      if (endLine > numTracks)
      {
        continue;
      }

      span = GridSpan::TranslatedDefiniteGridSpan(flexTrackIndex, flexTrackIndex + spanLength);

      // Removing gutters from the max-content contribution of the item, so they are not taken into account in
      // FindFrUnitSize().
      auto leftOverSpace =
        MaxContentContributionForGridItem(*gridItem, gridLayoutState)
        - GetRenderGrid()->GuttersSize(direction, span.StartLine(), span.IntegerSpan(), AvailableSpace());
      flexFraction = std::max(flexFraction, FindFrUnitSize(span, leftOverSpace));
    }
  }

  void IndefiniteSizeStrategy::AccumulateFlexFraction(double &flexFraction, GridIterator &iterator,
                                                      GridTrackSizingDirection outermostDirection,
                                                      HashSet<const RenderBox *> &itemsSet,
                                                      GridLayoutState &gridLayoutState) const noexcept
  {
    while (auto *gridItem = iterator.NextGridItem())
    {
      if (auto *inner = DynamicDowncast<RenderGrid>(gridItem);
          inner && inner->IsSubgridInParentDirection(iterator.Direction()))
      {
        const RenderGrid &subgrid = *inner;
        GridSpan span =
          Downcast<RenderGrid>(subgrid.Parent())->GridSpanForGridItem(subgrid, iterator.Direction());
        GridIterator subgridIterator = GridIterator::CreateForSubgrid(*inner, iterator, span);
        AccumulateFlexFraction(flexFraction, subgridIterator, outermostDirection, itemsSet, gridLayoutState);
        continue;
      }
      // Do not include already processed items.
      if (!itemsSet.insert(gridItem).second)
      {
        continue;
      }

      GridSpan span = _algorithm.GetRenderGrid()->GridSpanForGridItem(*gridItem, outermostDirection);

      // Removing gutters from the max-content contribution of the item, so they are not taken into account in
      // FindFrUnitSize().
      LayoutUnit leftOverSpace = MaxContentContributionForGridItem(*gridItem, gridLayoutState)
                                 - GetRenderGrid()->GuttersSize(outermostDirection, span.StartLine(),
                                                                span.IntegerSpan(), AvailableSpace());
      flexFraction = std::max(flexFraction, FindFrUnitSize(span, leftOverSpace));
    }
  }

  double IndefiniteSizeStrategy::FindUsedFlexFraction(List<uint32> &flexibleSizedTracksIndex,
                                                      GridTrackSizingDirection direction, Maybe<LayoutUnit>,
                                                      GridLayoutState &gridLayoutState) const noexcept
  {
    auto &allTracks = _algorithm.GetTracks(direction);

    double flexFraction = 0;
    for (const auto &trackIndex : flexibleSizedTracksIndex)
    {
      // FIXME: we pass TrackSizing to gridTrackSize() because it does not really matter
      // as we know the track is a flex sized track. It'd be nice not to have to do that.
      flexFraction = std::max(flexFraction, NormalizedFlexFraction(*allTracks[trackIndex]));
    }

    const Grid &grid = _algorithm.GetGrid();
    // If we are computing the flex fraction of the grid while under the "Sizing as if empty," phase,
    // then we should use the flex fraction computed up to this point since we do not want to avoid
    // taking into consideration the grid items.
    if (!grid.HasGridItems()
        || (GetRenderGrid()->ShouldCheckExplicitIntrinsicInnerLogicalSize(direction)
            && !_algorithm.GetRenderGrid()->ExplicitIntrinsicInnerLogicalSize(direction)))
    {
      return flexFraction;
    }

    HashSet<const RenderBox *> itemsSet;
    if (!_algorithm.GetRenderGrid()->IsMasonry())
    {
      for (const auto &trackIndex : flexibleSizedTracksIndex)
      {
        GridIterator iterator(grid, direction, trackIndex);
        AccumulateFlexFraction(flexFraction, iterator, direction, itemsSet, gridLayoutState);
      }
    }
    else
    {
      List<RenderBox *> indefiniteItems;
      for (size_t trackIndex = 0; trackIndex < _algorithm.GetTracks(direction).size(); trackIndex++)
      {
        GridIterator iterator(grid, direction, static_cast<uint32>(trackIndex));
        while (auto *gridItem = iterator.NextGridItem())
        {
          if (style::GridPositionsResolver::ResolveGridPositionsFromStyle(*_algorithm.GetRenderGrid(),
                                                                          *gridItem, direction)
                .IsIndefinite())
          {
            indefiniteItems.push_back(gridItem);
          }
        }
      }

      for (const auto &trackIndex : flexibleSizedTracksIndex)
      {
        GridIterator iterator(grid, direction, trackIndex);
        AccumulateFlexFractionMasonry(flexFraction, iterator, direction, trackIndex, itemsSet,
                                      indefiniteItems, gridLayoutState);
      }
    }

    return flexFraction;
  }

  bool IndefiniteSizeStrategy::RecomputeUsedFlexFractionIfNeeded(double &flexFraction,
                                                                 LayoutUnit &totalGrowth) const noexcept
  {
    if (Direction() == GridTrackSizingDirection::Columns)
    {
      return false;
    }

    const RenderGrid *renderGrid = GetRenderGrid();

    auto minSize = renderGrid->ComputeContentLogicalHeight(renderGrid->Style().LogicalMinHeight(), null);
    auto maxSize = renderGrid->ComputeContentLogicalHeight(renderGrid->Style().LogicalMaxHeight(), null);

    // Redo the flex fraction computation using min|max-height as definite available space in case
    // the total height is smaller than min-height or larger than max-height.
    LayoutUnit rowsSize = totalGrowth + ComputeTrackBasedSize();
    bool checkMinSize = minSize && rowsSize < minSize.value();
    bool checkMaxSize = maxSize && rowsSize > maxSize.value();
    if (!checkMinSize && !checkMaxSize)
    {
      return false;
    }

    LayoutUnit freeSpace = checkMaxSize ? maxSize.value() : -1_lu;
    const Grid &grid = _algorithm.GetGrid();
    freeSpace = std::max(freeSpace, minSize.value_or(0_lu))
                - renderGrid->GuttersSize(GridTrackSizingDirection::Rows, 0,
                                          grid.NumTracks(GridTrackSizingDirection::Rows), AvailableSpace());

    size_t numberOfTracks = _algorithm.GetTracks(GridTrackSizingDirection::Rows).size();
    flexFraction =
      FindFrUnitSize(GridSpan::TranslatedDefiniteGridSpan(0, static_cast<uint32>(numberOfTracks)), freeSpace);
    return true;
  }

  LayoutUnit IndefiniteSizeStrategy::FreeSpaceForStretchAutoTracksStep() const noexcept
  {
    krys_debug_assert(!_algorithm.FreeSpace(Direction()));
    if (Direction() == GridTrackSizingDirection::Columns)
    {
      return 0_lu;
    }

    auto minSize =
      GetRenderGrid()->ComputeContentLogicalHeight(GetRenderGrid()->Style().LogicalMinHeight(), null);
    if (!minSize)
    {
      return 0_lu;
    }
    return minSize.value() - ComputeTrackBasedSize();
  }

  /// @brief Track sizing for a definite size: the layout.
  class DefiniteSizeStrategy final : public GridTrackSizingAlgorithmStrategy
  {
  public:
    explicit DefiniteSizeStrategy(GridTrackSizingAlgorithm &algorithm) noexcept
        : GridTrackSizingAlgorithmStrategy(algorithm)
    {
    }

  private:
    void LayoutGridItemForMinSizeComputation(RenderBox &gridItem,
                                             bool overrideSizeHasChanged) const noexcept override;
    void MaximizeTracks(GridTrackSizingAlgorithm::Tracks &tracks,
                        Maybe<LayoutUnit> &freeSpace) noexcept override;
    KRYS_NODISCARD double FindUsedFlexFraction(List<uint32> &flexibleSizedTracksIndex,
                                               GridTrackSizingDirection direction,
                                               Maybe<LayoutUnit> freeSpace,
                                               GridLayoutState &gridLayoutState) const noexcept override;
    bool RecomputeUsedFlexFractionIfNeeded(double &flexFraction,
                                           LayoutUnit &totalGrowth) const noexcept override;
    KRYS_NODISCARD LayoutUnit FreeSpaceForStretchAutoTracksStep() const noexcept override;
    KRYS_NODISCARD LayoutUnit MinContentContributionForGridItem(
      RenderBox &gridItem, GridLayoutState &gridLayoutState) const noexcept override;
    KRYS_NODISCARD LayoutUnit
      MinLogicalSizeForGridItem(RenderBox &gridItem, const style::MinimumSize &gridItemMinSize,
                                Maybe<LayoutUnit> availableSize) const noexcept override;
    KRYS_NODISCARD bool IsComputingSizeContainment() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool IsComputingInlineSizeContainment() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool IsComputingSizeOrInlineSizeContainment() const noexcept override
    {
      return false;
    }
  };

  LayoutUnit DefiniteSizeStrategy::MinLogicalSizeForGridItem(RenderBox &gridItem,
                                                             const style::MinimumSize &gridItemMinSize,
                                                             Maybe<LayoutUnit> availableSize) const noexcept
  {
    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Columns);
    auto flowAwareDirection =
      GridLayoutFunctions::FlowAwareDirectionForGridItem(*GetRenderGrid(), gridItem, Direction());
    if (HasRelativeMarginOrPaddingForGridItem(gridItem, flowAwareDirection)
        || (Direction() != gridItemInlineDirection
            && GridLayoutFunctions::HasRelativeOrIntrinsicSizeForGridItem(gridItem, flowAwareDirection)))
    {
      auto indefiniteSize = Direction() == gridItemInlineDirection ? Maybe<LayoutUnit>(0_lu) : null;
      SetOverridingContainingBlockContentSizeForGridItem(*GetRenderGrid(), gridItem, Direction(),
                                                         indefiniteSize);
    }
    return GridTrackSizingAlgorithmStrategy::MinLogicalSizeForGridItem(gridItem, gridItemMinSize,
                                                                       availableSize);
  }

  void DefiniteSizeStrategy::MaximizeTracks(GridTrackSizingAlgorithm::Tracks &tracks,
                                            Maybe<LayoutUnit> &freeSpace) noexcept
  {
    List<GridTrack *> tracksForDistribution;
    tracksForDistribution.reserve(tracks.size());
    for (auto &track : tracks)
    {
      track->SetPlannedSize(track->BaseSize());
      tracksForDistribution.push_back(track.get());
    }

    krys_debug_assert(freeSpace);
    DistributeSpaceToTracks(tracksForDistribution, freeSpace.value());

    for (auto *track : tracksForDistribution)
    {
      track->SetBaseSize(track->PlannedSize());
    }
  }

  void DefiniteSizeStrategy::LayoutGridItemForMinSizeComputation(RenderBox &gridItem,
                                                                 bool overrideSizeHasChanged) const noexcept
  {
    if (overrideSizeHasChanged)
    {
      gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }
    gridItem.LayoutIfNeeded();
  }

  double DefiniteSizeStrategy::FindUsedFlexFraction(List<uint32> &, GridTrackSizingDirection direction,
                                                    Maybe<LayoutUnit> freeSpace,
                                                    GridLayoutState &) const noexcept
  {
    GridSpan allTracksSpan =
      GridSpan::TranslatedDefiniteGridSpan(0, static_cast<uint32>(_algorithm.GetTracks(direction).size()));
    krys_debug_assert(freeSpace);
    return FindFrUnitSize(allTracksSpan, freeSpace.value());
  }

  LayoutUnit DefiniteSizeStrategy::FreeSpaceForStretchAutoTracksStep() const noexcept
  {
    return _algorithm.FreeSpace(Direction()).value();
  }

  LayoutUnit
    DefiniteSizeStrategy::MinContentContributionForGridItem(RenderBox &gridItem,
                                                            GridLayoutState &gridLayoutState) const noexcept
  {
    auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
      *GetRenderGrid(), gridItem, GridTrackSizingDirection::Columns);

    auto shouldClearOverridingContainingBlockContentSize = [&]
    {
      auto direction = Direction();

      if (!GridLayoutFunctions::IsOrthogonalGridItem(*GetRenderGrid(), gridItem)
          && direction == GridTrackSizingDirection::Columns
          && _algorithm.IsIntrinsicSizedGridArea(gridItem, GridTrackSizingDirection::Columns)
          && (gridItem.Style().LogicalWidth().IsPercentOrCalculated()
              || HasRelativeMarginOrPaddingForGridItem(gridItem, direction)))
      {
        return true;
      }
      return direction == gridItemInlineDirection && gridItem.NeedsLayout()
             && ShouldClearOverridingContainingBlockContentSizeForGridItem(gridItem,
                                                                           GridTrackSizingDirection::Columns);
    }();

    if (shouldClearOverridingContainingBlockContentSize)
    {
      SetOverridingContainingBlockContentSizeForGridItem(*GetRenderGrid(), gridItem, gridItemInlineDirection,
                                                         LayoutUnit());
    }
    return GridTrackSizingAlgorithmStrategy::MinContentContributionForGridItem(gridItem, gridLayoutState);
  }

  bool DefiniteSizeStrategy::RecomputeUsedFlexFractionIfNeeded(double &, LayoutUnit &) const noexcept
  {
    return false;
  }

  // GridTrackSizingAlgorithm steps.

  void GridTrackSizingAlgorithm::InitializeTrackSizes() noexcept
  {
    krys_debug_assert(_contentSizedTracksIndex.empty());
    krys_debug_assert(_flexibleSizedTracksIndex.empty());
    krys_debug_assert(_autoSizedTracksForStretchIndex.empty());
    krys_debug_assert(!_hasPercentSizedRowsIndefiniteHeight);
    krys_debug_assert(!_hasFlexibleMaxTrackBreadth);

    auto &allTracks = GetTracks(_direction);
    const bool indefiniteHeight =
      _direction == GridTrackSizingDirection::Rows && !_renderGrid->HasDefiniteLogicalHeight();
    LayoutUnit maxSize = std::max(0_lu, AvailableSpace().value_or(0_lu));
    // 1. Initialize per Grid track variables.
    for (uint32 i = 0; i < allTracks.size(); ++i)
    {
      auto &track = *allTracks[i];
      const auto trackSize = CalculateGridTrackSize(_direction, i);
      track.SetCachedTrackSize(trackSize);
      track.SetBaseSize(InitialBaseSize(trackSize));
      track.SetGrowthLimit(InitialGrowthLimit(trackSize, track.BaseSize()));
      track.SetInfinitelyGrowable(false);

      if (trackSize.IsFitContent())
      {
        track.SetGrowthLimitCap(EvaluateTrackLength(trackSize.FitContentTrackLength(), maxSize));
      }
      if (trackSize.IsContentSized())
      {
        _contentSizedTracksIndex.push_back(i);
      }
      if (trackSize.MaxTrackBreadth().IsFlex())
      {
        _flexibleSizedTracksIndex.push_back(i);
      }
      if (trackSize.HasAutoMaxTrackBreadth() && !trackSize.IsFitContent())
      {
        _autoSizedTracksForStretchIndex.push_back(i);
      }

      if (indefiniteHeight)
      {
        auto &rawTrackSize = RawGridTrackSize(_direction, i);
        // Set the flag for repeating the track sizing algorithm. For flexible tracks, as per spec
        // https://drafts.csswg.org/css-grid/#algo-flex-tracks, in clause "if the free space is an indefinite
        // length:", it states that "If using this flex fraction would cause the grid to be smaller than the
        // grid container's min-width/height (or larger than the grid container's max-width/height), then redo
        // this step".
        if (!_hasFlexibleMaxTrackBreadth && rawTrackSize.MaxTrackBreadth().IsFlex())
        {
          _hasFlexibleMaxTrackBreadth = true;
        }
        if (!_hasPercentSizedRowsIndefiniteHeight
            && (rawTrackSize.MinTrackBreadth().IsPercentOrCalculated()
                || rawTrackSize.MaxTrackBreadth().IsPercentOrCalculated()))
        {
          _hasPercentSizedRowsIndefiniteHeight = true;
        }
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD LayoutUnit MarginAndBorderAndPaddingForEdge(const RenderGrid &grid,
                                                               GridTrackSizingDirection direction,
                                                               bool startEdge) noexcept
    {
      if (direction == GridTrackSizingDirection::Columns)
      {
        return startEdge ? grid.MarginAndBorderAndPaddingStart() : grid.MarginAndBorderAndPaddingEnd();
      }
      return startEdge ? grid.MarginAndBorderAndPaddingBefore() : grid.MarginAndBorderAndPaddingAfter();
    }

    // https://drafts.csswg.org/css-grid-2/#subgrid-edge-placeholders
    // FIXME: This is a simplification of the specified behavior, where we add the hypothetical
    // items directly to the edge tracks as if they had a span of 1. This matches the current Gecko
    // behavior.
    KRYS_NODISCARD LayoutUnit ComputeSubgridMarginBorderPadding(const RenderGrid *outermost,
                                                                GridTrackSizingDirection outermostDirection,
                                                                GridTrack &track, uint32 trackIndex,
                                                                GridSpan &span, RenderGrid *subgrid) noexcept
    {
      // Convert the direction into the coordinate space of subgrid (which may not be a direct child
      // of the outermost grid for which we're running the track sizing algorithm).
      auto direction =
        GridLayoutFunctions::FlowAwareDirectionForGridItem(*outermost, *subgrid, outermostDirection);
      bool reversed =
        GridLayoutFunctions::IsSubgridReversedDirection(*outermost, outermostDirection, *subgrid);

      LayoutUnit subgridMbp;
      if (trackIndex == span.StartLine() && track.CachedTrackSize().HasIntrinsicMinTrackBreadth())
      {
        // If the subgrid has a reversed flow direction relative to the outermost grid, then
        // we want the MBP from the end edge in its local coordinate space.
        subgridMbp = MarginAndBorderAndPaddingForEdge(*subgrid, direction, !reversed);
      }
      if (trackIndex == span.EndLine() - 1 && track.CachedTrackSize().HasIntrinsicMinTrackBreadth())
      {
        subgridMbp += MarginAndBorderAndPaddingForEdge(*subgrid, direction, reversed);
      }
      return subgridMbp;
    }

    KRYS_NODISCARD Maybe<LayoutUnit>
      ExtraMarginFromSubgridAncestorGutters(const RenderBox &gridItem, GridSpan itemSpan, uint32 trackIndex,
                                            GridTrackSizingDirection direction) noexcept
    {
      if (itemSpan.StartLine() != trackIndex && itemSpan.EndLine() - 1 != trackIndex)
      {
        return null;
      }

      auto gutterTotal = 0_lu;

      for (auto &currentAncestorSubgrid : AncestorSubgridsOfGridItem(gridItem, direction))
      {
        auto gridItemSpanInAncestor = currentAncestorSubgrid.GridSpanForGridItem(gridItem, direction);
        auto numTracksForCurrentAncestor = currentAncestorSubgrid.NumTracks(direction);

        const auto *currentAncestorSubgridParent =
          DynamicDowncast<RenderGrid>(currentAncestorSubgrid.Parent());
        krys_debug_assert(currentAncestorSubgridParent);
        if (!currentAncestorSubgridParent)
        {
          return null;
        }

        if (gridItemSpanInAncestor.StartLine())
        {
          gutterTotal +=
            (currentAncestorSubgrid.GridGap(direction) - currentAncestorSubgridParent->GridGap(direction))
            / 2;
        }
        if (gridItemSpanInAncestor.EndLine() != numTracksForCurrentAncestor)
        {
          gutterTotal +=
            (currentAncestorSubgrid.GridGap(direction) - currentAncestorSubgridParent->GridGap(direction))
            / 2;
        }
        direction = GridLayoutFunctions::FlowAwareDirectionForParent(
          currentAncestorSubgrid, *currentAncestorSubgridParent, direction);
      }
      return gutterTotal;
    }
  }

  bool GridTrackSizingAlgorithm::ShouldExcludeGridItemForMasonryTrackSizing(const RenderBox &gridItem,
                                                                            uint32 trackIndex,
                                                                            GridSpan itemSpan) const noexcept
  {
    bool shouldExcludeGridItemForMasonryTrackSizing = true;

    // Items specifically placed in this track.
    if (_renderGrid->GridSpanForGridItem(gridItem, _direction).StartLine() == trackIndex)
    {
      shouldExcludeGridItemForMasonryTrackSizing = false;
    }

    // Items that have an indefinite placement in the grid axis.
    if (style::GridPositionsResolver::ResolveGridPositionsFromStyle(*_renderGrid, gridItem, _direction)
          .IsIndefinite())
    {
      shouldExcludeGridItemForMasonryTrackSizing = false;
    }

    // If the item is going past the end of track do not consider it for inclusion.
    if (itemSpan.IntegerSpan() + itemSpan.StartLine() > GetTracks(_direction).size())
    {
      shouldExcludeGridItemForMasonryTrackSizing = true;
    }

    return shouldExcludeGridItemForMasonryTrackSizing;
  }

  void GridTrackSizingAlgorithm::AccumulateIntrinsicSizesForTrack(
    GridTrack &track, uint32 trackIndex, GridIterator &iterator,
    List<GridItemWithSpan> &itemsSortedByIncreasingSpan, List<GridItemWithSpan> &itemsCrossingFlexibleTracks,
    HashSet<const RenderBox *> &itemsSet, LayoutUnit currentAccumulatedMbp,
    GridLayoutState &gridLayoutState) noexcept
  {
    while (auto *gridItem = iterator.NextGridItem())
    {
      bool isNewEntry = itemsSet.insert(gridItem).second;
      GridSpan span = _renderGrid->GridSpanForGridItem(*gridItem, _direction);

      if (auto *inner = DynamicDowncast<RenderGrid>(*gridItem);
          inner && inner->IsSubgridInParentDirection(iterator.Direction()))
      {
        // Contribute the mbp of wrapper to the first and last tracks that we span.
        GridSpan subgridSpan =
          Downcast<RenderGrid>(inner->Parent())->GridSpanForGridItem(*inner, iterator.Direction());
        auto accumulatedMbpWithSubgrid =
          currentAccumulatedMbp
          + ComputeSubgridMarginBorderPadding(_renderGrid, _direction, track, trackIndex, span, inner);
        track.SetBaseSize(std::max(track.BaseSize(), accumulatedMbpWithSubgrid
                                                       + ExtraMarginFromSubgridAncestorGutters(
                                                           *gridItem, span, trackIndex, iterator.Direction())
                                                           .value_or(0_lu)));

        GridIterator subgridIterator = GridIterator::CreateForSubgrid(*inner, iterator, subgridSpan);

        AccumulateIntrinsicSizesForTrack(track, trackIndex, subgridIterator, itemsSortedByIncreasingSpan,
                                         itemsCrossingFlexibleTracks, itemsSet, accumulatedMbpWithSubgrid,
                                         gridLayoutState);
        continue;
      }

      if (!isNewEntry)
      {
        continue;
      }

      if (SpanningItemCrossesFlexibleSizedTracks(span))
      {
        itemsCrossingFlexibleTracks.emplace_back(*gridItem, span);
      }
      else if (span.IntegerSpan() == 1)
      {
        SizeTrackToFitNonSpanningItem(span, *gridItem, track, gridLayoutState);
      }
      else
      {
        itemsSortedByIncreasingSpan.emplace_back(*gridItem, span);
      }
    }
  }

  void GridTrackSizingAlgorithm::ComputeDefiniteAndIndefiniteItemsForMasonry(
    std::map<SpanLength, MasonryMinMaxTrackSize> &indefiniteSpanSizes,
    std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> &definiteItemSizes,
    List<MasonryMinMaxTrackSizeWithGridSpan> &definiteItemSizesSpanFlexTrack,
    GridLayoutState &gridLayoutState) noexcept
  {
    auto &allTracks = GetTracks(_direction);

    auto populateDefiniteItems =
      [&](uint32 trackIndex, GridSpan &gridSpan, uint32 spanLength, RenderBox &gridItem)
    {
      if (gridSpan.StartLine() != trackIndex)
      {
        return;
      }

      auto minContentContributionForGridItem =
        _strategy->MinContentContributionForGridItem(gridItem, gridLayoutState);
      auto maxContentContributionForGridItem =
        _strategy->MaxContentContributionForGridItem(gridItem, gridLayoutState);
      auto minContributionForGridItem = _strategy->MinContributionForGridItem(gridItem, gridLayoutState);

      bool spansFlexTracks = SpanningItemCrossesFlexibleSizedTracks(gridSpan);

      if (spanLength == 1 && !spansFlexTracks)
      {
        SizeTrackToFitNonSpanningItem(gridSpan, gridItem, *allTracks[trackIndex], gridLayoutState);
      }
      else
      {
        auto minMaxTrackSizeWithGridSpan = MasonryMinMaxTrackSizeWithGridSpan {
          MasonryMinMaxTrackSize {minContentContributionForGridItem, maxContentContributionForGridItem,
                                  minContributionForGridItem},
          gridSpan};

        if (spansFlexTracks)
        {
          definiteItemSizesSpanFlexTrack.push_back(minMaxTrackSizeWithGridSpan);
        }
        else
        {
          definiteItemSizes[spanLength].push_back(minMaxTrackSizeWithGridSpan);
        }
      }
    };

    auto populateIndefiniteItems = [&](RenderBox &gridItem, uint32 spanLength)
    {
      auto minContentContributionForGridItem =
        _strategy->MinContentContributionForGridItem(gridItem, gridLayoutState);
      auto maxContentContributionForGridItem =
        _strategy->MaxContentContributionForGridItem(gridItem, gridLayoutState);
      auto minContributionForGridItem = _strategy->MinContributionForGridItem(gridItem, gridLayoutState);

      auto &trackSize = indefiniteSpanSizes[spanLength];

      trackSize.MinContentSize = std::max(trackSize.MinContentSize, minContentContributionForGridItem);
      trackSize.MaxContentSize = std::max(trackSize.MaxContentSize, maxContentContributionForGridItem);
      trackSize.MinSize = std::max(trackSize.MinSize, minContributionForGridItem);
    };

    auto trackLength = allTracks.size();
    for (size_t trackIndex = 0; trackIndex < trackLength; trackIndex++)
    {
      GridIterator iterator(_grid, _direction, static_cast<uint32>(trackIndex));

      while (auto *gridItem = iterator.NextGridItem())
      {
        auto gridSpan = _renderGrid->GridSpanForGridItem(*gridItem, _direction);
        auto spanLength = gridSpan.IntegerSpan();

        if (!style::GridPositionsResolver::ResolveGridPositionsFromStyle(*_renderGrid, *gridItem, _direction)
               .IsIndefinite())
        {
          populateDefiniteItems(static_cast<uint32>(trackIndex), gridSpan, spanLength, *gridItem);
          continue;
        }

        auto endLine = trackIndex + spanLength;
        if (endLine > trackLength)
        {
          continue;
        }

        populateIndefiniteItems(*gridItem, spanLength);
      }
    }
  }

  void GridTrackSizingAlgorithm::HandleInfinityGrowthLimit() noexcept
  {
    auto &allTracks = GetTracks(_direction);
    for (auto trackIndex : _contentSizedTracksIndex)
    {
      auto &track = *allTracks[trackIndex];
      if (track.GrowthLimit() == infinity)
      {
        track.SetGrowthLimit(track.BaseSize());
      }
    }
  }

  void GridTrackSizingAlgorithm::ResolveIntrinsicTrackSizes(GridLayoutState &gridLayoutState) noexcept
  {
    if (_strategy->IsComputingSizeContainment())
    {
      HandleInfinityGrowthLimit();
      return;
    }

    auto &allTracks = GetTracks(_direction);
    List<GridItemWithSpan> itemsSortedByIncreasingSpan;
    List<GridItemWithSpan> itemsCrossingFlexibleTracks;
    HashSet<const RenderBox *> itemsSet;

    if (_grid.HasGridItems())
    {
      for (auto trackIndex : _contentSizedTracksIndex)
      {
        GridIterator iterator(_grid, _direction, trackIndex);
        auto &track = *allTracks[trackIndex];

        AccumulateIntrinsicSizesForTrack(track, trackIndex, iterator, itemsSortedByIncreasingSpan,
                                         itemsCrossingFlexibleTracks, itemsSet, 0_lu, gridLayoutState);
      }
      std::stable_sort(itemsSortedByIncreasingSpan.begin(), itemsSortedByIncreasingSpan.end());
    }

    auto itemSpan = Span<GridItemWithSpan>(itemsSortedByIncreasingSpan);
    while (!itemSpan.empty())
    {
      auto upperBound = std::upper_bound(itemSpan.begin(), itemSpan.end(), itemSpan[0]);
      auto rangeSize = static_cast<size_t>(upperBound - itemSpan.begin());
      IncreaseSizesToAccommodateSpanningItems<TrackSizeComputationVariant::NotCrossingFlexibleTracks>(
        itemSpan.first(rangeSize), gridLayoutState);
      itemSpan = itemSpan.subspan(rangeSize);
    }
    IncreaseSizesToAccommodateSpanningItems<TrackSizeComputationVariant::CrossingFlexibleTracks>(
      Span<GridItemWithSpan>(itemsCrossingFlexibleTracks), gridLayoutState);
    HandleInfinityGrowthLimit();
  }

  void GridTrackSizingAlgorithm::ResolveIntrinsicTrackSizesMasonry(GridLayoutState &gridLayoutState) noexcept
  {
    if (_strategy->IsComputingSizeContainment() || !_grid.HasGridItems())
    {
      HandleInfinityGrowthLimit();
      return;
    }
    std::map<SpanLength, MasonryMinMaxTrackSize> indefiniteSpanSizes;
    std::map<SpanLength, List<MasonryMinMaxTrackSizeWithGridSpan>> definiteItemSizes;
    List<MasonryMinMaxTrackSizeWithGridSpan> definiteItemSizesSpanFlexTrack;

    ComputeDefiniteAndIndefiniteItemsForMasonry(indefiniteSpanSizes, definiteItemSizes,
                                                definiteItemSizesSpanFlexTrack, gridLayoutState);

    // Update intrinsic tracks with single span items that do not cross flex tracks.
    auto &allTracks = GetTracks(_direction);

    if (auto item = indefiniteSpanSizes.find(1); item != indefiniteSpanSizes.end())
    {
      auto &singleTrackSpanSize = item->second;
      for (auto trackIndex : _contentSizedTracksIndex)
      {
        auto &track = *allTracks[trackIndex];

        auto itemSpan = GridSpan::TranslatedDefiniteGridSpan(trackIndex, trackIndex + 1);
        if (SpanningItemCrossesFlexibleSizedTracks(itemSpan))
        {
          continue;
        }

        SizeTrackToFitSingleSpanMasonryGroup(itemSpan, singleTrackSpanSize, track);
      }
    }

    ConvertIndefiniteItemsToDefiniteMasonry(indefiniteSpanSizes, definiteItemSizes,
                                            definiteItemSizesSpanFlexTrack);

    IncreaseSizesToAccommodateSpanningItemsMasonry<TrackSizeComputationVariant::NotCrossingFlexibleTracks>(
      definiteItemSizes);

    IncreaseSizesToAccommodateSpanningItemsMasonryWithFlex<
      TrackSizeComputationVariant::CrossingFlexibleTracks>(definiteItemSizesSpanFlexTrack);

    HandleInfinityGrowthLimit();
  }

  void GridTrackSizingAlgorithm::StretchFlexibleTracks(Maybe<LayoutUnit> freeSpace,
                                                       GridLayoutState &gridLayoutState) noexcept
  {
    if (_flexibleSizedTracksIndex.empty())
    {
      return;
    }

    double flexFraction =
      _strategy->FindUsedFlexFraction(_flexibleSizedTracksIndex, _direction, freeSpace, gridLayoutState);

    LayoutUnit totalGrowth;
    List<LayoutUnit> increments(_flexibleSizedTracksIndex.size());
    ComputeFlexSizedTracksGrowth(flexFraction, increments, totalGrowth);

    if (_strategy->RecomputeUsedFlexFractionIfNeeded(flexFraction, totalGrowth))
    {
      totalGrowth = 0_lu;
      ComputeFlexSizedTracksGrowth(flexFraction, increments, totalGrowth);
    }

    size_t i = 0;
    auto &allTracks = GetTracks(_direction);
    for (auto trackIndex : _flexibleSizedTracksIndex)
    {
      auto &track = *allTracks[trackIndex];
      if (LayoutUnit increment = increments[i++])
      {
        track.SetBaseSize(track.BaseSize() + increment);
      }
    }
    if (FreeSpace(_direction))
    {
      SetFreeSpace(_direction, FreeSpace(_direction).value() - totalGrowth);
    }
    _maxContentSize += totalGrowth;
  }

  void GridTrackSizingAlgorithm::StretchAutoTracks() noexcept
  {
    auto currentFreeSpace = _strategy->FreeSpaceForStretchAutoTracksStep();
    if (_autoSizedTracksForStretchIndex.empty() || currentFreeSpace <= 0
        || (_renderGrid->ContentAlignment(_direction).Distribution() != ContentDistribution::Stretch))
    {
      return;
    }

    auto &allTracks = GetTracks(_direction);
    auto numberOfAutoSizedTracks = static_cast<int32>(_autoSizedTracksForStretchIndex.size());
    LayoutUnit sizeToIncrease = currentFreeSpace / numberOfAutoSizedTracks;
    for (const auto &trackIndex : _autoSizedTracksForStretchIndex)
    {
      auto &track = *allTracks[trackIndex];
      track.SetBaseSize(track.BaseSize() + sizeToIncrease);
    }
    SetFreeSpace(_direction, 0_lu);
  }

  void GridTrackSizingAlgorithm::AdvanceNextState() noexcept
  {
    switch (_sizingState)
    {
      case SizingState::ColumnSizingFirstIteration:
        _sizingState = SizingState::RowSizingFirstIteration;
        return;
      case SizingState::RowSizingFirstIteration:
        _sizingState = _strategy->IsComputingSizeContainment()
                         ? SizingState::RowSizingExtraIterationForSizeContainment
                         : SizingState::ColumnSizingSecondIteration;
        return;
      case SizingState::RowSizingExtraIterationForSizeContainment:
        _sizingState = SizingState::ColumnSizingSecondIteration;
        return;
      case SizingState::ColumnSizingSecondIteration:
        _sizingState = SizingState::RowSizingSecondIteration;
        return;
      case SizingState::RowSizingSecondIteration:
        _sizingState = SizingState::ColumnSizingFirstIteration;
        return;
    }
    krys_debug_assert(false);
    _sizingState = SizingState::ColumnSizingFirstIteration;
  }

  bool GridTrackSizingAlgorithm::IsValidTransition() const noexcept
  {
    switch (_sizingState)
    {
      case SizingState::ColumnSizingFirstIteration:
      case SizingState::ColumnSizingSecondIteration:               return _direction == GridTrackSizingDirection::Columns;
      case SizingState::RowSizingFirstIteration:
      case SizingState::RowSizingExtraIterationForSizeContainment:
      case SizingState::RowSizingSecondIteration:                  return _direction == GridTrackSizingDirection::Rows;
    }
    krys_debug_assert(false);
    return false;
  }

  // GridTrackSizingAlgorithm API.

  void GridTrackSizingAlgorithm::Setup(GridTrackSizingDirection direction, uint32 numTracks,
                                       SizingOperation sizingOperation,
                                       Maybe<LayoutUnit> availableSpace) noexcept
  {
    krys_debug_assert(_needsSetup);
    _direction = direction;
    SetAvailableSpace(direction,
                      availableSpace ? Maybe<LayoutUnit>(std::max(0_lu, *availableSpace)) : availableSpace);

    _sizingOperation = sizingOperation;
    switch (_sizingOperation)
    {
      case SizingOperation::IntrinsicSizeComputation:
        _strategy = CreateUnique<IndefiniteSizeStrategy>(*this);
        break;
      case SizingOperation::TrackSizing: _strategy = CreateUnique<DefiniteSizeStrategy>(*this); break;
    }

    _contentSizedTracksIndex.clear();
    _flexibleSizedTracksIndex.clear();
    _autoSizedTracksForStretchIndex.clear();

    if (availableSpace)
    {
      LayoutUnit guttersSize =
        _renderGrid->GuttersSize(direction, 0, _grid.NumTracks(direction), AvailableSpace(direction));
      SetFreeSpace(direction, *availableSpace - guttersSize);
    }
    else
    {
      SetFreeSpace(direction, null);
    }

    auto &tracks = GetTracks(direction);
    tracks.clear();
    tracks.reserve(numTracks);
    for (uint32 i = 0; i < numTracks; ++i)
    {
      tracks.push_back(CreateUnique<GridTrack>());
    }

    _needsSetup = false;
    _hasPercentSizedRowsIndefiniteHeight = false;
    _hasFlexibleMaxTrackBreadth = false;

    auto resolveAndSetNonAutoRowStartMarginsOnRowSubgrids = [&]
    {
      for (auto *subgrid : _rowSubgridsWithBaselineAlignedItems)
      {
        const auto subgridSpan =
          _renderGrid->GridSpanForGridItem(*subgrid, GridTrackSizingDirection::Columns);
        const auto &subgridStyle = subgrid->Style();
        auto &subgridRowStartMargin = subgridStyle.MarginBefore(_renderGrid->GetWritingMode());
        if (!subgridRowStartMargin.IsAuto())
        {
          auto spanSize =
            ComputeGridSpanSize(GetTracks(GridTrackSizingDirection::Columns), subgridSpan,
                                Maybe<LayoutUnit>(_renderGrid->GridItemOffset(direction)),
                                _renderGrid->GuttersSize(GridTrackSizingDirection::Columns,
                                                         subgridSpan.StartLine(), subgridSpan.IntegerSpan(),
                                                         AvailableSpace(GridTrackSizingDirection::Columns)));
          _renderGrid->SetMarginBeforeForChild(
            const_cast<RenderGrid &>(*subgrid),
            style::EvaluateMinimum<LayoutUnit>(subgridRowStartMargin, spanSize,
                                               subgridStyle.UsedZoomForLength()));
        }
      }
    };
    if (_direction == GridTrackSizingDirection::Rows
        && (_sizingState == SizingState::RowSizingFirstIteration
            || _sizingState == SizingState::RowSizingSecondIteration))
    {
      resolveAndSetNonAutoRowStartMarginsOnRowSubgrids();
    }

    ComputeBaselineAlignmentContext();
  }

  void GridTrackSizingAlgorithm::ComputeBaselineAlignmentContext() noexcept
  {
    auto alignmentContextType = _direction;
    _baselineAlignment.Clear(alignmentContextType);
    _baselineAlignment.SetWritingMode(_renderGrid->Style().GetWritingMode());
    BaselineItemsCache &baselineItemsCache = alignmentContextType == GridTrackSizingDirection::Rows
                                               ? _baselineAlignmentItemsForRows
                                               : _baselineAlignmentItemsForColumns;
    BaselineItemsCache tmpBaselineItemsCache = baselineItemsCache;
    for (auto &[gridItem, participates] : tmpBaselineItemsCache)
    {
      // FIXME (jfernandez): We may have to get rid of the baseline participation
      // flag (hence just using a HashSet) depending on the CSS WG resolution on
      // https://github.com/w3c/csswg-drafts/issues/3046
      if (CanParticipateInBaselineAlignment(*gridItem, alignmentContextType))
      {
        UpdateBaselineAlignmentContext(*gridItem, alignmentContextType);
        baselineItemsCache[gridItem] = true;
      }
      else
      {
        baselineItemsCache[gridItem] = false;
      }
    }
  }

  namespace
  {
    void RemoveSubgridMarginBorderPaddingFromTracks(GridTrackSizingAlgorithm::Tracks &tracks, LayoutUnit mbp,
                                                    bool forwards) noexcept
    {
      int numTracks = static_cast<int>(tracks.size());
      int i = forwards ? 0 : numTracks - 1;
      while (mbp > 0 && (forwards ? i < numTracks : i >= 0))
      {
        LayoutUnit size = tracks[i]->BaseSize();
        if (size > mbp)
        {
          size -= mbp;
          mbp = 0_lu;
        }
        else
        {
          mbp -= size;
          size = 0_lu;
        }
        tracks[i]->SetBaseSize(size);

        forwards ? i++ : i--;
      }
    }
  }

  bool GridTrackSizingAlgorithm::CopyUsedTrackSizesForSubgrid() noexcept
  {
    auto *outer = Downcast<RenderGrid>(_renderGrid->Parent());
    auto &parentAlgo = outer->_trackSizingAlgorithm;
    auto direction = GridLayoutFunctions::FlowAwareDirectionForParent(*_renderGrid, *outer, _direction);
    auto &parentTracks = parentAlgo.GetTracks(direction);

    if (parentTracks.empty())
    {
      return false;
    }

    auto span = outer->GridSpanForGridItem(*_renderGrid, direction);
    auto &allTracks = GetTracks(_direction);
    int numTracks = static_cast<int>(allTracks.size());
    krys_assert(static_cast<int>(parentTracks.size()) - 1
                >= numTracks - 1 + static_cast<int>(span.StartLine()));
    for (int i = 0; i < numTracks; i++)
    {
      *allTracks[i] = *parentTracks[i + span.StartLine()];
    }

    if (GridLayoutFunctions::IsSubgridReversedDirection(*outer, direction, *_renderGrid))
    {
      std::ranges::reverse(allTracks);
    }

    LayoutUnit startMBP = (_direction == GridTrackSizingDirection::Columns)
                            ? _renderGrid->MarginAndBorderAndPaddingStart()
                            : _renderGrid->MarginAndBorderAndPaddingBefore();
    RemoveSubgridMarginBorderPaddingFromTracks(allTracks, startMBP, true);
    LayoutUnit endMBP = (_direction == GridTrackSizingDirection::Columns)
                          ? _renderGrid->MarginAndBorderAndPaddingEnd()
                          : _renderGrid->MarginAndBorderAndPaddingAfter();
    RemoveSubgridMarginBorderPaddingFromTracks(allTracks, endMBP, false);

    LayoutUnit gapDifference =
      (_renderGrid->GridGap(_direction, AvailableSpace(_direction)) - outer->GridGap(direction)) / 2;
    for (int i = 0; i < numTracks; i++)
    {
      LayoutUnit size = allTracks[i]->BaseSize();
      if (i)
      {
        size -= gapDifference;
      }
      if (i != numTracks - 1)
      {
        size -= gapDifference;
      }
      allTracks[i]->SetBaseSize(size);
    }
    return true;
  }

  void GridTrackSizingAlgorithm::Run(GridTrackSizingDirection direction, uint32 numTracks,
                                     SizingOperation sizingOperation, Maybe<LayoutUnit> availableSpace,
                                     GridLayoutState &gridLayoutState) noexcept
  {
    Setup(direction, numTracks, sizingOperation, availableSpace);

    StateMachine stateMachine(*this);

    if (_renderGrid->IsMasonry(_direction))
    {
      return;
    }

    if (_renderGrid->IsSubgrid(_direction) && CopyUsedTrackSizesForSubgrid())
    {
      return;
    }

    // Step 1.
    const Maybe<LayoutUnit> initialFreeSpace = FreeSpace(_direction);
    InitializeTrackSizes();

    // Step 2.
    if (!_contentSizedTracksIndex.empty())
    {
      if (!_renderGrid->IsMasonry())
      {
        ResolveIntrinsicTrackSizes(gridLayoutState);
      }
      else
      {
        ResolveIntrinsicTrackSizesMasonry(gridLayoutState);
      }
    }

    // This is not exactly a step of the track sizing algorithm, but we use the track sizes computed
    // up to this moment (before maximization) to calculate the grid container intrinsic sizes.
    ComputeGridContainerIntrinsicSizes();

    if (FreeSpace(_direction))
    {
      LayoutUnit updatedFreeSpace = FreeSpace(_direction).value() - _minContentSize;
      SetFreeSpace(_direction, updatedFreeSpace);
      if (updatedFreeSpace <= 0)
      {
        return;
      }
    }

    // Step 3.
    _strategy->MaximizeTracks(GetTracks(_direction), _direction == GridTrackSizingDirection::Columns
                                                       ? _freeSpaceColumns
                                                       : _freeSpaceRows);

    // Step 4.
    StretchFlexibleTracks(initialFreeSpace, gridLayoutState);

    // Step 5.
    StretchAutoTracks();
  }

  void GridTrackSizingAlgorithm::Reset() noexcept
  {
    krys_debug_assert(WasSetup());
    _sizingState = SizingState::ColumnSizingFirstIteration;
    _columns.clear();
    _rows.clear();
    _contentSizedTracksIndex.clear();
    _flexibleSizedTracksIndex.clear();
    _autoSizedTracksForStretchIndex.clear();
    SetAvailableSpace(GridTrackSizingDirection::Rows, null);
    SetAvailableSpace(GridTrackSizingDirection::Columns, null);
    _hasPercentSizedRowsIndefiniteHeight = false;
    _hasFlexibleMaxTrackBreadth = false;
  }

#if KRYS_DEBUG
  bool GridTrackSizingAlgorithm::TracksAreWiderThanMinTrackBreadth() const noexcept
  {
    // Subgrids inherit their sizing directly from the parent, so may be unrelated
    // to their initial base size.
    if (_renderGrid->IsSubgrid(_direction))
    {
      return true;
    }

    if (_renderGrid->IsMasonry(_direction))
    {
      return true;
    }

    auto &allTracks = GetTracks(_direction);
    for (size_t i = 0; i < allTracks.size(); ++i)
    {
      const auto &trackSize = allTracks[i]->CachedTrackSize();
      if (InitialBaseSize(trackSize) > allTracks[i]->BaseSize())
      {
        return false;
      }
    }
    return true;
  }
#endif

  GridTrackSizingAlgorithm::StateMachine::StateMachine(GridTrackSizingAlgorithm &algorithm) noexcept
      : _algorithm(algorithm)
  {
    krys_debug_assert(_algorithm.IsValidTransition());
    krys_debug_assert(!_algorithm._needsSetup);
  }

  GridTrackSizingAlgorithm::StateMachine::~StateMachine() noexcept
  {
    _algorithm.AdvanceNextState();
    _algorithm._needsSetup = true;
  }

  bool GridTrackSizingAlgorithm::IsDirectionInMasonryDirection() const noexcept
  {
    return _renderGrid->IsMasonry(_direction);
  }

  bool GridTrackSizingAlgorithm::HasAllLengthRowSizes() const noexcept
  {
    for (uint32 rowIndex = 0; rowIndex < _renderGrid->NumTracks(GridTrackSizingDirection::Rows); ++rowIndex)
    {
      auto &trackSize = RawGridTrackSize(GridTrackSizingDirection::Rows, rowIndex);
      if (!trackSize.IsBreadth() && !trackSize.MinTrackBreadth().IsLength())
      {
        return false;
      }
    }
    return true;
  }
}
