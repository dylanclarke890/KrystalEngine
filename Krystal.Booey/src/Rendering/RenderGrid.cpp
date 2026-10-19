#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Layout/Integration/Grid/LayoutIntegrationGridLayout.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/BaselineAlignment.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/GridLayoutState.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridPositionsResolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  using style::GridTrackSizingDirection;

  RenderGrid::RenderGrid(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(Type::Grid, element, krys::move(style), {}), _grid(*this),
        _trackSizingAlgorithm(this, CurrentGrid()), _masonryLayout(*this)
  {
    krys_debug_assert(IsRenderGrid());
    // All of our children must be block level.
    SetChildrenInline(false);
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderGrid::~RenderGrid() noexcept = default;

  bool RenderGrid::IsExtrinsicallySized() const noexcept
  {
    auto &gridStyle = Style();
    auto allTracksAreExtrinsicallySized = [&]
    {
      for (auto &column : gridStyle.GridTemplateColumns().Sizes)
      {
        if (column.IsContentSized())
        {
          return false;
        }
      }

      for (auto &row : gridStyle.GridTemplateRows().Sizes)
      {
        if (row.IsContentSized())
        {
          return false;
        }
      }
      return true;
    };

    // Since we currently only check if the grid's logical width is auto, it being
    // extrinsically sized in this regard depends on the formatting context it
    // participates in. For now we only check and allow if it participates in block
    // layout since that is simple.
    auto participatesInBlockLayout = [&]
    {
      auto *containingBlock = ContainingBlock();
      return containingBlock && containingBlock->IsBlockContainer() && !containingBlock->ChildrenInline();
    };

    if (!gridStyle.LogicalWidth().IsAuto() || !participatesInBlockLayout()
        || !gridStyle.LogicalHeight().IsFixed() || !allTracksAreExtrinsicallySized()
        || gridStyle.HasAspectRatio() || IsSubgrid() || IsMasonry())
    {
      return false;
    }

    for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
    {
      // FIXME: If we do not need to perform item placement we should be able
      // to check any implicitly created tracks as well.
      if (!IsPlacedWithinExtrinsicallySizedExplicitTracks(gridItem))
      {
        return false;
      }
    }

    return true;
  }

  StyleSelfAlignmentData RenderGrid::SelfAlignmentForGridItem(GridTrackSizingDirection alignmentContextType,
                                                              const RenderBox &gridItem,
                                                              const RenderStyle *gridStyle) const noexcept
  {
    return alignmentContextType == GridTrackSizingDirection::Columns
             ? JustifySelfForGridItem(gridItem, StretchingMode::Any, gridStyle)
             : AlignSelfForGridItem(gridItem, StretchingMode::Any, gridStyle);
  }

  bool RenderGrid::SelfAlignmentChangedToStretch(GridTrackSizingDirection alignmentContextType,
                                                 const RenderStyle &oldStyle, const RenderStyle &newStyle,
                                                 const RenderBox &gridItem) const noexcept
  {
    return SelfAlignmentForGridItem(alignmentContextType, gridItem, &oldStyle).Position()
             != ItemPosition::Stretch
           && SelfAlignmentForGridItem(alignmentContextType, gridItem, &newStyle).Position()
                == ItemPosition::Stretch;
  }

  bool RenderGrid::SelfAlignmentChangedFromStretch(GridTrackSizingDirection alignmentContextType,
                                                   const RenderStyle &oldStyle, const RenderStyle &newStyle,
                                                   const RenderBox &gridItem) const noexcept
  {
    return SelfAlignmentForGridItem(alignmentContextType, gridItem, &oldStyle).Position()
             == ItemPosition::Stretch
           && SelfAlignmentForGridItem(alignmentContextType, gridItem, &newStyle).Position()
                != ItemPosition::Stretch;
  }

  void RenderGrid::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlock::StyleDidChange(diff, oldStyle);
    if (!oldStyle || diff != style::DifferenceResult::Layout)
    {
      return;
    }

    _intrinsicLogicalHeightsForRowSizingFirstPass.reset();
    const RenderStyle &newStyle = Style();

    auto hasDifferentTrackSizes = [&newStyle, &oldStyle](GridTrackSizingDirection direction)
    {
      return newStyle.GridTemplateList(direction).Sizes != oldStyle->GridTemplateList(direction).Sizes;
    };

    if (hasDifferentTrackSizes(GridTrackSizingDirection::Columns)
        || hasDifferentTrackSizes(GridTrackSizingDirection::Rows))
    {
      for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
      {
        gridItem.SetChildNeedsLayout();
      }
    }

    if (oldStyle->GetAlignItems().Resolve(SelfAlignmentNormalBehavior(this)).Position()
        == ItemPosition::Stretch)
    {
      // Style changes on the grid container implying stretching (to-stretch) or
      // shrinking (from-stretch) require the affected items to be laid out again.
      // These logic only applies to 'stretch' since the rest of the alignment
      // values don't change the size of the box.
      // In any case, the items' overrideSize will be cleared and recomputed (if
      // necessary)  as part of the Grid layout logic, triggered by this style
      // change.
      for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
      {
        if (gridItem.IsOutOfFlowPositioned())
        {
          continue;
        }

        if (SelfAlignmentChangedToStretch(GridTrackSizingDirection::Columns, *oldStyle, newStyle, gridItem)
            || SelfAlignmentChangedFromStretch(GridTrackSizingDirection::Columns, *oldStyle, newStyle,
                                               gridItem)
            || SelfAlignmentChangedToStretch(GridTrackSizingDirection::Rows, *oldStyle, newStyle, gridItem)
            || SelfAlignmentChangedFromStretch(GridTrackSizingDirection::Rows, *oldStyle, newStyle, gridItem))
        {
          gridItem.SetNeedsLayout();
        }
      }
    }

    auto subgridDidChange = GetSubgridDidChange(*oldStyle);
    auto isSubgridWithIndependentFormattingContextChange = [&]
    {
      if (newStyle.GridTemplateRows().Subgrid || newStyle.GridTemplateColumns().Subgrid)
      {
        return EstablishesIndependentFormattingContextIgnoringDisplayType(*oldStyle)
               != EstablishesIndependentFormattingContextIgnoringDisplayType(Style());
      }
      return false;
    };
    if (ExplicitGridDidResize(*oldStyle) || NamedGridLinesDefinitionDidChange(*oldStyle)
        || ImplicitGridLinesDefinitionDidChange(*oldStyle)
        || oldStyle->GetGridAutoFlow() != Style().GetGridAutoFlow()
        || Style().GridTemplateColumns().AutoRepeatSizes.size()
        || Style().GridTemplateRows().AutoRepeatSizes.size() || subgridDidChange == SubgridDidChange::Yes
        || isSubgridWithIndependentFormattingContextChange())
    {
      SetNeedsItemPlacement(subgridDidChange);
    }
  }

  SubgridDidChange RenderGrid::GetSubgridDidChange(const RenderStyle &oldStyle) const noexcept
  {
    if (oldStyle.GridTemplateRows().Subgrid != Style().GridTemplateRows().Subgrid
        || oldStyle.GridTemplateColumns().Subgrid != Style().GridTemplateColumns().Subgrid)
    {
      return SubgridDidChange::Yes;
    }
    return SubgridDidChange::No;
  }

  bool RenderGrid::ExplicitGridDidResize(const RenderStyle &oldStyle) const noexcept
  {
    auto &oldGridTemplateColumns = oldStyle.GridTemplateColumns();
    auto &oldGridTemplateRows = oldStyle.GridTemplateRows();
    auto &oldGridTemplateAreas = oldStyle.GetGridTemplateAreas();
    auto &newGridTemplateColumns = Style().GridTemplateColumns();
    auto &newGridTemplateRows = Style().GridTemplateRows();
    auto &newGridTemplateAreas = Style().GetGridTemplateAreas();

    return oldGridTemplateColumns.Sizes.size() != newGridTemplateColumns.Sizes.size()
           || oldGridTemplateColumns.AutoRepeatSizes.size() != newGridTemplateColumns.AutoRepeatSizes.size()
           || oldGridTemplateRows.Sizes.size() != newGridTemplateRows.Sizes.size()
           || oldGridTemplateRows.AutoRepeatSizes.size() != newGridTemplateRows.AutoRepeatSizes.size()
           || oldGridTemplateAreas.Map.ColumnCount != newGridTemplateAreas.Map.ColumnCount
           || oldGridTemplateAreas.Map.RowCount != newGridTemplateAreas.Map.RowCount;
  }

  bool RenderGrid::NamedGridLinesDefinitionDidChange(const RenderStyle &oldStyle) const noexcept
  {
    return oldStyle.GridTemplateRows().NamedLines.Map != Style().GridTemplateRows().NamedLines.Map
           || oldStyle.GridTemplateColumns().NamedLines.Map != Style().GridTemplateColumns().NamedLines.Map;
  }

  bool RenderGrid::ImplicitGridLinesDefinitionDidChange(const RenderStyle &oldStyle) const noexcept
  {
    auto &oldGridTemplateAreas = oldStyle.GetGridTemplateAreas();
    auto &newGridTemplateAreas = Style().GetGridTemplateAreas();

    return oldGridTemplateAreas.ImplicitNamedGridRowLines != newGridTemplateAreas.ImplicitNamedGridRowLines
           || oldGridTemplateAreas.ImplicitNamedGridColumnLines
                != newGridTemplateAreas.ImplicitNamedGridColumnLines;
  }

  // This method optimizes the gutters computation by skipping the available size
  // call if gaps are fixed size (it's only needed for percentages).
  Maybe<LayoutUnit> RenderGrid::AvailableSpaceForGutters(GridTrackSizingDirection direction) const noexcept
  {
    if (!Style().Gap(direction).IsPercentOrCalculated())
    {
      return null;
    }

    return direction == GridTrackSizingDirection::Columns ? ContentBoxLogicalWidth()
                                                          : ContentBoxLogicalHeight();
  }

  void RenderGrid::ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection direction,
                                                    LayoutUnit availableSpace,
                                                    GridLayoutState &gridLayoutState) noexcept
  {
    SetForScope autoMarginResolutionScope(_isComputingTrackSizes, true);
    _trackSizingAlgorithm.Run(direction, NumTracks(direction), SizingOperation::TrackSizing, availableSpace,
                              gridLayoutState);
    krys_debug_assert(_trackSizingAlgorithm.TracksAreWiderThanMinTrackBreadth());
  }

  void RenderGrid::RepeatTracksSizingIfNeeded(LayoutUnit availableSpaceForColumns,
                                              LayoutUnit availableSpaceForRows,
                                              GridLayoutState &gridLayoutState) noexcept
  {
    // In orthogonal flow cases column track's size is determined by using the computed
    // row track's size, which it was estimated during the first cycle of the sizing
    // algorithm. Hence we need to repeat computeUsedBreadthOfGridTracks for both,
    // columns and rows, to determine the final values.
    // TODO (lajava): orthogonal flows is just one of the cases which may require
    // a new cycle of the sizing algorithm; there may be more. In addition, not all the
    // cases with orthogonal flows require this extra cycle; we need a more specific
    // condition to detect whether grid item's min-content contribution has changed or not.
    // The complication with repeating the track sizing algorithm for flex max-sizing is that
    // it might change a grid item's status of participating in Baseline Alignment for
    // a cyclic sizing dependency case, which should be definitively excluded. See
    // https://github.com/w3c/csswg-drafts/issues/3046 for details.
    // FIXME: we are avoiding repeating the track sizing algorithm for grid item with baseline alignment
    // here in the case of using flex max-sizing functions. We probably also need to investigate whether
    // it is applicable for the case of percent-sized rows with indefinite height as well.
    if (gridLayoutState.NeedsSecondTrackSizingPass()
        || _trackSizingAlgorithm.HasAnyPercentSizedRowsIndefiniteHeight()
        || (_trackSizingAlgorithm.HasAnyFlexibleMaxTrackBreadth()
            && !_trackSizingAlgorithm.HasAnyBaselineAlignmentItem())
        || gridLayoutState.HasAspectRatioBlockSizeDependentItem())
    {
      PopulateGridPositionsForDirection(_trackSizingAlgorithm, GridTrackSizingDirection::Rows);
      ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Columns, availableSpaceForColumns,
                                       gridLayoutState);
      _offsetBetweenColumns = ComputeContentPositionAndDistributionOffset(
        GridTrackSizingDirection::Columns,
        _trackSizingAlgorithm.FreeSpace(GridTrackSizingDirection::Columns).value(),
        NonCollapsedTracks(GridTrackSizingDirection::Columns));

      ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Rows, availableSpaceForRows,
                                       gridLayoutState);
      _offsetBetweenRows = ComputeContentPositionAndDistributionOffset(
        GridTrackSizingDirection::Rows,
        _trackSizingAlgorithm.FreeSpace(GridTrackSizingDirection::Rows).value(),
        NonCollapsedTracks(GridTrackSizingDirection::Rows));
    }
  }

  bool RenderGrid::CanPerformSimplifiedLayout() const noexcept
  {
    // We cannot perform a simplified layout if we need to position the items and we have some
    // positioned items to be laid out.
    if (CurrentGrid().NeedsItemsPlacement() && OutOfFlowChildNeedsLayout())
    {
      return false;
    }

    return RenderBlock::CanPerformSimplifiedLayout();
  }

  namespace
  {
    enum class AlignmentContextType : uint8
    {
      Columns,
      Rows
    };

    template <typename F>
    void CacheBaselineAlignedGridItems(const RenderGrid &grid, GridTrackSizingAlgorithm &algorithm,
                                       EnumSet<AlignmentContextType> alignmentContextTypes, F &callback,
                                       bool cachingRowSubgridsForRootGrid) noexcept
    {
      for (auto &gridItem : ChildrenOfType<RenderBox>(grid))
      {
        // FIXME: Skip a fieldset's legend (isLegend), once there are fieldsets.
        if (gridItem.IsOutOfFlowPositioned())
        {
          continue;
        }

        callback(const_cast<RenderBox *>(&gridItem));

        // We keep a cache of items with baseline as alignment values so that we only compute the baseline
        // shims for such items. This cache is needed for performance related reasons due to the cost of
        // evaluating the item's participation in a baseline context during the track sizing algorithm.
        EnumSet<AlignmentContextType> innerAlignmentContextTypes;
        auto *inner = DynamicDowncast<RenderGrid>(gridItem);

        if (alignmentContextTypes.Contains(AlignmentContextType::Rows))
        {
          if (inner && inner->IsSubgridInParentDirection(GridTrackSizingDirection::Rows))
          {
            innerAlignmentContextTypes.Add(GridLayoutFunctions::IsOrthogonalGridItem(grid, gridItem)
                                             ? AlignmentContextType::Columns
                                             : AlignmentContextType::Rows);
          }
          else if (grid.IsBaselineAlignmentForGridItem(gridItem, GridTrackSizingDirection::Rows))
          {
            algorithm.CacheBaselineAlignedItem(gridItem, GridTrackSizingDirection::Rows,
                                               cachingRowSubgridsForRootGrid);
          }
        }

        if (alignmentContextTypes.Contains(AlignmentContextType::Columns))
        {
          if (inner && inner->IsSubgridInParentDirection(GridTrackSizingDirection::Columns))
          {
            innerAlignmentContextTypes.Add(GridLayoutFunctions::IsOrthogonalGridItem(grid, gridItem)
                                             ? AlignmentContextType::Rows
                                             : AlignmentContextType::Columns);
          }
          else if (grid.IsBaselineAlignmentForGridItem(gridItem, GridTrackSizingDirection::Columns))
          {
            algorithm.CacheBaselineAlignedItem(gridItem, GridTrackSizingDirection::Columns,
                                               cachingRowSubgridsForRootGrid);
          }
        }

        if (inner && cachingRowSubgridsForRootGrid)
        {
          cachingRowSubgridsForRootGrid =
            GridLayoutFunctions::IsOrthogonalGridItem(*algorithm.GetRenderGrid(), *inner)
              ? inner->IsSubgridColumns()
              : inner->IsSubgridRows();
        }

        if (!innerAlignmentContextTypes.IsEmpty())
        {
          CacheBaselineAlignedGridItems(*inner, algorithm, innerAlignmentContextTypes, callback,
                                        cachingRowSubgridsForRootGrid);
        }
      }
    }

    void ClearGridItemOverridingSizesBeforeLayout(RenderGrid &renderGrid) noexcept
    {
      // Grid's layout logic controls the grid item's override content size, hence we need to
      // clear any override set previously, so it doesn't interfere in current layout
      // execution.
      for (auto &gridItem : ChildrenOfType<RenderBox>(renderGrid))
      {
        // FIXME: Skip a fieldset's legend (isLegend), once there are fieldsets.
        if (gridItem.IsOutOfFlowPositioned())
        {
          continue;
        }
        gridItem.ClearOverridingSize();
      }
    }
  }

  List<RenderBox *>
    RenderGrid::ComputeAspectRatioDependentAndBaselineItems(GridLayoutState &gridLayoutState) noexcept
  {
    List<RenderBox *> dependentGridItems;

    _baselineItemsCached = true;

    auto computeOrthogonalAndDependentItems = [&](RenderBox *gridItem)
    {
      // For a grid item that has an aspect-ratio and block-constraints such as the relative logical height,
      // when the grid width is auto, we may need get the real grid width before laying out the item.
      if (GridLayoutFunctions::IsAspectRatioBlockSizeDependentGridItem(*gridItem)
          && (Style().LogicalWidth().IsAuto() || Style().LogicalWidth().IsMinContent()
              || Style().LogicalWidth().IsMaxContent()))
      {
        dependentGridItems.push_back(gridItem);
        gridLayoutState.SetHasAspectRatioBlockSizeDependentItem();
      }
    };

    CacheBaselineAlignedGridItems(
      *this, _trackSizingAlgorithm,
      EnumSet<AlignmentContextType> {AlignmentContextType::Columns, AlignmentContextType::Rows},
      computeOrthogonalAndDependentItems, !IsSubgridRows());
    return dependentGridItems;
  }

  bool RenderGrid::CanSetColumnAxisStretchRequirementForItem(const RenderBox &gridItem) const noexcept
  {
    auto gridItemBlockFlowDirection =
      GridLayoutFunctions::FlowAwareDirectionForGridItem(*this, gridItem, GridTrackSizingDirection::Rows);
    return gridItemBlockFlowDirection == GridTrackSizingDirection::Rows
           && GridLayoutFunctions::AllowedToStretchGridItemAlongColumnAxis(
             gridItem, AlignSelfForGridItem(gridItem).Position(), GetWritingMode());
  }

  void
    RenderGrid::ComputeLayoutRequirementsForItemsBeforeLayout(GridLayoutState &gridLayoutState) const noexcept
  {
    for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
    {
      auto gridItemAlignSelf = AlignSelfForGridItem(gridItem).Position();
      if (GridLayoutFunctions::IsGridItemInlineSizeDependentOnBlockConstraints(gridItem, *this,
                                                                               gridItemAlignSelf))
      {
        gridLayoutState.SetNeedsSecondTrackSizingPass();
        gridLayoutState.SetLayoutRequirementForGridItem(
          gridItem, ItemLayoutRequirement::MinContentContributionForSecondColumnPass);
      }

      if (!gridItem.NeedsLayout() || gridItem.IsOutOfFlowPositioned()
          || gridItem.IsExcludedFromNormalLayout())
      {
        continue;
      }

      if (CanSetColumnAxisStretchRequirementForItem(gridItem))
      {
        gridLayoutState.SetLayoutRequirementForGridItem(
          gridItem, ItemLayoutRequirement::NeedsColumnAxisStretchAlignment);
      }
    }
  }

  void RenderGrid::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    // The layoutBlock was handling the layout of both the grid and masonry implementations.
    // This caused a huge amount of branching code to handle masonry specific cases. Splitting up the code
    // to layout will simplify both implementations.
    if (!IsMasonry())
    {
      LayoutGrid(relayoutChildren);
    }
    else
    {
      LayoutMasonry(relayoutChildren);
    }
  }

  bool RenderGrid::HasDefiniteLogicalHeight() const noexcept
  {
    // FIXME: We should use RenderBlock::hasDefiniteLogicalHeight() only but it does not work for out of flow
    // content.
    return RenderBlock::HasDefiniteLogicalHeight() || OverridingBorderBoxLogicalHeight()
           || ComputeContentLogicalHeight(Style().LogicalHeight(), null)
           || ShouldComputeLogicalHeightFromAspectRatio();
  }

  Maybe<LayoutUnit> RenderGrid::AvailableLogicalHeightForContentBox() const noexcept
  {
    if (!HasDefiniteLogicalHeight())
    {
      return {};
    }

    if (auto overridingLogicalHeight = OverridingBorderBoxLogicalHeight())
    {
      return ConstrainContentBoxLogicalHeightByMinMax(
        *overridingLogicalHeight - BorderAndPaddingLogicalHeight(), null);
    }
    return AvailableLogicalHeight(AvailableLogicalHeightType::ExcludeMarginBorderPadding);
  }

  void RenderGrid::LayoutGrid(RelayoutChildren relayoutChildren) noexcept
  {
    // FIXME: Repaint (LayoutRepainter), once there's painting.
    {
      // There are no reflections (-webkit-box-reflect was dropped).
      LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                        IsTransformed() || GetWritingMode().IsBlockFlipped());

      GridLayoutState gridLayoutState;

      UpdateIntrinsicLogicalHeightsForRowSizingFirstPassCacheAvailability();
      ClearGridItemOverridingSizesBeforeLayout(*this);
      ComputeLayoutRequirementsForItemsBeforeLayout(gridLayoutState);

      // FIXME: The fragments' widths (preparePaginationBeforeBlockLayout), once there's pagination.
      BeginUpdateScrollInfoAfterLayoutTransaction();

      LayoutSize previousSize = Size();

      auto aspectRatioBlockSizeDependentGridItems =
        ComputeAspectRatioDependentAndBaselineItems(gridLayoutState);

      ResetLogicalHeightBeforeLayoutIfNeeded();

      UpdateLogicalWidth();

      if (LayoutUsingGridFormattingContext())
      {
        EndAndCommitUpdateScrollInfoAfterLayoutTransaction();
        _trackSizingAlgorithm.ClearBaselineItemsCache();
        _baselineItemsCached = false;
        return;
      }

      // Fieldsets need to find their legend and position it inside the border of the object.
      // The legend then gets skipped during normal layout. The same is true for ruby text.
      // It doesn't get included in the normal layout process but is instead skipped.
      LayoutExcludedChildren(relayoutChildren);

      LayoutUnit availableSpaceForColumns = ContentBoxLogicalWidth();
      PlaceItemsOnGrid(availableSpaceForColumns);

      _trackSizingAlgorithm.SetAvailableSpace(GridTrackSizingDirection::Columns, availableSpaceForColumns);
      PerformPreLayoutForGridItems(_trackSizingAlgorithm, ShouldUpdateGridAreaLogicalSize::Yes);

      // 1. First, the track sizing algorithm is used to resolve the sizes of the grid columns. At this point
      // the logical width is always definite as the above call to updateLogicalWidth() properly resolves
      // intrinsic sizes. We cannot do the same for heights though because many code paths inside
      // updateLogicalHeight() require a previous call to setLogicalHeight() to resolve heights properly (like
      // for positioned items for example).
      ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Columns, availableSpaceForColumns,
                                       gridLayoutState);

      // 1.5. Compute Content Distribution offsets for column tracks
      _offsetBetweenColumns = ComputeContentPositionAndDistributionOffset(
        GridTrackSizingDirection::Columns,
        _trackSizingAlgorithm.FreeSpace(GridTrackSizingDirection::Columns).value(),
        NonCollapsedTracks(GridTrackSizingDirection::Columns));

      // 2. Next, the track sizing algorithm resolves the sizes of the grid rows,
      // using the grid column sizes calculated in the previous step.
      auto availableLogicalHeightForContentBox = AvailableLogicalHeightForContentBox();
      bool shouldRecomputeHeight = false;
      if (!availableLogicalHeightForContentBox)
      {
        ComputeTrackSizesForIndefiniteSize(_trackSizingAlgorithm, GridTrackSizingDirection::Rows,
                                           gridLayoutState);
        if (ShouldApplySizeContainment())
        {
          shouldRecomputeHeight = true;
        }
      }
      else
      {
        ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Rows, *availableLogicalHeightForContentBox,
                                         gridLayoutState);
      }

      LayoutUnit trackBasedLogicalHeight =
        BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight());
      if (auto size = ExplicitIntrinsicInnerLogicalSize(GridTrackSizingDirection::Rows))
      {
        trackBasedLogicalHeight += size.value();
      }
      else
      {
        trackBasedLogicalHeight += _trackSizingAlgorithm.ComputeTrackBasedSize();
      }

      if (shouldRecomputeHeight)
      {
        ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Rows, trackBasedLogicalHeight,
                                         gridLayoutState);
      }

      SetLogicalHeight(trackBasedLogicalHeight);

      UpdateLogicalHeight();

      // Once grid's indefinite height is resolved, we can compute the
      // available free space for Content Alignment.
      if (!availableLogicalHeightForContentBox)
      {
        _trackSizingAlgorithm.SetFreeSpace(GridTrackSizingDirection::Rows,
                                           LogicalHeight() - trackBasedLogicalHeight);
      }

      // 2.5. Compute Content Distribution offsets for rows tracks
      _offsetBetweenRows = ComputeContentPositionAndDistributionOffset(
        GridTrackSizingDirection::Rows,
        _trackSizingAlgorithm.FreeSpace(GridTrackSizingDirection::Rows).value(),
        NonCollapsedTracks(GridTrackSizingDirection::Rows));

      if (!aspectRatioBlockSizeDependentGridItems.empty())
      {
        UpdateGridAreaForAspectRatioItems(aspectRatioBlockSizeDependentGridItems, gridLayoutState);
        UpdateLogicalWidth();
      }

      // 3. If the min-content contribution of any grid items have changed based on the row
      // sizes calculated in step 2, steps 1 and 2 are repeated with the new min-content
      // contribution (once only).
      RepeatTracksSizingIfNeeded(availableSpaceForColumns, ContentBoxLogicalHeight(), gridLayoutState);

      // Grid container should have the minimum height of a line if it's editable. That does not affect track
      // sizing though.
      if (HasLineIfEmpty())
      {
        LayoutUnit minHeightForEmptyLine =
          BorderAndPaddingLogicalHeight() + LineHeight() + LayoutUnit(ScrollbarLogicalHeight());
        SetLogicalHeight(std::max(LogicalHeight(), minHeightForEmptyLine));
      }

      LayoutGridItems(gridLayoutState);

      EndAndCommitUpdateScrollInfoAfterLayoutTransaction();

      if (Size() != previousSize)
      {
        relayoutChildren = RelayoutChildren::Yes;
      }

      if (IsDocumentElementRenderer())
      {
        LayoutOutOfFlowBoxes(RelayoutChildren::Yes);
      }
      else
      {
        LayoutOutOfFlowBoxes(relayoutChildren);
      }

      _trackSizingAlgorithm.Reset();

      ComputeOverflow(LayoutOverflowLogicalBottom(*this));

      UpdateDescendantTransformsAfterLayout();
    }

    UpdateLayerTransform();

    _trackSizingAlgorithm.ClearBaselineItemsCache();
    _baselineItemsCached = false;
  }

  bool RenderGrid::LayoutUsingGridFormattingContext() noexcept
  {
    if (_hasGridFormattingContextLayout && !*_hasGridFormattingContextLayout)
    {
      // FIXME: Avoid continous content checking on (potentially) unsupported content. Remove when the
      // coverage check becomes less expensive.
      return false;
    }

    _hasGridFormattingContextLayout = layout_integration::GridLayout::CanUseFor(*this);
    if (!*_hasGridFormattingContextLayout)
    {
      // The grid formatting context placed the items without the grid, so place them again.
      CurrentGrid().SetNeedsItemsPlacement(true);
      return false;
    }

    auto gridLayout = layout_integration::GridLayout {*this};
    gridLayout.UpdateFormattingContextGeometries();

    gridLayout.Layout();
    UpdateLogicalHeight();
    return true;
  }

  void RenderGrid::LayoutMasonry(RelayoutChildren relayoutChildren) noexcept
  {
    // FIXME: Repaint (LayoutRepainter), once there's painting.
    {
      // There are no reflections (-webkit-box-reflect was dropped).
      LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                        IsTransformed() || GetWritingMode().IsBlockFlipped());
      GridLayoutState gridLayoutState;

      ClearGridItemOverridingSizesBeforeLayout(*this);

      // FIXME: The fragments' widths (preparePaginationBeforeBlockLayout), once there's pagination.
      BeginUpdateScrollInfoAfterLayoutTransaction();

      LayoutSize previousSize = Size();

      // FIXME: We should use RenderBlock::hasDefiniteLogicalHeight() only but it does not work for positioned
      // stuff.
      // FIXME: Consider caching the hasDefiniteLogicalHeight value throughout the layout.
      // FIXME: We might need to cache the hasDefiniteLogicalHeight if the call of
      // RenderBlock::hasDefiniteLogicalHeight() causes a relevant performance regression.
      bool hasDefiniteLogicalHeight = RenderBlock::HasDefiniteLogicalHeight()
                                      || OverridingBorderBoxLogicalHeight()
                                      || ComputeContentLogicalHeight(Style().LogicalHeight(), null);

      auto aspectRatioBlockSizeDependentGridItems =
        ComputeAspectRatioDependentAndBaselineItems(gridLayoutState);

      ResetLogicalHeightBeforeLayoutIfNeeded();

      // Fieldsets need to find their legend and position it inside the border of the object.
      // The legend then gets skipped during normal layout. The same is true for ruby text.
      // It doesn't get included in the normal layout process but is instead skipped.
      LayoutExcludedChildren(relayoutChildren);

      UpdateLogicalWidth();

      LayoutUnit availableSpaceForColumns = ContentBoxLogicalWidth();
      PlaceItemsOnGrid(availableSpaceForColumns);

      _trackSizingAlgorithm.SetAvailableSpace(GridTrackSizingDirection::Columns, availableSpaceForColumns);
      PerformPreLayoutForGridItems(_trackSizingAlgorithm, ShouldUpdateGridAreaLogicalSize::Yes);

      // 1. First, the track sizing algorithm is used to resolve the sizes of the grid columns. At this point
      // the logical width is always definite as the above call to updateLogicalWidth() properly resolves
      // intrinsic sizes. We cannot do the same for heights though because many code paths inside
      // updateLogicalHeight() require a previous call to setLogicalHeight() to resolve heights properly (like
      // for positioned items for example).
      ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Columns, availableSpaceForColumns,
                                       gridLayoutState);

      // 1.5. Compute Content Distribution offsets for column tracks
      _offsetBetweenColumns = ComputeContentPositionAndDistributionOffset(
        GridTrackSizingDirection::Columns,
        _trackSizingAlgorithm.FreeSpace(GridTrackSizingDirection::Columns).value(),
        NonCollapsedTracks(GridTrackSizingDirection::Columns));

      // 2. Next, the track sizing algorithm resolves the sizes of the grid rows,
      // using the grid column sizes calculated in the previous step.
      bool shouldRecomputeHeight = false;
      if (!hasDefiniteLogicalHeight)
      {
        ComputeTrackSizesForIndefiniteSize(_trackSizingAlgorithm, GridTrackSizingDirection::Rows,
                                           gridLayoutState);
        if (ShouldApplySizeContainment())
        {
          shouldRecomputeHeight = true;
        }
      }
      else
      {
        ComputeTrackSizesForDefiniteSize(
          GridTrackSizingDirection::Rows,
          AvailableLogicalHeight(AvailableLogicalHeightType::ExcludeMarginBorderPadding), gridLayoutState);
      }

      auto performMasonryPlacement = [&](const GridTrackSizingDirection masonryAxisDirection)
      {
        auto gridAxisDirection = style::OrthogonalDirection(masonryAxisDirection);
        uint32 gridAxisTracksBeforeAutoPlacement = CurrentGrid().NumTracks(gridAxisDirection);

        _masonryLayout.PerformMasonryPlacement(_trackSizingAlgorithm, gridAxisTracksBeforeAutoPlacement,
                                               masonryAxisDirection,
                                               GridMasonryLayout::MasonryLayoutPhase::LayoutPhase);
      };

      if (AreMasonryRows())
      {
        performMasonryPlacement(GridTrackSizingDirection::Rows);
      }
      else if (AreMasonryColumns())
      {
        performMasonryPlacement(GridTrackSizingDirection::Columns);
      }

      LayoutUnit trackBasedLogicalHeight =
        BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight());
      if (auto size = ExplicitIntrinsicInnerLogicalSize(GridTrackSizingDirection::Rows))
      {
        trackBasedLogicalHeight += size.value();
      }
      else
      {
        if (AreMasonryRows())
        {
          trackBasedLogicalHeight += _masonryLayout.GridContentSize();
        }
        else
        {
          trackBasedLogicalHeight += _trackSizingAlgorithm.ComputeTrackBasedSize();
        }
      }
      if (shouldRecomputeHeight)
      {
        ComputeTrackSizesForDefiniteSize(GridTrackSizingDirection::Rows, trackBasedLogicalHeight,
                                         gridLayoutState);
      }

      SetLogicalHeight(trackBasedLogicalHeight);

      UpdateLogicalHeight();

      // Once grid's indefinite height is resolved, we can compute the
      // available free space for Content Alignment.
      if (!hasDefiniteLogicalHeight || AreMasonryRows())
      {
        _trackSizingAlgorithm.SetFreeSpace(GridTrackSizingDirection::Rows,
                                           LogicalHeight() - trackBasedLogicalHeight);
      }

      // 2.5. Compute Content Distribution offsets for rows tracks
      _offsetBetweenRows = ComputeContentPositionAndDistributionOffset(
        GridTrackSizingDirection::Rows,
        _trackSizingAlgorithm.FreeSpace(GridTrackSizingDirection::Rows).value(),
        NonCollapsedTracks(GridTrackSizingDirection::Rows));

      if (!aspectRatioBlockSizeDependentGridItems.empty())
      {
        UpdateGridAreaForAspectRatioItems(aspectRatioBlockSizeDependentGridItems, gridLayoutState);
        UpdateLogicalWidth();
      }

      // Grid container should have the minimum height of a line if it's editable. That does not affect track
      // sizing though.
      if (HasLineIfEmpty())
      {
        LayoutUnit minHeightForEmptyLine =
          BorderAndPaddingLogicalHeight() + LineHeight() + LayoutUnit(ScrollbarLogicalHeight());
        SetLogicalHeight(std::max(LogicalHeight(), minHeightForEmptyLine));
      }

      LayoutMasonryItems(gridLayoutState);

      EndAndCommitUpdateScrollInfoAfterLayoutTransaction();

      if (Size() != previousSize)
      {
        relayoutChildren = RelayoutChildren::Yes;
      }

      if (IsDocumentElementRenderer())
      {
        LayoutOutOfFlowBoxes(RelayoutChildren::Yes);
      }
      else
      {
        LayoutOutOfFlowBoxes(relayoutChildren);
      }

      _trackSizingAlgorithm.Reset();

      ComputeOverflow(LayoutOverflowLogicalBottom(*this));

      UpdateDescendantTransformsAfterLayout();
    }

    UpdateLayerTransform();

    _trackSizingAlgorithm.ClearBaselineItemsCache();
    _baselineItemsCached = false;
  }

  LayoutUnit RenderGrid::GridGap(GridTrackSizingDirection direction,
                                 Maybe<LayoutUnit> availableSize) const noexcept
  {
    krys_debug_assert(!availableSize || *availableSize >= 0);
    auto &gap = Style().Gap(direction);
    if (gap.IsNormal())
    {
      if (!IsSubgrid(direction))
      {
        return 0_lu;
      }

      auto parentDirection = GridLayoutFunctions::FlowAwareDirectionForParent(*this, *Parent(), direction);
      if (!availableSize)
      {
        return Downcast<RenderGrid>(Parent())->GridGap(parentDirection, null);
      }
      return Downcast<RenderGrid>(Parent())->GridGap(parentDirection);
    }

    return style::EvaluateMinimum<LayoutUnit>(gap, availableSize.value_or(0_lu), style::ZoomNeeded {});
  }

  LayoutUnit RenderGrid::GridGap(GridTrackSizingDirection direction) const noexcept
  {
    return GridGap(direction, AvailableSpaceForGutters(direction));
  }

  LayoutUnit RenderGrid::GridItemOffset(GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Rows ? _offsetBetweenRows.DistributionOffset
                                                       : _offsetBetweenColumns.DistributionOffset;
  }

  LayoutUnit RenderGrid::GuttersSize(GridTrackSizingDirection direction, uint32 startLine, uint32 span,
                                     Maybe<LayoutUnit> availableSize) const noexcept
  {
    if (span <= 1)
    {
      return {};
    }

    LayoutUnit gap = GridGap(direction, availableSize);

    // Fast path, no collapsing tracks.
    if (!CurrentGrid().HasAutoRepeatEmptyTracks(direction))
    {
      return gap * static_cast<int>(span - 1);
    }

    // If there are collapsing tracks we need to be sure that gutters are properly collapsed. Apart
    // from that, if we have a collapsed track in the edges of the span we're considering, we need
    // to move forward (or backwards) in order to know whether the collapsed tracks reach the end of
    // the grid (so the gap becomes 0) or there is a non empty track before that.

    LayoutUnit gapAccumulator;
    uint32 endLine = startLine + span;

    for (uint32 line = startLine; line < endLine - 1; ++line)
    {
      if (!CurrentGrid().IsEmptyAutoRepeatTrack(direction, line))
      {
        gapAccumulator += gap;
      }
    }

    // The above loop adds one extra gap for trailing collapsed tracks.
    if (gapAccumulator && CurrentGrid().IsEmptyAutoRepeatTrack(direction, endLine - 1))
    {
      krys_debug_assert(gapAccumulator >= gap);
      gapAccumulator -= gap;
    }

    // If the startLine is the start line of a collapsed track we need to go backwards till we reach
    // a non collapsed track. If we find a non collapsed track we need to add that gap.
    size_t nonEmptyTracksBeforeStartLine = 0;
    if (startLine && CurrentGrid().IsEmptyAutoRepeatTrack(direction, startLine))
    {
      nonEmptyTracksBeforeStartLine = startLine;
      auto begin = CurrentGrid().AutoRepeatEmptyTracks(direction)->begin();
      for (auto it = begin; *it != startLine; ++it)
      {
        krys_debug_assert(nonEmptyTracksBeforeStartLine);
        --nonEmptyTracksBeforeStartLine;
      }
      if (nonEmptyTracksBeforeStartLine)
      {
        gapAccumulator += gap;
      }
    }

    // If the endLine is the end line of a collapsed track we need to go forward till we reach a non
    // collapsed track. If we find a non collapsed track we need to add that gap.
    if (CurrentGrid().IsEmptyAutoRepeatTrack(direction, endLine - 1))
    {
      uint32 nonEmptyTracksAfterEndLine = CurrentGrid().NumTracks(direction) - endLine;
      auto currentEmptyTrack = CurrentGrid().AutoRepeatEmptyTracks(direction)->find(endLine - 1);
      auto endEmptyTrack = CurrentGrid().AutoRepeatEmptyTracks(direction)->end();
      // HashSet iterators do not implement operator- so we have to manually iterate to know the number of
      // remaining empty tracks.
      for (auto it = ++currentEmptyTrack; it != endEmptyTrack; ++it)
      {
        krys_debug_assert(nonEmptyTracksAfterEndLine >= 1);
        --nonEmptyTracksAfterEndLine;
      }
      if (nonEmptyTracksAfterEndLine)
      {
        // We shouldn't count the gap twice if the span starts and ends in a collapsed track between two
        // non-empty tracks.
        if (!nonEmptyTracksBeforeStartLine)
        {
          gapAccumulator += gap;
        }
      }
      else if (nonEmptyTracksBeforeStartLine)
      {
        // We shouldn't count the gap if the span starts and ends in a collapsed but there isn't non-empty
        // tracks afterwards (it's at the end of the grid).
        gapAccumulator -= gap;
      }
    }

    return gapAccumulator;
  }

  void RenderGrid::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                 LayoutUnit &maxLogicalWidth) const noexcept
  {
    GridLayoutState gridLayoutState;

    LayoutUnit gridItemMinWidth;
    LayoutUnit gridItemMaxWidth;
    bool hadExcludedChildren = ComputePreferredWidthsForExcludedChildren(gridItemMinWidth, gridItemMaxWidth);

    Grid grid(const_cast<RenderGrid &>(*this));
    _grid.CurrentGrid = std::ref(grid);
    GridTrackSizingAlgorithm algorithm(this, grid);
    // placeItemsOnGrid isn't const since it mutates our grid, but it's safe to do
    // so here since we've overridden m_currentGrid with a stack based temporary.
    const_cast<RenderGrid &>(*this).PlaceItemsOnGrid(null);

    PerformPreLayoutForGridItems(algorithm, ShouldUpdateGridAreaLogicalSize::No);

    if (_baselineItemsCached)
    {
      algorithm.CopyBaselineItemsCache(_trackSizingAlgorithm, GridTrackSizingDirection::Columns);
    }
    else
    {
      auto emptyCallback = [](RenderBox *) {};
      CacheBaselineAlignedGridItems(*this, algorithm,
                                    EnumSet<AlignmentContextType> {AlignmentContextType::Columns},
                                    emptyCallback, !IsSubgridRows());
    }

    ComputeTrackSizesForIndefiniteSize(algorithm, GridTrackSizingDirection::Columns, gridLayoutState,
                                       &minLogicalWidth, &maxLogicalWidth);

    if (IsMasonry(GridTrackSizingDirection::Columns))
    {
      // The track sizing algorithm will only be run once in this case, since track sizing will not run in the
      // masonry direction.
      ComputeTrackSizesForIndefiniteSize(algorithm, GridTrackSizingDirection::Rows, gridLayoutState,
                                         &minLogicalWidth, &maxLogicalWidth);

      auto gridAxisTracksCountBeforeAutoPlacement = CurrentGrid().NumTracks(GridTrackSizingDirection::Rows);

      // To determine the width of the grid when we have a masonry layout in the column direction we need to
      // perform a layout with the min and max content sizes. We will override the grid items widths to
      // accomplish this and then calculate the final grid content size after placement.
      _masonryLayout.PerformMasonryPlacement(algorithm, gridAxisTracksCountBeforeAutoPlacement,
                                             GridTrackSizingDirection::Columns,
                                             GridMasonryLayout::MasonryLayoutPhase::MinContentPhase);
      minLogicalWidth = _masonryLayout.GridContentSize();

      _masonryLayout.PerformMasonryPlacement(algorithm, gridAxisTracksCountBeforeAutoPlacement,
                                             GridTrackSizingDirection::Columns,
                                             GridMasonryLayout::MasonryLayoutPhase::MaxContentPhase);
      maxLogicalWidth = _masonryLayout.GridContentSize();
    }

    _grid.ResetCurrentGrid();

    if (hadExcludedChildren)
    {
      minLogicalWidth = std::max(minLogicalWidth, gridItemMinWidth);
      maxLogicalWidth = std::max(maxLogicalWidth, gridItemMaxWidth);
    }

    LayoutUnit scrollbarWidth = IntrinsicScrollbarLogicalWidthIncludingGutter();
    minLogicalWidth += scrollbarWidth;
    maxLogicalWidth += scrollbarWidth;
  }

  void RenderGrid::ComputeTrackSizesForIndefiniteSize(GridTrackSizingAlgorithm &algorithm,
                                                      GridTrackSizingDirection direction,
                                                      GridLayoutState &gridLayoutState,
                                                      LayoutUnit *minIntrinsicSize,
                                                      LayoutUnit *maxIntrinsicSize) const noexcept
  {
    SetForScope autoMarginResolutionScope(_isComputingTrackSizes, true);
    algorithm.Run(direction, NumTracks(direction), SizingOperation::IntrinsicSizeComputation, null,
                  gridLayoutState);

    size_t numberOfTracks = algorithm.GetTracks(direction).size();
    LayoutUnit totalGuttersSize = direction == GridTrackSizingDirection::Columns
                                      && ExplicitIntrinsicInnerLogicalSize(direction).has_value()
                                    ? 0_lu
                                    : GuttersSize(direction, 0, static_cast<uint32>(numberOfTracks), null);

    if (minIntrinsicSize)
    {
      *minIntrinsicSize = algorithm.MinContentSize() + totalGuttersSize;
    }
    if (maxIntrinsicSize)
    {
      *maxIntrinsicSize = algorithm.MaxContentSize() + totalGuttersSize;
    }

    krys_debug_assert(algorithm.TracksAreWiderThanMinTrackBreadth());
  }

  bool RenderGrid::ShouldCheckExplicitIntrinsicInnerLogicalSize(
    GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Columns ? ShouldApplySizeOrInlineSizeContainment()
                                                          : ShouldApplySizeContainment();
  }

  Maybe<LayoutUnit>
    RenderGrid::ExplicitIntrinsicInnerLogicalSize(GridTrackSizingDirection direction) const noexcept
  {
    if (!ShouldCheckExplicitIntrinsicInnerLogicalSize(direction))
    {
      return null;
    }
    if (direction == GridTrackSizingDirection::Columns)
    {
      return ExplicitIntrinsicInnerLogicalWidth();
    }
    return ExplicitIntrinsicInnerLogicalHeight();
  }

  uint32 RenderGrid::ComputeAutoRepeatTracksCount(GridTrackSizingDirection direction,
                                                  Maybe<LayoutUnit> availableSize) const noexcept
  {
    krys_debug_assert(!availableSize || availableSize.value() != -1);
    bool isRowAxis = direction == GridTrackSizingDirection::Columns;
    if (IsSubgrid(direction))
    {
      return 0;
    }

    const auto &tracks = Style().GridTemplateList(direction);
    const auto &autoRepeatTracks = tracks.AutoRepeatSizes;
    auto autoRepeatTrackListLength = static_cast<uint32>(autoRepeatTracks.size());

    if (!autoRepeatTrackListLength)
    {
      return 0;
    }

    bool needsToFulfillMinimumSize = false;
    if (!availableSize)
    {
      // Both min-width/height and max-width/height calculations may need the containing block size, so it is
      // cached if lazily computed.
      Maybe<LayoutUnit> cachedContainingBlockAvailableSize;
      auto containingBlockAvailableSize = [&]() -> LayoutUnit
      {
        if (cachedContainingBlockAvailableSize)
        {
          return *cachedContainingBlockAvailableSize;
        }
        cachedContainingBlockAvailableSize =
          isRowAxis
            ? ContainingBlockLogicalWidthForContent()
            : ContainingBlockLogicalHeightForContent(AvailableLogicalHeightType::ExcludeMarginBorderPadding);
        return *cachedContainingBlockAvailableSize;
      };

      auto adjustForBoxSizing = [&](LayoutUnit size) -> Maybe<LayoutUnit>
      {
        return isRowAxis ? AdjustContentBoxLogicalWidthForBoxSizing(size)
                         : AdjustContentBoxLogicalHeightForBoxSizing(size);
      };

      auto &maxSize = isRowAxis ? Style().LogicalMaxWidth() : Style().LogicalMaxHeight();
      auto availableMaxSize = maxSize.SwitchOn(
        [&](const style::MaximumSize::Fixed &fixedMaxSize) -> Maybe<LayoutUnit>
        { return adjustForBoxSizing(LayoutUnit {fixedMaxSize.ResolveZoom(Style().UsedZoomForLength())}); },
        [&](const style::MaximumSize::Percentage &percentageMaxSize) -> Maybe<LayoutUnit>
        {
          return adjustForBoxSizing(
            style::Evaluate<LayoutUnit>(percentageMaxSize, containingBlockAvailableSize()));
        },
        [&](const style::MaximumSize::Calc &calcMaxSize) -> Maybe<LayoutUnit>
        {
          return adjustForBoxSizing(style::Evaluate<LayoutUnit>(calcMaxSize, containingBlockAvailableSize(),
                                                                Style().UsedZoomForLength()));
        },
        [&](const auto &) -> Maybe<LayoutUnit> { return {}; });

      auto &minSize = isRowAxis ? Style().LogicalMinWidth() : Style().LogicalMinHeight();
      auto &minSizeForOrthogonalAxis = isRowAxis ? Style().LogicalMinHeight() : Style().LogicalMinWidth();
      bool shouldComputeMinSizeFromAspectRatio =
        minSizeForOrthogonalAxis.IsSpecified() && !ShouldIgnoreAspectRatio();
      auto explicitIntrinsicInnerSize = ExplicitIntrinsicInnerLogicalSize(direction);

      if (!availableMaxSize && !minSize.IsSpecified() && !shouldComputeMinSizeFromAspectRatio
          && !explicitIntrinsicInnerSize)
      {
        return autoRepeatTrackListLength;
      }

      auto availableMinSize = minSize.SwitchOn(
        [&](const style::MinimumSize::Fixed &fixedMinSize) -> Maybe<LayoutUnit>
        { return adjustForBoxSizing(LayoutUnit {fixedMinSize.ResolveZoom(Style().UsedZoomForLength())}); },
        [&](const style::MinimumSize::Percentage &percentageMinSize) -> Maybe<LayoutUnit>
        {
          return adjustForBoxSizing(
            style::Evaluate<LayoutUnit>(percentageMinSize, containingBlockAvailableSize()));
        },
        [&](const style::MinimumSize::Calc &calcMinSize) -> Maybe<LayoutUnit>
        {
          return adjustForBoxSizing(style::Evaluate<LayoutUnit>(calcMinSize, containingBlockAvailableSize(),
                                                                Style().UsedZoomForLength()));
        },
        [&](const auto &) -> Maybe<LayoutUnit>
        {
          if (!shouldComputeMinSizeFromAspectRatio)
          {
            return {};
          }
          auto [logicalMinWidth, logicalMaxWidth] = ComputeMinMaxLogicalWidthFromAspectRatio();
          return logicalMinWidth;
        });

      if (!maxSize.IsSpecified() || explicitIntrinsicInnerSize)
      {
        needsToFulfillMinimumSize = true;
      }

      availableSize = std::max(std::max(availableMinSize.value_or(0_lu), availableMaxSize.value_or(0_lu)),
                               explicitIntrinsicInnerSize.value_or(0_lu));
      if (maxSize.IsSpecified() && availableMaxSize < availableSize)
      {
        availableSize = std::max(availableMinSize, availableMaxSize);
      }
    }

    LayoutUnit autoRepeatTracksSize;
    for (auto &autoTrackSize : autoRepeatTracks)
    {
      krys_debug_assert(autoTrackSize.MinTrackBreadth().IsLength());
      krys_debug_assert(!autoTrackSize.MinTrackBreadth().IsFlex());

      auto &minTrackSizingFunction = autoTrackSize.MinTrackBreadth();
      auto &maxTrackSizingFunction = autoTrackSize.MaxTrackBreadth();
      bool hasDefiniteMaxTrackSizingFunction =
        maxTrackSizingFunction.IsLength() && !maxTrackSizingFunction.IsContentSized();
      bool hasDefiniteMinTrackSizingFunction =
        minTrackSizingFunction.IsLength() && !minTrackSizingFunction.IsContentSized();

      auto evaluate = [&](const style::GridTrackBreadth &breadth)
      {
        return style::EvaluateMinimum<LayoutUnit>(breadth.Length(), *availableSize, style::ZoomNeeded {});
      };
      auto contributingTrackSize = [&]
      {
        if (hasDefiniteMaxTrackSizingFunction && hasDefiniteMinTrackSizingFunction)
        {
          return std::max(evaluate(minTrackSizingFunction), evaluate(maxTrackSizingFunction));
        }
        return hasDefiniteMaxTrackSizingFunction ? evaluate(maxTrackSizingFunction)
                                                 : evaluate(minTrackSizingFunction);
      };
      autoRepeatTracksSize += contributingTrackSize();
    }
    // For the purpose of finding the number of auto-repeated tracks, the UA must floor the track size to a
    // UA-specified value to avoid division by zero. It is suggested that this floor be 1px.
    autoRepeatTracksSize = std::max<LayoutUnit>(1_lu, autoRepeatTracksSize);

    // There will be always at least 1 auto-repeat track, so take it already into account when computing the
    // total track size.
    LayoutUnit tracksSize = autoRepeatTracksSize;
    auto &trackSizes = tracks.Sizes;

    for (const auto &track : trackSizes)
    {
      bool hasDefiniteMaxTrackBreadth =
        track.MaxTrackBreadth().IsLength() && !track.MaxTrackBreadth().IsContentSized();
      krys_debug_assert(hasDefiniteMaxTrackBreadth
                        || (track.MinTrackBreadth().IsLength() && !track.MinTrackBreadth().IsContentSized()));
      tracksSize += style::EvaluateMinimum<LayoutUnit>(
        hasDefiniteMaxTrackBreadth ? track.MaxTrackBreadth().Length() : track.MinTrackBreadth().Length(),
        availableSize.value(), style::ZoomNeeded {});
    }

    // Add gutters as if auto repeat tracks were only repeated once. Gaps between different repetitions will
    // be added later when computing the number of repetitions of the auto repeat().
    LayoutUnit gapSize = GridGap(direction, availableSize);
    tracksSize += gapSize * static_cast<int>(trackSizes.size() + autoRepeatTrackListLength - 1);

    LayoutUnit freeSpace = availableSize.value() - tracksSize;
    if (freeSpace <= 0)
    {
      return autoRepeatTrackListLength;
    }

    LayoutUnit autoRepeatSizeWithGap =
      autoRepeatTracksSize + gapSize * static_cast<int>(autoRepeatTrackListLength);
    uint32 repetitions = 1 + (freeSpace / autoRepeatSizeWithGap).ToUnsigned();
    freeSpace -= autoRepeatSizeWithGap * static_cast<int>(repetitions - 1);
    krys_debug_assert(freeSpace >= 0);

    // Provided the grid container does not have a definite size or max-size in the relevant axis,
    // if the min size is definite then the number of repetitions is the largest possible positive
    // integer that fulfills that minimum requirement.
    if (needsToFulfillMinimumSize && freeSpace)
    {
      ++repetitions;
    }

    return repetitions * autoRepeatTrackListLength;
  }

  layout::IndexRange RenderGrid::AutoRepeatTracksRange(GridTrackSizingDirection direction) const noexcept
  {
    auto insertionPoint = Style().GridTemplateList(direction).AutoRepeatInsertionPoint;
    auto firstAutoRepeatTrack = insertionPoint + CurrentGrid().ExplicitGridStart(direction);
    auto lastAutoRepeatTrack = firstAutoRepeatTrack + CurrentGrid().AutoRepeatTracks(direction);

    return {firstAutoRepeatTrack, lastAutoRepeatTrack};
  }

  UniquePtr<OrderedTrackIndexSet>
    RenderGrid::ComputeEmptyTracksForAutoRepeat(GridTrackSizingDirection direction) const noexcept
  {
    if (GetAutoRepeatType(direction) != AutoRepeatType::Fit)
    {
      return nullptr;
    }

    UniquePtr<OrderedTrackIndexSet> emptyTrackIndexes;
    auto autoRepeatTracksRange = AutoRepeatTracksRange(direction);

    if (!CurrentGrid().HasGridItems())
    {
      emptyTrackIndexes = CreateUnique<OrderedTrackIndexSet>();
      for (auto trackIndex = autoRepeatTracksRange.Begin(); trackIndex < autoRepeatTracksRange.End();
           ++trackIndex)
      {
        emptyTrackIndexes->add(trackIndex);
      }
    }
    else
    {
      for (auto trackIndex = autoRepeatTracksRange.Begin(); trackIndex < autoRepeatTracksRange.End();
           ++trackIndex)
      {
        GridIterator iterator(CurrentGrid(), direction, static_cast<uint32>(trackIndex));
        if (!iterator.NextGridItem())
        {
          if (!emptyTrackIndexes)
          {
            emptyTrackIndexes = CreateUnique<OrderedTrackIndexSet>();
          }
          emptyTrackIndexes->add(trackIndex);
        }
      }
    }
    return emptyTrackIndexes;
  }

  uint32 RenderGrid::ClampAutoRepeatTracks(GridTrackSizingDirection direction,
                                           uint32 autoRepeatTracks) const noexcept
  {
    if (!autoRepeatTracks)
    {
      return 0;
    }

    uint32 insertionPoint = Style().GridTemplateList(direction).AutoRepeatInsertionPoint;
    auto maxTracks = static_cast<uint32>(style::GridPosition::Max());

    if (!insertionPoint)
    {
      return std::min(autoRepeatTracks, maxTracks);
    }

    if (insertionPoint >= maxTracks)
    {
      return 0;
    }

    return std::min(autoRepeatTracks, maxTracks - insertionPoint);
  }

  void RenderGrid::PlaceItems() noexcept
  {
    UpdateLogicalWidth();

    LayoutUnit availableSpaceForColumns = ContentBoxLogicalWidth();
    PlaceItemsOnGrid(availableSpaceForColumns);
  }

  namespace
  {
    GridArea InsertIntoGrid(Grid &grid, RenderBox &gridItem, const GridArea &area) noexcept
    {
      GridArea clamped = grid.Insert(gridItem, area);

      auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem);
      if (!renderGrid)
      {
        return clamped;
      }

      if (renderGrid->IsSubgridRows() || renderGrid->IsSubgridColumns())
      {
        renderGrid->PlaceItems();
      }
      return clamped;
    }
  }

  bool RenderGrid::IsMasonry() const noexcept
  {
    return AreMasonryRows() || AreMasonryColumns();
  }

  // Masonry Spec Section 2
  // "If masonry is specified for both grid-template-columns and grid-template-rows, then the used value for
  // grid-template-columns is none, and thus the inline axis will be the grid axis."
  bool RenderGrid::IsMasonry(GridTrackSizingDirection direction) const noexcept
  {
    // isSubgrid will return false if the masonry axis matches. Need to check style if we are a subgrid
    auto &tracks = Style().GridTemplateList(direction);
    if (auto *parentGrid = DynamicDowncast<RenderGrid>(Parent()); parentGrid && tracks.Subgrid)
    {
      return parentGrid->IsMasonry(direction);
    }
    if (Style().Display() != DisplayType::GridLanes && Style().Display() != DisplayType::InlineGridLanes)
    {
      return false;
    }
    return (direction == GridTrackSizingDirection::Columns) == Style().GetGridAutoFlow().IsColumn();
  }

  // Masonry Spec Section 2.3.1 repeat(auto-fit)
  // "repeat(auto-fit) behaves as repeat(auto-fill) when the other axis is a masonry axis."
  // We need to lie here that we are really an auto-fill instead of an auto-fit.
  AutoRepeatType RenderGrid::GetAutoRepeatType(GridTrackSizingDirection direction) const noexcept
  {
    auto autoRepeatType = Style().GridTemplateList(direction).AutoRepeatType;

    if (IsMasonry(style::OrthogonalDirection(direction)) && autoRepeatType == AutoRepeatType::Fit)
    {
      return AutoRepeatType::Fill;
    }

    return autoRepeatType;
  }

  // FIXME: We shouldn't have to pass the available logical width as argument. The problem is that
  // contentBoxLogicalWidth() does always return a value even if we cannot resolve it like when
  // computing the intrinsic size (preferred widths). That's why we pass the responsibility to the
  // caller who does know whether the available logical width is indefinite or not.
  void RenderGrid::PlaceItemsOnGrid(Maybe<LayoutUnit> availableLogicalWidth) noexcept
  {
    uint32 autoRepeatColumns =
      ComputeAutoRepeatTracksCount(GridTrackSizingDirection::Columns, availableLogicalWidth);
    uint32 autoRepeatRows = ComputeAutoRepeatTracksCount(GridTrackSizingDirection::Rows,
                                                         AvailableLogicalHeightForPercentageComputation());
    autoRepeatRows = ClampAutoRepeatTracks(GridTrackSizingDirection::Rows, autoRepeatRows);
    autoRepeatColumns = ClampAutoRepeatTracks(GridTrackSizingDirection::Columns, autoRepeatColumns);

    if (IsSubgridInParentDirection(GridTrackSizingDirection::Columns)
        || IsSubgridInParentDirection(GridTrackSizingDirection::Rows))
    {
      auto *parent = DynamicDowncast<RenderGrid>(Parent());
      if (parent && parent->CurrentGrid().NeedsItemsPlacement())
      {
        CurrentGrid().SetNeedsItemsPlacement(true);
      }
    }

    if (autoRepeatColumns != CurrentGrid().AutoRepeatTracks(GridTrackSizingDirection::Columns)
        || autoRepeatRows != CurrentGrid().AutoRepeatTracks(GridTrackSizingDirection::Rows) || IsMasonry())
    {
      CurrentGrid().SetNeedsItemsPlacement(true);
      CurrentGrid().SetAutoRepeatTracks(autoRepeatRows, autoRepeatColumns);
    }

    if (!CurrentGrid().NeedsItemsPlacement())
    {
      return;
    }

    krys_debug_assert(!CurrentGrid().HasGridItems());
    PopulateExplicitGridAndOrderIterator();

    List<RenderBox *> autoMajorAxisAutoGridItems;
    List<RenderBox *> specifiedMajorAxisAutoGridItems;
    auto &orderIterator = CurrentGrid().GetOrderIterator();
    for (auto *gridItem = orderIterator.First(); gridItem; gridItem = orderIterator.Next())
    {
      if (orderIterator.ShouldSkipChild(*gridItem))
      {
        continue;
      }

      // Grid items should use the grid area sizes instead of the containing block (grid container)
      // sizes, we initialize the overrides here if needed to ensure it.
      if (!gridItem->GridAreaContentLogicalWidth())
      {
        gridItem->SetGridAreaContentLogicalWidth(0_lu);
      }
      if (!gridItem->GridAreaContentLogicalHeight())
      {
        gridItem->SetGridAreaContentLogicalHeight(null);
      }

      GridArea area = CurrentGrid().GridItemArea(*gridItem);
      CurrentGrid().ClampAreaToSubgridIfNeeded(area);
      if (!area.Rows.IsIndefinite())
      {
        area.Rows.Translate(CurrentGrid().ExplicitGridStart(GridTrackSizingDirection::Rows));
      }
      if (!area.Columns.IsIndefinite())
      {
        area.Columns.Translate(CurrentGrid().ExplicitGridStart(GridTrackSizingDirection::Columns));
      }

      if (area.Rows.IsIndefinite() || area.Columns.IsIndefinite())
      {
        CurrentGrid().SetGridItemArea(*gridItem, area);
        bool majorAxisDirectionIsForColumns =
          AutoPlacementMajorAxisDirection() == GridTrackSizingDirection::Columns;
        if ((majorAxisDirectionIsForColumns && area.Columns.IsIndefinite())
            || (!majorAxisDirectionIsForColumns && area.Rows.IsIndefinite()))
        {
          autoMajorAxisAutoGridItems.push_back(gridItem);
        }
        else
        {
          specifiedMajorAxisAutoGridItems.push_back(gridItem);
        }
        continue;
      }
      (void)InsertIntoGrid(CurrentGrid(), *gridItem, {area.Rows, area.Columns});
    }

    krys_debug_assert(
      !CurrentGrid().HasGridItems()
      || (CurrentGrid().NumTracks(GridTrackSizingDirection::Rows)
            >= style::GridPositionsResolver::ExplicitGridCount(*this, GridTrackSizingDirection::Rows)
          && CurrentGrid().NumTracks(GridTrackSizingDirection::Columns)
               >= style::GridPositionsResolver::ExplicitGridCount(*this, GridTrackSizingDirection::Columns)));

    PlaceSpecifiedMajorAxisItemsOnGrid(specifiedMajorAxisAutoGridItems);
    PlaceAutoMajorAxisItemsOnGrid(autoMajorAxisAutoGridItems);
    // Compute collapsible tracks for auto-fit.
    CurrentGrid().SetAutoRepeatEmptyColumns(
      ComputeEmptyTracksForAutoRepeat(GridTrackSizingDirection::Columns));
    CurrentGrid().SetAutoRepeatEmptyRows(ComputeEmptyTracksForAutoRepeat(GridTrackSizingDirection::Rows));

    CurrentGrid().SetNeedsItemsPlacement(false);

#if KRYS_DEBUG
    for (auto *gridItem = orderIterator.First(); gridItem; gridItem = orderIterator.Next())
    {
      if (orderIterator.ShouldSkipChild(*gridItem))
      {
        continue;
      }

      GridArea area = CurrentGrid().GridItemArea(*gridItem);
      krys_debug_assert(area.Rows.IsTranslatedDefinite() && area.Columns.IsTranslatedDefinite());
    }
#endif
  }

  LayoutUnit RenderGrid::MasonryContentSize() const noexcept
  {
    return _masonryLayout.GridContentSize();
  }

  void RenderGrid::PerformPreLayoutForGridItems(
    const GridTrackSizingAlgorithm &algorithm,
    ShouldUpdateGridAreaLogicalSize shouldUpdateGridAreaLogicalSize) const noexcept
  {
    krys_debug_assert(!algorithm.GetGrid().NeedsItemsPlacement());
    // FIXME: We need a way when we are calling this during intrinsic size computation before performing
    // the layout. Maybe using the PreLayout phase ?
    for (auto *gridItem = FirstChildBox(); gridItem; gridItem = gridItem->NextSiblingBox())
    {
      if (gridItem->IsOutOfFlowPositioned())
      {
        continue;
      }
      // Orthogonal items should be laid out in order to properly compute content-sized tracks that may depend
      // on item's intrinsic size. We also need to properly estimate its grid area size, since it may affect
      // to the baseline shims if such item participates in baseline alignment.
      if (GridLayoutFunctions::IsOrthogonalGridItem(*this, *gridItem))
      {
        UpdateGridAreaLogicalSize(
          *gridItem,
          algorithm.EstimatedGridAreaBreadthForGridItem(*gridItem, GridTrackSizingDirection::Columns),
          algorithm.EstimatedGridAreaBreadthForGridItem(*gridItem, GridTrackSizingDirection::Rows));
        gridItem->LayoutIfNeeded();
        continue;
      }
      // We need to layout the item to know whether it must synthesize its
      // baseline or not, which may imply a cyclic sizing dependency.
      // FIXME: Can we avoid it ?
      // FIXME: We also want to layout baseline aligned items within subgrids, but
      // we don't currently have a way to do that here.
      if (IsBaselineAlignmentForGridItem(*gridItem))
      {
        // FIXME: Hack to fix nested grid text size overflow during re-layouts.
        if (shouldUpdateGridAreaLogicalSize == ShouldUpdateGridAreaLogicalSize::Yes)
        {
          UpdateGridAreaLogicalSize(
            *gridItem,
            algorithm.EstimatedGridAreaBreadthForGridItem(*gridItem, GridTrackSizingDirection::Columns),
            algorithm.EstimatedGridAreaBreadthForGridItem(*gridItem, GridTrackSizingDirection::Rows));
        }
        gridItem->LayoutIfNeeded();
      }
    }
  }

  void RenderGrid::PopulateExplicitGridAndOrderIterator() noexcept
  {
    OrderIteratorPopulator populator(CurrentGrid().GetOrderIterator());
    uint32 explicitRowStart = 0;
    uint32 explicitColumnStart = 0;
    uint32 maximumRowIndex =
      style::GridPositionsResolver::ExplicitGridCount(*this, GridTrackSizingDirection::Rows);
    uint32 maximumColumnIndex =
      style::GridPositionsResolver::ExplicitGridCount(*this, GridTrackSizingDirection::Columns);

    for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
    {
      if (!populator.CollectChild(gridItem))
      {
        continue;
      }

      auto rowPositions = style::GridPositionsResolver::ResolveGridPositionsFromStyle(
        *this, gridItem, GridTrackSizingDirection::Rows);
      if (!IsSubgridRows())
      {
        if (!rowPositions.IsIndefinite())
        {
          explicitRowStart = static_cast<uint32>(
            std::max<int>(static_cast<int>(explicitRowStart), -rowPositions.UntranslatedStartLine()));
          maximumRowIndex = static_cast<uint32>(
            std::max<int>(static_cast<int>(maximumRowIndex), rowPositions.UntranslatedEndLine()));
        }
        else
        {
          // Grow the grid for items with a definite row span, getting the largest such span.
          uint32 spanSize =
            style::GridPositionsResolver::SpanSizeForAutoPlacedItem(gridItem, GridTrackSizingDirection::Rows);
          maximumRowIndex = std::max(maximumRowIndex, spanSize);
        }
      }

      auto columnPositions = style::GridPositionsResolver::ResolveGridPositionsFromStyle(
        *this, gridItem, GridTrackSizingDirection::Columns);
      if (!IsSubgridColumns())
      {
        if (!columnPositions.IsIndefinite())
        {
          explicitColumnStart = static_cast<uint32>(
            std::max<int>(static_cast<int>(explicitColumnStart), -columnPositions.UntranslatedStartLine()));
          maximumColumnIndex = static_cast<uint32>(
            std::max<int>(static_cast<int>(maximumColumnIndex), columnPositions.UntranslatedEndLine()));
        }
        else
        {
          // Grow the grid for items with a definite column span, getting the largest such span.
          uint32 spanSize = style::GridPositionsResolver::SpanSizeForAutoPlacedItem(
            gridItem, GridTrackSizingDirection::Columns);
          maximumColumnIndex = std::max(maximumColumnIndex, spanSize);
        }
      }

      CurrentGrid().SetGridItemArea(gridItem, {rowPositions, columnPositions});
    }

    CurrentGrid().SetExplicitGridStart(explicitRowStart, explicitColumnStart);
    CurrentGrid().EnsureGridSize(maximumRowIndex + explicitRowStart,
                                 maximumColumnIndex + explicitColumnStart);
    CurrentGrid().SetClampingForSubgrid(IsSubgridRows() ? maximumRowIndex : 0,
                                        IsSubgridColumns() ? maximumColumnIndex : 0);
  }

  GridArea RenderGrid::CreateEmptyGridAreaAtSpecifiedPositionsOutsideGrid(
    const RenderBox &gridItem, GridTrackSizingDirection specifiedDirection,
    const GridSpan &specifiedPositions) const noexcept
  {
    auto crossDirection = style::OrthogonalDirection(specifiedDirection);
    auto endOfCrossDirection = CurrentGrid().NumTracks(crossDirection);
    auto crossDirectionSpanSize =
      style::GridPositionsResolver::SpanSizeForAutoPlacedItem(gridItem, crossDirection);
    auto crossDirectionPositions =
      GridSpan::TranslatedDefiniteGridSpan(endOfCrossDirection, endOfCrossDirection + crossDirectionSpanSize);

    return specifiedDirection == GridTrackSizingDirection::Columns
             ? GridArea {crossDirectionPositions, specifiedPositions}
             : GridArea {specifiedPositions, crossDirectionPositions};
  }

  bool RenderGrid::IsPlacedWithinExtrinsicallySizedExplicitTracks(const RenderBox &gridItem) const noexcept
  {
    auto &currentGrid = CurrentGrid();
    if (currentGrid.NeedsItemsPlacement())
    {
      return false;
    }

    auto &gridStyle = Style();
    auto gridItemArea = currentGrid.GridItemArea(gridItem);
    auto &gridColumnSizes = gridStyle.GridTemplateColumns().Sizes;
    if (gridItemArea.Columns.EndLine() > gridColumnSizes.size())
    {
      return false;
    }

    for (auto columnIndex : gridItemArea.Columns)
    {
      if (gridColumnSizes[columnIndex].IsContentSized())
      {
        return false;
      }
    }

    auto &gridRowSizes = gridStyle.GridTemplateRows().Sizes;
    if (gridItemArea.Rows.EndLine() > gridRowSizes.size())
    {
      return false;
    }

    for (auto rowIndex : gridItemArea.Rows)
    {
      if (gridRowSizes[rowIndex].IsContentSized())
      {
        return false;
      }
    }
    return true;
  }

  void RenderGrid::PlaceSpecifiedMajorAxisItemsOnGrid(const List<RenderBox *> &autoGridItems) noexcept
  {
    bool isForColumns = AutoPlacementMajorAxisDirection() == GridTrackSizingDirection::Columns;
    bool isGridAutoFlowDense = Style().GetGridAutoFlow().IsDense();

    // Mapping between the major axis tracks (rows or columns) and the last auto-placed item's position
    // inserted on that track. This is needed to implement "sparse" packing for items locked to a given track.
    // See https://drafts.csswg.org/css-grid-2/#auto-placement-algo
    HashMap<uint32, uint32> minorAxisCursors;

    for (auto *autoGridItem : autoGridItems)
    {
      GridSpan majorAxisPositions =
        CurrentGrid().GridItemSpan(*autoGridItem, AutoPlacementMajorAxisDirection());
      krys_debug_assert(majorAxisPositions.IsTranslatedDefinite());
      krys_debug_assert(
        CurrentGrid().GridItemSpan(*autoGridItem, AutoPlacementMinorAxisDirection()).IsIndefinite());
      uint32 minorAxisSpanSize = style::GridPositionsResolver::SpanSizeForAutoPlacedItem(
        *autoGridItem, AutoPlacementMinorAxisDirection());
      uint32 majorAxisInitialPosition = majorAxisPositions.StartLine();

      auto cursor = minorAxisCursors.find(majorAxisInitialPosition);
      GridIterator iterator(CurrentGrid(), AutoPlacementMajorAxisDirection(), majorAxisPositions.StartLine(),
                            isGridAutoFlowDense || cursor == minorAxisCursors.end() ? 0 : cursor->second);
      auto emptyGridArea = iterator.NextEmptyGridArea(majorAxisPositions.IntegerSpan(), minorAxisSpanSize);
      if (!emptyGridArea)
      {
        emptyGridArea = CreateEmptyGridAreaAtSpecifiedPositionsOutsideGrid(
          *autoGridItem, AutoPlacementMajorAxisDirection(), majorAxisPositions);
      }

      emptyGridArea = InsertIntoGrid(CurrentGrid(), *autoGridItem, *emptyGridArea);

      if (!isGridAutoFlowDense)
      {
        minorAxisCursors.insert_or_assign(majorAxisInitialPosition, isForColumns
                                                                      ? emptyGridArea->Rows.StartLine()
                                                                      : emptyGridArea->Columns.StartLine());
      }
    }
  }

  void RenderGrid::PlaceAutoMajorAxisItemsOnGrid(const List<RenderBox *> &autoGridItems) noexcept
  {
    AutoPlacementCursor autoPlacementCursor = {0, 0};
    bool isGridAutoFlowDense = Style().GetGridAutoFlow().IsDense();

    for (auto *autoGridItem : autoGridItems)
    {
      PlaceAutoMajorAxisItemOnGrid(*autoGridItem, autoPlacementCursor);

      if (isGridAutoFlowDense)
      {
        autoPlacementCursor.first = 0;
        autoPlacementCursor.second = 0;
      }
    }
  }

  void RenderGrid::PlaceAutoMajorAxisItemOnGrid(RenderBox &gridItem,
                                                AutoPlacementCursor &autoPlacementCursor) noexcept
  {
    krys_debug_assert(CurrentGrid().GridItemSpan(gridItem, AutoPlacementMajorAxisDirection()).IsIndefinite());
    uint32 majorAxisSpanSize =
      style::GridPositionsResolver::SpanSizeForAutoPlacedItem(gridItem, AutoPlacementMajorAxisDirection());

    const uint32 endOfMajorAxis = CurrentGrid().NumTracks(AutoPlacementMajorAxisDirection());
    uint32 majorAxisAutoPlacementCursor =
      AutoPlacementMajorAxisDirection() == GridTrackSizingDirection::Columns ? autoPlacementCursor.second
                                                                             : autoPlacementCursor.first;
    uint32 minorAxisAutoPlacementCursor =
      AutoPlacementMajorAxisDirection() == GridTrackSizingDirection::Columns ? autoPlacementCursor.first
                                                                             : autoPlacementCursor.second;

    auto emptyGridArea = Maybe<GridArea> {};
    GridSpan minorAxisPositions = CurrentGrid().GridItemSpan(gridItem, AutoPlacementMinorAxisDirection());
    if (minorAxisPositions.IsTranslatedDefinite())
    {
      // Move to the next track in major axis if initial position in minor axis is before auto-placement
      // cursor.
      if (minorAxisPositions.StartLine() < minorAxisAutoPlacementCursor)
      {
        majorAxisAutoPlacementCursor++;
      }

      if (majorAxisAutoPlacementCursor < endOfMajorAxis)
      {
        GridIterator iterator(CurrentGrid(), AutoPlacementMinorAxisDirection(),
                              minorAxisPositions.StartLine(), majorAxisAutoPlacementCursor);
        emptyGridArea = iterator.NextEmptyGridArea(minorAxisPositions.IntegerSpan(), majorAxisSpanSize);
      }

      if (!emptyGridArea)
      {
        emptyGridArea = CreateEmptyGridAreaAtSpecifiedPositionsOutsideGrid(
          gridItem, AutoPlacementMinorAxisDirection(), minorAxisPositions);
      }
    }
    else
    {
      uint32 minorAxisSpanSize =
        style::GridPositionsResolver::SpanSizeForAutoPlacedItem(gridItem, AutoPlacementMinorAxisDirection());

      for (uint32 majorAxisIndex = majorAxisAutoPlacementCursor; majorAxisIndex < endOfMajorAxis;
           ++majorAxisIndex)
      {
        GridIterator iterator(CurrentGrid(), AutoPlacementMajorAxisDirection(), majorAxisIndex,
                              minorAxisAutoPlacementCursor);
        emptyGridArea = iterator.NextEmptyGridArea(majorAxisSpanSize, minorAxisSpanSize);

        if (emptyGridArea)
        {
          // Check that it fits in the minor axis direction, as we shouldn't grow in that direction here (it
          // was already managed in populateExplicitGridAndOrderIterator()).
          uint32 minorAxisFinalPositionIndex =
            AutoPlacementMinorAxisDirection() == GridTrackSizingDirection::Columns
              ? emptyGridArea->Columns.EndLine()
              : emptyGridArea->Rows.EndLine();
          const uint32 endOfMinorAxis = CurrentGrid().NumTracks(AutoPlacementMinorAxisDirection());
          if (minorAxisFinalPositionIndex <= endOfMinorAxis)
          {
            break;
          }

          // Discard empty grid area as it does not fit in the minor axis direction.
          // We don't need to create a new empty grid area yet as we might find a valid one in the next
          // iteration.
          emptyGridArea = null;
        }

        // As we're moving to the next track in the major axis we should reset the auto-placement cursor in
        // the minor axis.
        minorAxisAutoPlacementCursor = 0;
      }

      if (!emptyGridArea)
      {
        emptyGridArea = CreateEmptyGridAreaAtSpecifiedPositionsOutsideGrid(
          gridItem, AutoPlacementMinorAxisDirection(),
          GridSpan::TranslatedDefiniteGridSpan(0, minorAxisSpanSize));
      }
    }

    emptyGridArea = InsertIntoGrid(CurrentGrid(), gridItem, *emptyGridArea);
    autoPlacementCursor.first = emptyGridArea->Rows.StartLine();
    autoPlacementCursor.second = emptyGridArea->Columns.StartLine();
  }

  GridTrackSizingDirection RenderGrid::AutoPlacementMajorAxisDirection() const noexcept
  {
    return Style().GetGridAutoFlow().IsColumn() ? GridTrackSizingDirection::Columns
                                                : GridTrackSizingDirection::Rows;
  }

  GridTrackSizingDirection RenderGrid::AutoPlacementMinorAxisDirection() const noexcept
  {
    return style::OrthogonalDirection(AutoPlacementMajorAxisDirection());
  }

  void RenderGrid::SetNeedsItemPlacement(SubgridDidChange subgridDidChange) noexcept
  {
    // The cached heights are keyed by address, so they go with the items they were computed for.
    _intrinsicLogicalHeightsForRowSizingFirstPass.reset();

    if (CurrentGrid().NeedsItemsPlacement())
    {
      return;
    }

    CurrentGrid().SetNeedsItemsPlacement(true);

    auto *currentChild = this;
    while (currentChild
           && (subgridDidChange == SubgridDidChange::Yes || currentChild->IsSubgridRows()
               || currentChild->IsSubgridColumns()))
    {
      currentChild = DynamicDowncast<RenderGrid>(currentChild->Parent());
      if (currentChild)
      {
        currentChild->CurrentGrid().SetNeedsItemsPlacement(true);
      }
      subgridDidChange = SubgridDidChange::No;
    }
  }

  List<LayoutUnit> RenderGrid::TrackSizesForComputedStyle(GridTrackSizingDirection direction) const noexcept
  {
    const auto &positions = Positions(direction);
    auto numPositions = positions.size();

    List<LayoutUnit> tracks;
    if (numPositions < 2)
    {
      return tracks;
    }

    krys_debug_assert(!CurrentGrid().NeedsItemsPlacement());
    auto offsetBetweenTracks = OffsetBetweenTracks(direction).DistributionOffset;
    bool hasCollapsedTracks = CurrentGrid().HasAutoRepeatEmptyTracks(direction);
    auto gap = !hasCollapsedTracks ? GridGap(direction) : 0_lu;
    tracks.reserve(numPositions - 1);
    for (size_t i = 0; i < numPositions - 2; ++i)
    {
      tracks.push_back(positions[i + 1] - positions[i] - offsetBetweenTracks - gap);
    }
    tracks.push_back(positions[numPositions - 1] - positions[numPositions - 2]);

    if (!hasCollapsedTracks)
    {
      return tracks;
    }

    size_t remainingEmptyTracks = CurrentGrid().AutoRepeatEmptyTracks(direction)->size();
    size_t lastLine = tracks.size();
    gap = GridGap(direction);
    for (size_t i = 1; i < lastLine; ++i)
    {
      if (CurrentGrid().IsEmptyAutoRepeatTrack(direction, static_cast<uint32>(i - 1)))
      {
        --remainingEmptyTracks;
      }
      else
      {
        // Remove the gap between consecutive non empty tracks. Remove it also just once for an
        // arbitrary number of empty tracks between two non empty ones.
        bool allRemainingTracksAreEmpty = remainingEmptyTracks == (lastLine - i);
        if (!allRemainingTracksAreEmpty
            || !CurrentGrid().IsEmptyAutoRepeatTrack(direction, static_cast<uint32>(i)))
        {
          tracks[i - 1] -= gap;
        }
      }
    }

    return tracks;
  }

  namespace
  {
    KRYS_NODISCARD const StyleContentAlignmentData &ContentAlignmentNormalBehaviorGrid() noexcept
    {
      static const StyleContentAlignmentData normalBehavior = {ContentPosition::Normal,
                                                               ContentDistribution::Stretch};
      return normalBehavior;
    }

    KRYS_NODISCARD bool OverrideSizeChanged(const RenderBox &gridItem, GridTrackSizingDirection direction,
                                            Maybe<LayoutUnit> width, Maybe<LayoutUnit> height) noexcept
    {
      if (direction == GridTrackSizingDirection::Columns)
      {
        if (auto gridAreaContentLogicalWidth = gridItem.GridAreaContentLogicalWidth())
        {
          return *gridAreaContentLogicalWidth != width;
        }
        return true;
      }
      if (auto gridAreaContentLogicalHeight = gridItem.GridAreaContentLogicalHeight())
      {
        return *gridAreaContentLogicalHeight != height;
      }
      return true;
    }

    KRYS_NODISCARD bool HasRelativeBlockAxisSize(const RenderGrid &grid, const RenderBox &gridItem) noexcept
    {
      return GridLayoutFunctions::IsOrthogonalGridItem(grid, gridItem)
               ? gridItem.HasRelativeLogicalWidth() || gridItem.Style().LogicalWidth().IsAuto()
               : gridItem.HasRelativeLogicalHeight();
    }

    KRYS_NODISCARD LayoutUnit ComputeOverflowAlignmentOffset(OverflowAlignment overflow, LayoutUnit trackSize,
                                                             LayoutUnit gridItemSize) noexcept
    {
      LayoutUnit offset = trackSize - gridItemSize;
      switch (overflow)
      {
        case OverflowAlignment::Safe:
          // If overflow is 'safe', we have to make sure we don't overflow the 'start'
          // edge (potentially cause some data loss as the overflow is unreachable).
          return std::max<LayoutUnit>(0_lu, offset);
        case OverflowAlignment::Unsafe:
        case OverflowAlignment::Default:
          // If we overflow our alignment container and overflow is 'true' (default), we
          // ignore the overflow and just return the value regardless (which may cause data
          // loss as we overflow the 'start' edge).
          return offset;
      }

      krys_debug_assert(false);
      return 0_lu;
    }

    KRYS_NODISCARD Pair<OverflowAlignment, ContentPosition>
      ResolveContentDistributionFallback(ContentDistribution distribution) noexcept
    {
      switch (distribution)
      {
        case ContentDistribution::SpaceBetween: return {OverflowAlignment::Default, ContentPosition::Start};
        case ContentDistribution::SpaceAround:  return {OverflowAlignment::Safe, ContentPosition::Center};
        case ContentDistribution::SpaceEvenly:  return {OverflowAlignment::Safe, ContentPosition::Center};
        case ContentDistribution::Stretch:      return {OverflowAlignment::Default, ContentPosition::Start};
        case ContentDistribution::Default:      return {OverflowAlignment::Default, ContentPosition::Normal};
      }

      krys_debug_assert(false);
      return {OverflowAlignment::Default, ContentPosition::Normal};
    }
  }

  void RenderGrid::UpdateGridAreaLogicalSize(RenderBox &gridItem, Maybe<LayoutUnit> width,
                                             Maybe<LayoutUnit> height) const noexcept
  {
    // Because the grid area cannot be styled, we don't need to adjust
    // the grid breadth to account for 'box-sizing'.
    bool gridAreaWidthChanged =
      OverrideSizeChanged(gridItem, GridTrackSizingDirection::Columns, width, height);
    bool gridAreaHeightChanged = OverrideSizeChanged(gridItem, GridTrackSizingDirection::Rows, width, height);
    if (gridAreaWidthChanged || (gridAreaHeightChanged && HasRelativeBlockAxisSize(*this, gridItem)))
    {
      gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }

    gridItem.SetGridAreaContentLogicalWidth(width);
    gridItem.SetGridAreaContentLogicalHeight(height);
  }

  void RenderGrid::UpdateGridAreaForAspectRatioItems(const List<RenderBox *> &autoGridItems,
                                                     GridLayoutState &gridLayoutState) noexcept
  {
    PopulateGridPositionsForDirection(_trackSizingAlgorithm, GridTrackSizingDirection::Columns);
    PopulateGridPositionsForDirection(_trackSizingAlgorithm, GridTrackSizingDirection::Rows);

    for (auto *autoGridItem : autoGridItems)
    {
      UpdateGridAreaLogicalSize(
        *autoGridItem,
        GridAreaBreadthForGridItemIncludingAlignmentOffsets(*autoGridItem, GridTrackSizingDirection::Columns),
        GridAreaBreadthForGridItemIncludingAlignmentOffsets(*autoGridItem, GridTrackSizingDirection::Rows));
      // For an item with aspect-ratio, if it has stretch alignment that stretches to the definite row, we
      // also need to transfer the size before laying out the grid item.
      if (autoGridItem->HasStretchedLogicalHeight())
      {
        ApplyStretchAlignmentToGridItemIfNeeded(*autoGridItem, gridLayoutState);
      }
    }
  }

  void RenderGrid::LayoutGridItems(GridLayoutState &gridLayoutState) noexcept
  {
    PopulateGridPositionsForDirection(_trackSizingAlgorithm, GridTrackSizingDirection::Columns);
    PopulateGridPositionsForDirection(_trackSizingAlgorithm, GridTrackSizingDirection::Rows);

    for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
    {
      if (CurrentGrid().GetOrderIterator().ShouldSkipChild(gridItem))
      {
        if (gridItem.IsOutOfFlowPositioned())
        {
          PrepareGridItemForPositionedLayout(gridItem);
        }
        continue;
      }

      auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem);
      if (renderGrid && (renderGrid->IsSubgridColumns() || renderGrid->IsSubgridRows()))
      {
        gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }

      // Setting the definite grid area's sizes. It may imply that the
      // item must perform a layout if its area differs from the one
      // used during the track sizing algorithm.
      UpdateGridAreaLogicalSize(
        gridItem,
        GridAreaBreadthForGridItemIncludingAlignmentOffsets(gridItem, GridTrackSizingDirection::Columns),
        GridAreaBreadthForGridItemIncludingAlignmentOffsets(gridItem, GridTrackSizingDirection::Rows));

      // Stretching logic might force a grid item layout, so we need to run it before the layoutIfNeeded
      // call to avoid unnecessary relayouts. This might imply that grid item margins, needed to correctly
      // determine the available space before stretching, are not set yet.
      ApplyStretchAlignmentToGridItemIfNeeded(gridItem, gridLayoutState);
      ApplySubgridStretchAlignmentToGridItemIfNeeded(gridItem);

      gridItem.LayoutIfNeeded();

      // We need pending layouts to be done in order to compute auto-margins properly.
      GridLayoutFunctions::UpdateAutoMarginsIfNeeded(gridItem, GetWritingMode());

      SetLogicalPositionForGridItem(gridItem);

      // FIXME: Repaint the grid item if it moved (repaintDuringLayoutIfMoved), once there's painting.
    }
  }

  void RenderGrid::LayoutMasonryItems(GridLayoutState &gridLayoutState) noexcept
  {
    // Masonry items are laid out as grid items, at the positions masonry placement gave them.
    LayoutGridItems(gridLayoutState);
  }

  void RenderGrid::PrepareGridItemForPositionedLayout(RenderBox &gridItem) noexcept
  {
    krys_debug_assert(gridItem.IsOutOfFlowPositioned());
    gridItem.ContainingBlock()->AddOutOfFlowBox(gridItem);

    auto *gridItemLayer = gridItem.Layer();
    // Static position of a positioned grid item should use the content-box
    // (https://drafts.csswg.org/css-grid/#static-position).
    gridItemLayer->SetStaticInlinePosition(BorderAndPaddingStart());
    gridItemLayer->SetStaticBlockPosition(BorderAndPaddingBefore());
  }

  bool RenderGrid::HasStaticPositionForGridItem(const RenderBox &gridItem,
                                                GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Columns
             ? gridItem.Style().HasStaticInlinePosition(IsHorizontalWritingMode())
             : gridItem.Style().HasStaticBlockPosition(IsHorizontalWritingMode());
  }

  void RenderGrid::LayoutOutOfFlowBox(RenderBox &gridItem, RelayoutChildren relayoutChildren,
                                      bool fixedPositionObjectsOnly) noexcept
  {
    // FIXME: Content skipped by content-visibility (isSkippedContentRootForLayout), once layout skips
    // content.

    LayoutRange columnBreadth = GridAreaRangeForOutOfFlow(gridItem, GridTrackSizingDirection::Columns);
    LayoutRange rowBreadth = GridAreaRangeForOutOfFlow(gridItem, GridTrackSizingDirection::Rows);

    gridItem.SetGridAreaContentLogicalWidth(columnBreadth.Size());
    gridItem.SetGridAreaContentLogicalHeight(rowBreadth.Size());

    // Mark for layout as we're resetting the position before and we relay in generic layout logic
    // for positioned items in order to get the offsets properly resolved.
    gridItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);

    RenderBlock::LayoutOutOfFlowBox(gridItem, relayoutChildren, fixedPositionObjectsOnly);
  }

  LayoutUnit RenderGrid::GridAreaBreadthForGridItemIncludingAlignmentOffsets(
    const RenderBox &gridItem, GridTrackSizingDirection direction) const noexcept
  {
    if (direction == GridTrackSizingDirection::Rows)
    {
      if (AreMasonryRows())
      {
        return IsHorizontalWritingMode() ? gridItem.Height() + gridItem.VerticalMarginExtent()
                                         : gridItem.Width() + gridItem.HorizontalMarginExtent();
      }
    }
    else if (AreMasonryColumns())
    {
      return IsHorizontalWritingMode() ? gridItem.Width() + gridItem.HorizontalMarginExtent()
                                       : gridItem.Height() + gridItem.VerticalMarginExtent();
    }

    // We need the cached value when available because Content Distribution alignment properties
    // may have some influence in the final grid area breadth.
    const auto &tracks = _trackSizingAlgorithm.GetTracks(direction);
    const auto &span = CurrentGrid().GridItemSpan(gridItem, direction);
    const auto &positions = Positions(direction);

    auto initialTrackPosition = positions[span.StartLine()];
    auto finalTrackPosition = positions[span.EndLine() - 1];

    // Track Positions vector stores the 'start' grid line of each track, so we have to add last track's
    // baseSize.
    return finalTrackPosition - initialTrackPosition + tracks[span.EndLine() - 1]->BaseSize();
  }

  void RenderGrid::PopulateGridPositionsForDirection(const GridTrackSizingAlgorithm &algorithm,
                                                     GridTrackSizingDirection direction) noexcept
  {
    // Since we add alignment offsets and track gutters, grid lines are not always adjacent. Hence, we will
    // have to assume from now on that we just store positions of the initial grid lines of each track, except
    // the last one, which is the only one considered as a final grid line of a track.

    // The grid container's frame elements (border, padding and <content-position> offset) are sensible to the
    // inline-axis flow direction. However, column lines positions are 'direction' unaware. This
    // simplification allows us to use the same indexes to identify the columns independently on the
    // inline-axis direction.
    bool isRowAxis = direction == GridTrackSizingDirection::Columns;
    auto &tracks = algorithm.GetTracks(direction);
    auto numberOfTracks = static_cast<uint32>(tracks.size());
    uint32 numberOfLines = numberOfTracks + 1;
    uint32 lastLine = numberOfLines - 1;
    bool hasCollapsedTracks = CurrentGrid().HasAutoRepeatEmptyTracks(direction);
    size_t numberOfCollapsedTracks =
      hasCollapsedTracks ? CurrentGrid().AutoRepeatEmptyTracks(direction)->size() : 0;
    const auto &offsetBetweenTracks = OffsetBetweenTracks(direction);
    auto &positions = MutablePositions(direction);
    positions.resize(numberOfLines);

    auto borderAndPadding = isRowAxis ? BorderAndPaddingStart() : BorderAndPaddingBefore();

    positions[0] = borderAndPadding + offsetBetweenTracks.PositionOffset;
    if (numberOfLines > 1)
    {
      // If we have collapsed tracks we just ignore gaps here and add them later as we might not
      // compute the gap between two consecutive tracks without examining the surrounding ones.
      LayoutUnit gap = !hasCollapsedTracks ? GridGap(direction) : 0_lu;
      uint32 nextToLastLine = numberOfLines - 2;

      for (uint32 i = 0; i < nextToLastLine; ++i)
      {
        positions[i + 1] =
          positions[i] + offsetBetweenTracks.DistributionOffset + tracks[i]->UnclampedBaseSize() + gap;
      }
      positions[lastLine] = positions[nextToLastLine] + tracks[nextToLastLine]->UnclampedBaseSize();

      if (IsMasonry(direction))
      {
        positions[lastLine] = _masonryLayout.GridContentSize() + positions[0];
      }

      // Adjust collapsed gaps. Collapsed tracks cause the surrounding gutters to collapse (they
      // coincide exactly) except on the edges of the grid where they become 0.
      if (hasCollapsedTracks)
      {
        gap = GridGap(direction);
        size_t remainingEmptyTracks = numberOfCollapsedTracks;
        LayoutUnit offsetAccumulator;
        LayoutUnit gapAccumulator;
        for (uint32 i = 1; i < lastLine; ++i)
        {
          if (CurrentGrid().IsEmptyAutoRepeatTrack(direction, i - 1))
          {
            --remainingEmptyTracks;
            offsetAccumulator += offsetBetweenTracks.DistributionOffset;
          }
          else
          {
            // Add gap between consecutive non empty tracks. Add it also just once for an
            // arbitrary number of empty tracks between two non empty ones.
            bool allRemainingTracksAreEmpty = remainingEmptyTracks == (lastLine - i);
            if (!allRemainingTracksAreEmpty || !CurrentGrid().IsEmptyAutoRepeatTrack(direction, i))
            {
              gapAccumulator += gap;
            }
          }
          positions[i] += gapAccumulator - offsetAccumulator;
        }
        positions[lastLine] += gapAccumulator - offsetAccumulator;
      }
    }
  }

  StyleSelfAlignmentData RenderGrid::AlignSelfForGridItem(const RenderBox &gridItem,
                                                          StretchingMode stretchingMode,
                                                          const RenderStyle *gridStyle) const noexcept
  {
    auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem);
    if (renderGrid && renderGrid->IsSubgridInParentDirection(GridTrackSizingDirection::Rows))
    {
      return {ItemPosition::Stretch, OverflowAlignment::Default};
    }

    auto normalBehavior =
      stretchingMode == StretchingMode::Any ? SelfAlignmentNormalBehavior(&gridItem) : ItemPosition::Normal;

    if (!gridItem.Style().GetAlignSelf().IsAuto())
    {
      return gridItem.Style().GetAlignSelf().Resolve(normalBehavior);
    }

    if (!gridStyle)
    {
      gridStyle = &Style();
    }
    return gridStyle->GetAlignItems().Resolve(normalBehavior);
  }

  StyleSelfAlignmentData RenderGrid::JustifySelfForGridItem(const RenderBox &gridItem,
                                                            StretchingMode stretchingMode,
                                                            const RenderStyle *gridStyle) const noexcept
  {
    auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem);
    if (renderGrid && renderGrid->IsSubgridInParentDirection(GridTrackSizingDirection::Columns))
    {
      return {ItemPosition::Stretch, OverflowAlignment::Default};
    }

    auto normalBehavior =
      stretchingMode == StretchingMode::Any ? SelfAlignmentNormalBehavior(&gridItem) : ItemPosition::Normal;

    if (!gridItem.Style().GetJustifySelf().IsAuto())
    {
      return gridItem.Style().GetJustifySelf().Resolve(normalBehavior);
    }

    if (!gridStyle)
    {
      gridStyle = &Style();
    }
    return gridStyle->GetJustifyItems().Resolve(normalBehavior);
  }

  bool RenderGrid::AspectRatioPrefersInline(const RenderBox &gridItem, bool blockFlowIsColumnAxis) noexcept
  {
    if (!gridItem.Style().HasAspectRatio())
    {
      return false;
    }
    bool hasExplicitInlineStretch =
      JustifySelfForGridItem(gridItem, StretchingMode::Explicit).Position() == ItemPosition::Stretch;
    bool hasExplicitBlockStretch =
      AlignSelfForGridItem(gridItem, StretchingMode::Explicit).Position() == ItemPosition::Stretch;
    if (!blockFlowIsColumnAxis)
    {
      std::swap(hasExplicitInlineStretch, hasExplicitBlockStretch);
    }
    return !hasExplicitBlockStretch;
  }

  // FIXME: This logic is shared by RenderFlexibleBox, so it should be moved to RenderBox.
  void RenderGrid::ApplyStretchAlignmentToGridItemIfNeeded(RenderBox &gridItem,
                                                           GridLayoutState &gridLayoutState) noexcept
  {
    krys_debug_assert(gridItem.GridAreaContentLogicalHeight());
    krys_debug_assert(gridItem.GridAreaContentLogicalWidth());

    // We clear height and width override values because we will decide now whether it's allowed or
    // not, evaluating the conditions which might have changed since the old values were set.
    gridItem.ClearOverridingSize();

    auto gridItemBlockDirection =
      GridLayoutFunctions::FlowAwareDirectionForGridItem(*this, gridItem, GridTrackSizingDirection::Rows);
    auto gridItemInlineDirection =
      GridLayoutFunctions::FlowAwareDirectionForGridItem(*this, gridItem, GridTrackSizingDirection::Columns);
    bool blockFlowIsColumnAxis = gridItemBlockDirection == GridTrackSizingDirection::Rows;
    bool allowedToStretchGridItemBlockSize =
      blockFlowIsColumnAxis ? GridLayoutFunctions::AllowedToStretchGridItemAlongColumnAxis(
                                gridItem, AlignSelfForGridItem(gridItem).Position(), GetWritingMode())
                            : GridLayoutFunctions::AllowedToStretchGridItemAlongRowAxis(
                                gridItem, JustifySelfForGridItem(gridItem).Position(), GetWritingMode());
    if (allowedToStretchGridItemBlockSize && !AspectRatioPrefersInline(gridItem, blockFlowIsColumnAxis))
    {
      auto overridingContainingBlockContentSizeForGridItem =
        GridLayoutFunctions::OverridingContainingBlockContentSizeForGridItem(gridItem,
                                                                             gridItemBlockDirection);
      krys_debug_assert(overridingContainingBlockContentSizeForGridItem
                        && *overridingContainingBlockContentSizeForGridItem);
      LayoutUnit stretchedLogicalHeight =
        GridLayoutFunctions::AvailableAlignmentSpaceForGridItemBeforeStretching(
          *this, overridingContainingBlockContentSizeForGridItem->value(), gridItem,
          GridTrackSizingDirection::Rows);
      LayoutUnit desiredLogicalHeight = gridItem.ConstrainLogicalHeightByMinMax(stretchedLogicalHeight, null);
      gridItem.SetOverridingBorderBoxLogicalHeight(desiredLogicalHeight);

      auto itemNeedsRelayoutForStretchAlignment = [&]()
      {
        if (desiredLogicalHeight != gridItem.LogicalHeight())
        {
          return true;
        }

        if (CanSetColumnAxisStretchRequirementForItem(gridItem))
        {
          return gridLayoutState.ContainsLayoutRequirementForGridItem(
            gridItem, ItemLayoutRequirement::NeedsColumnAxisStretchAlignment);
        }

        auto *block = DynamicDowncast<RenderBlock>(gridItem);
        return block && block->HasPercentHeightDescendants();
      }();
      // Checking the logical-height of a grid item isn't enough. Setting an override logical-height
      // changes the definiteness, resulting in percentages to resolve differently.
      //
      // FIXME: Can avoid laying out here in some cases. See https://webkit.org/b/87905.
      if (itemNeedsRelayoutForStretchAlignment)
      {
        gridItem.SetLogicalHeight(0_lu);
        gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
    }
    else if (!allowedToStretchGridItemBlockSize
             && GridLayoutFunctions::AllowedToStretchGridItemAlongRowAxis(
               gridItem, JustifySelfForGridItem(gridItem).Position(), GetWritingMode()))
    {
      auto overridingContainingBlockContentSizeForGridItem =
        GridLayoutFunctions::OverridingContainingBlockContentSizeForGridItem(gridItem,
                                                                             gridItemInlineDirection);
      krys_debug_assert(overridingContainingBlockContentSizeForGridItem
                        && *overridingContainingBlockContentSizeForGridItem);
      LayoutUnit stretchedLogicalWidth =
        GridLayoutFunctions::AvailableAlignmentSpaceForGridItemBeforeStretching(
          *this, overridingContainingBlockContentSizeForGridItem->value(), gridItem,
          GridTrackSizingDirection::Columns);
      LayoutUnit desiredLogicalWidth =
        gridItem.ConstrainLogicalWidthByMinMax(stretchedLogicalWidth, ContentBoxWidth(), *this);
      gridItem.SetOverridingBorderBoxLogicalWidth(desiredLogicalWidth);
      if (desiredLogicalWidth != gridItem.LogicalWidth())
      {
        gridItem.SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
    }
  }

  void RenderGrid::ApplySubgridStretchAlignmentToGridItemIfNeeded(RenderBox &gridItem) noexcept
  {
    auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem);
    if (!renderGrid)
    {
      return;
    }

    if (renderGrid->IsSubgrid(GridTrackSizingDirection::Rows))
    {
      auto gridItemBlockDirection =
        GridLayoutFunctions::FlowAwareDirectionForGridItem(*this, gridItem, GridTrackSizingDirection::Rows);
      auto overridingContainingBlockContentSizeForGridItem =
        GridLayoutFunctions::OverridingContainingBlockContentSizeForGridItem(gridItem,
                                                                             gridItemBlockDirection);
      krys_debug_assert(overridingContainingBlockContentSizeForGridItem
                        && *overridingContainingBlockContentSizeForGridItem);
      auto stretchedLogicalHeight = GridLayoutFunctions::AvailableAlignmentSpaceForGridItemBeforeStretching(
        *this, overridingContainingBlockContentSizeForGridItem->value(), gridItem,
        GridTrackSizingDirection::Rows);
      gridItem.SetOverridingBorderBoxLogicalHeight(stretchedLogicalHeight);
    }

    if (renderGrid->IsSubgrid(GridTrackSizingDirection::Columns))
    {
      auto gridItemInlineDirection = GridLayoutFunctions::FlowAwareDirectionForGridItem(
        *this, gridItem, GridTrackSizingDirection::Columns);
      auto overridingContainingBlockContentSizeForGridItem =
        GridLayoutFunctions::OverridingContainingBlockContentSizeForGridItem(gridItem,
                                                                             gridItemInlineDirection);
      krys_debug_assert(overridingContainingBlockContentSizeForGridItem
                        && *overridingContainingBlockContentSizeForGridItem);
      auto stretchedLogicalWidth = GridLayoutFunctions::AvailableAlignmentSpaceForGridItemBeforeStretching(
        *this, overridingContainingBlockContentSizeForGridItem->value(), gridItem,
        GridTrackSizingDirection::Columns);
      gridItem.SetOverridingBorderBoxLogicalWidth(stretchedLogicalWidth);
    }
  }

  bool RenderGrid::IsChildEligibleForMarginTrim(style::MarginTrimSide marginTrimSide,
                                                const RenderBox &gridItem) const noexcept
  {
    krys_debug_assert(Style().GetMarginTrim().Contains(marginTrimSide));

    auto isTrimmingBlockDirection = marginTrimSide == style::MarginTrimSide::BlockStart
                                    || marginTrimSide == style::MarginTrimSide::BlockEnd;
    auto itemGridSpan =
      isTrimmingBlockDirection
        ? CurrentGrid().GridItemSpanIgnoringCollapsedTracks(gridItem, GridTrackSizingDirection::Rows)
        : CurrentGrid().GridItemSpanIgnoringCollapsedTracks(gridItem, GridTrackSizingDirection::Columns);
    switch (marginTrimSide)
    {
      case style::MarginTrimSide::BlockStart:
      case style::MarginTrimSide::InlineStart: return !itemGridSpan.StartLine();
      case style::MarginTrimSide::BlockEnd:
        return itemGridSpan.EndLine() == CurrentGrid().NumTracks(GridTrackSizingDirection::Rows);
      case style::MarginTrimSide::InlineEnd:
        return itemGridSpan.EndLine() == CurrentGrid().NumTracks(GridTrackSizingDirection::Columns);
    }
    krys_debug_assert(false);
    return false;
  }

  bool RenderGrid::IsBaselineAlignmentForGridItem(const RenderBox &gridItem) const noexcept
  {
    return IsBaselineAlignmentForGridItem(gridItem, GridTrackSizingDirection::Columns)
           || IsBaselineAlignmentForGridItem(gridItem, GridTrackSizingDirection::Rows);
  }

  bool
    RenderGrid::IsBaselineAlignmentForGridItem(const RenderBox &gridItem,
                                               GridTrackSizingDirection alignmentContextType) const noexcept
  {
    if (gridItem.IsOutOfFlowPositioned())
    {
      return false;
    }
    auto align = SelfAlignmentForGridItem(alignmentContextType, gridItem).Position();
    bool hasAutoMargins = alignmentContextType == GridTrackSizingDirection::Rows
                            ? GridLayoutFunctions::HasAutoMarginsInColumnAxis(gridItem, GetWritingMode())
                            : GridLayoutFunctions::HasAutoMarginsInRowAxis(gridItem, GetWritingMode());
    return IsBaselinePosition(align) && !hasAutoMargins;
  }

  Maybe<LayoutUnit> RenderGrid::FirstLineBaseline() const noexcept
  {
    if ((IsWritingModeRoot() && !IsFlexItem()) || !CurrentGrid().HasGridItems()
        || ShouldApplyLayoutContainment())
    {
      return {};
    }

    // Finding the first grid item in grid order.
    auto *baselineGridItem = BaselineGridItem(ItemPosition::Baseline);

    if (!baselineGridItem)
    {
      return {};
    }

    auto baseline = Maybe<LayoutUnit> {};
    if (!GridLayoutFunctions::IsOrthogonalGridItem(*this, *baselineGridItem))
    {
      baseline = baselineGridItem->FirstLineBaseline();
    }

    if (!baseline)
    {
      // We take border-box's bottom if no valid baseline.
      // FIXME: We should pass |direction| into firstLineBaseline and stop bailing out if we're a writing
      // mode root. This would also fix some cases where the grid is orthogonal to its container.
      auto gridWritingMode = Style().GetWritingMode();
      auto dominantBaseline = BaselineAlignmentState::DominantBaseline(gridWritingMode);
      auto direction = IsHorizontalWritingMode() ? LineDirection::Horizontal : LineDirection::Vertical;
      baseline = BaselineAlignmentState::SynthesizedBaseline(
        *baselineGridItem, dominantBaseline, gridWritingMode, direction, BaselineSynthesisEdge::BorderBox);
    }
    // Subpixel inline layout is off: the item's top is snapped to an integer.
    return LayoutUnit(LogicalTopForChild(*baselineGridItem).ToInt()) + *baseline;
  }

  Maybe<LayoutUnit> RenderGrid::LastLineBaseline() const noexcept
  {
    if (IsWritingModeRoot() || !CurrentGrid().HasGridItems() || ShouldApplyLayoutContainment())
    {
      return {};
    }

    auto *baselineGridItem = BaselineGridItem(ItemPosition::LastBaseline);
    if (!baselineGridItem)
    {
      return {};
    }

    auto baseline = Maybe<LayoutUnit> {};
    if (!GridLayoutFunctions::IsOrthogonalGridItem(*this, *baselineGridItem))
    {
      baseline = baselineGridItem->LastLineBaseline();
    }

    if (!baseline)
    {
      auto direction = IsHorizontalWritingMode() ? LineDirection::Horizontal : LineDirection::Vertical;
      auto gridWritingMode = Style().GetWritingMode();
      auto dominantBaseline = BaselineAlignmentState::DominantBaseline(gridWritingMode);
      baseline = BaselineAlignmentState::SynthesizedBaseline(
        *baselineGridItem, dominantBaseline, gridWritingMode, direction, BaselineSynthesisEdge::BorderBox);
    }
    // Subpixel inline layout is off: the item's top is snapped to an integer.
    return LayoutUnit(LogicalTopForChild(*baselineGridItem).ToInt()) + *baseline;
  }

  const RenderBox *RenderGrid::BaselineGridItem(ItemPosition alignment) const noexcept
  {
    krys_debug_assert(alignment == ItemPosition::Baseline || alignment == ItemPosition::LastBaseline);
    const RenderBox *baselineGridItem = nullptr;
    uint32 numColumns = CurrentGrid().NumTracks(GridTrackSizingDirection::Columns);
    auto rowIndexDeterminingBaseline =
      alignment == ItemPosition::Baseline ? 0 : CurrentGrid().NumTracks(GridTrackSizingDirection::Rows) - 1;
    for (uint32 column = 0; column < numColumns; column++)
    {
      auto &cell = CurrentGrid().Cell(rowIndexDeterminingBaseline,
                                      alignment == ItemPosition::Baseline ? column : numColumns - column - 1);

      for (auto *gridItem : cell)
      {
        krys_debug_assert(gridItem);
        // If an item participates in baseline alignment, we select such item.
        if (IsBaselineAlignmentForGridItem(*gridItem, GridTrackSizingDirection::Rows))
        {
          auto gridItemAlignment =
            SelfAlignmentForGridItem(GridTrackSizingDirection::Rows, *gridItem).Position();
          if (rowIndexDeterminingBaseline
              == GridLayoutFunctions::AlignmentContextForBaselineAlignment(
                GridSpanForGridItem(*gridItem, GridTrackSizingDirection::Rows), gridItemAlignment))
          {
            // FIXME: self-baseline and content-baseline alignment not implemented yet.
            return gridItem;
          }
        }
        if (!baselineGridItem)
        {
          baselineGridItem = gridItem;
        }
      }
    }
    return baselineGridItem;
  }

  LayoutUnit RenderGrid::ColumnAxisBaselineOffsetForGridItem(const RenderBox &gridItem) const noexcept
  {
    // FIXME : CSS Masonry does not properly handle baseline calculations currently.
    // We will just skip this running this step if we detect the RenderGrid is Masonry for now.
    if (IsMasonry())
    {
      return LayoutUnit {};
    }

    if (IsSubgridRows())
    {
      auto *outer = Downcast<RenderGrid>(Parent());
      if (GridLayoutFunctions::IsOrthogonalGridItem(*outer, *this))
      {
        return outer->RowAxisBaselineOffsetForGridItem(gridItem);
      }
      return outer->ColumnAxisBaselineOffsetForGridItem(gridItem);
    }
    return _trackSizingAlgorithm.BaselineOffsetForGridItem(gridItem, GridTrackSizingDirection::Rows);
  }

  LayoutUnit RenderGrid::RowAxisBaselineOffsetForGridItem(const RenderBox &gridItem) const noexcept
  {
    // FIXME : CSS Masonry does not properly handle baseline calculations currently.
    // We will just skip this running this step if we detect the RenderGrid is Masonry for now.
    if (IsMasonry())
    {
      return LayoutUnit {};
    }

    if (IsSubgridColumns())
    {
      auto *outer = Downcast<RenderGrid>(Parent());
      if (GridLayoutFunctions::IsOrthogonalGridItem(*outer, *this))
      {
        return outer->ColumnAxisBaselineOffsetForGridItem(gridItem);
      }
      return outer->RowAxisBaselineOffsetForGridItem(gridItem);
    }
    return _trackSizingAlgorithm.BaselineOffsetForGridItem(gridItem, GridTrackSizingDirection::Columns);
  }

  GridAxisPosition RenderGrid::ColumnAxisPositionForGridItem(const RenderBox &gridItem) const noexcept
  {
    if (gridItem.IsOutOfFlowPositioned()
        && !HasStaticPositionForGridItem(gridItem, GridTrackSizingDirection::Rows))
    {
      return GridAxisPosition::GridAxisStart;
    }

    bool hasSameDirection =
      IsHorizontalWritingMode()
        ? GetWritingMode().IsBlockTopToBottom() == gridItem.GetWritingMode().IsAnyTopToBottom()
        : GetWritingMode().IsBlockLeftToRight() == gridItem.GetWritingMode().IsAnyLeftToRight();

    switch (const auto gridItemAlignSelf = AlignSelfForGridItem(gridItem).Position())
    {
      case ItemPosition::SelfStart:
        // self-start is based on the grid item's block-flow direction.
        return hasSameDirection ? GridAxisPosition::GridAxisStart : GridAxisPosition::GridAxisEnd;
      case ItemPosition::SelfEnd:
        // self-end is based on the grid item's block-flow direction.
        return hasSameDirection ? GridAxisPosition::GridAxisEnd : GridAxisPosition::GridAxisStart;
      case ItemPosition::Left:
        // Aligns the alignment subject to be flush with the alignment container's 'line-left' edge.
        // The alignment axis (column axis) is always orthogonal to the inline axis, hence this value behaves
        // as 'start'.
        return GridAxisPosition::GridAxisStart;
      case ItemPosition::Right:
        // Aligns the alignment subject to be flush with the alignment container's 'line-right' edge.
        // The alignment axis (column axis) is always orthogonal to the inline axis, hence this value behaves
        // as 'start'.
        return GridAxisPosition::GridAxisStart;
      case ItemPosition::Center:
      case ItemPosition::AnchorCenter: return GridAxisPosition::GridAxisCenter;
      case ItemPosition::FlexStart:    // Only used in flex layout, otherwise equivalent to 'start'.
                                    // Aligns the alignment subject to be flush with the alignment container's
                                    // 'start' edge (block-start) in the column axis.
      case ItemPosition::Start:        return GridAxisPosition::GridAxisStart;
      case ItemPosition::FlexEnd:      // Only used in flex layout, otherwise equivalent to 'end'.
                                  // Aligns the alignment subject to be flush with the alignment container's
                                  // 'end' edge (block-end) in the column axis.
      case ItemPosition::End:          return GridAxisPosition::GridAxisEnd;
      case ItemPosition::Stretch:      return GridAxisPosition::GridAxisStart;
      case ItemPosition::Baseline:
      case ItemPosition::LastBaseline:
      {
        auto fallbackAlignment = [&]
        {
          if (gridItemAlignSelf == ItemPosition::Baseline)
          {
            return hasSameDirection ? GridAxisPosition::GridAxisStart : GridAxisPosition::GridAxisEnd;
          }
          return hasSameDirection ? GridAxisPosition::GridAxisEnd : GridAxisPosition::GridAxisStart;
        };
        if (GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem))
        {
          return gridItemAlignSelf == ItemPosition::Baseline ? GridAxisPosition::GridAxisStart
                                                             : GridAxisPosition::GridAxisEnd;
        }
        return fallbackAlignment();
      }
      case ItemPosition::Legacy:
      case ItemPosition::Auto:
      case ItemPosition::Normal: break;
    }

    krys_debug_assert(false);
    return GridAxisPosition::GridAxisStart;
  }

  GridAxisPosition RenderGrid::RowAxisPositionForGridItem(const RenderBox &gridItem) const noexcept
  {
    if (gridItem.IsOutOfFlowPositioned()
        && !HasStaticPositionForGridItem(gridItem, GridTrackSizingDirection::Columns))
    {
      return GridAxisPosition::GridAxisStart;
    }

    bool hasSameDirection =
      IsHorizontalWritingMode()
        ? GetWritingMode().IsInlineLeftToRight() == gridItem.GetWritingMode().IsAnyLeftToRight()
        : GetWritingMode().IsInlineTopToBottom() == gridItem.GetWritingMode().IsAnyTopToBottom();

    switch (JustifySelfForGridItem(gridItem).Position())
    {
      case ItemPosition::SelfStart:
        // self-start is based on the grid item's inline-flow direction.
        return hasSameDirection ? GridAxisPosition::GridAxisStart : GridAxisPosition::GridAxisEnd;
      case ItemPosition::SelfEnd:
        // self-end is based on the grid item's inline-flow direction.
        return hasSameDirection ? GridAxisPosition::GridAxisEnd : GridAxisPosition::GridAxisStart;
      case ItemPosition::Left:
        // Aligns the alignment subject to be flush with the alignment container's 'line-left' edge.
        // We want the physical 'left' side, so we have to take account, container's inline-flow direction.
        return GetWritingMode().IsBidiLTR() ? GridAxisPosition::GridAxisStart : GridAxisPosition::GridAxisEnd;
      case ItemPosition::Right:
        // Aligns the alignment subject to be flush with the alignment container's 'line-right' edge.
        // We want the physical 'right' side, so we have to take account, container's inline-flow direction.
        return GetWritingMode().IsBidiLTR() ? GridAxisPosition::GridAxisEnd : GridAxisPosition::GridAxisStart;
      case ItemPosition::Center:
      case ItemPosition::AnchorCenter: return GridAxisPosition::GridAxisCenter;
      case ItemPosition::FlexStart:    // Only used in flex layout, otherwise equivalent to 'start'.
                                    // Aligns the alignment subject to be flush with the alignment container's
                                    // 'start' edge (inline-start) in the row axis.
      case ItemPosition::Start:        return GridAxisPosition::GridAxisStart;
      case ItemPosition::FlexEnd:      // Only used in flex layout, otherwise equivalent to 'end'.
                                  // Aligns the alignment subject to be flush with the alignment container's
                                  // 'end' edge (inline-end) in the row axis.
      case ItemPosition::End:          return GridAxisPosition::GridAxisEnd;
      case ItemPosition::Stretch:      return GridAxisPosition::GridAxisStart;
      case ItemPosition::Baseline:
        // FIXME: Handle non-inline matching orthogonal grid items properly.
        if (GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem)
            && !GetWritingMode().IsInlineMatchingAny(gridItem.GetWritingMode()))
        {
          return GridAxisPosition::GridAxisStart;
        }
        return hasSameDirection ? GridAxisPosition::GridAxisStart : GridAxisPosition::GridAxisEnd;
      case ItemPosition::LastBaseline:
        // FIXME: Handle non-inline matching orthogonal grid items properly.
        if (GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem)
            && !GetWritingMode().IsInlineMatchingAny(gridItem.GetWritingMode()))
        {
          return GridAxisPosition::GridAxisStart;
        }
        return hasSameDirection ? GridAxisPosition::GridAxisEnd : GridAxisPosition::GridAxisStart;
      case ItemPosition::Legacy:
      case ItemPosition::Auto:
      case ItemPosition::Normal: break;
    }

    krys_debug_assert(false);
    return GridAxisPosition::GridAxisStart;
  }

  LayoutUnit RenderGrid::ColumnAxisOffsetForGridItem(const RenderBox &gridItem) const noexcept
  {
    auto [startOfRow, endOfRow] = GridAreaPositionForInFlowGridItem(gridItem, GridTrackSizingDirection::Rows);
    LayoutUnit startPosition = startOfRow + MarginBeforeForChild(gridItem);
    LayoutUnit columnAxisGridItemSize = GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem)
                                          ? gridItem.LogicalWidth() + gridItem.MarginLogicalWidth()
                                          : gridItem.LogicalHeight() + gridItem.MarginLogicalHeight();
    LayoutUnit masonryOffset = AreMasonryRows() ? _masonryLayout.OffsetForGridItem(gridItem) : 0_lu;
    auto overflow = AlignSelfForGridItem(gridItem).Overflow();
    LayoutUnit offsetFromStartPosition =
      ComputeOverflowAlignmentOffset(overflow, endOfRow - startOfRow, columnAxisGridItemSize);
    if (GridLayoutFunctions::HasAutoMarginsInColumnAxis(gridItem, GetWritingMode()))
    {
      return startPosition;
    }
    GridAxisPosition axisPosition = ColumnAxisPositionForGridItem(gridItem);
    switch (axisPosition)
    {
      case GridAxisPosition::GridAxisStart:
        return startPosition + ColumnAxisBaselineOffsetForGridItem(gridItem) + masonryOffset;
      case GridAxisPosition::GridAxisEnd:
        return (startPosition + offsetFromStartPosition) - ColumnAxisBaselineOffsetForGridItem(gridItem);
      case GridAxisPosition::GridAxisCenter: return startPosition + (offsetFromStartPosition / 2);
    }

    krys_debug_assert(false);
    return 0_lu;
  }

  LayoutUnit RenderGrid::RowAxisOffsetForGridItem(const RenderBox &gridItem) const noexcept
  {
    auto [startOfColumn, endOfColumn] =
      GridAreaPositionForInFlowGridItem(gridItem, GridTrackSizingDirection::Columns);
    LayoutUnit startPosition = startOfColumn + MarginStartForChild(gridItem);
    LayoutUnit masonryOffset = AreMasonryColumns() ? _masonryLayout.OffsetForGridItem(gridItem) : 0_lu;
    if (GridLayoutFunctions::HasAutoMarginsInRowAxis(gridItem, GetWritingMode()))
    {
      return startPosition;
    }
    LayoutUnit rowAxisGridItemSize = GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem)
                                       ? gridItem.LogicalHeight() + gridItem.MarginLogicalHeight()
                                       : gridItem.LogicalWidth() + gridItem.MarginLogicalWidth();
    auto overflow = JustifySelfForGridItem(gridItem).Overflow();
    auto rowAxisBaselineOffset = RowAxisBaselineOffsetForGridItem(gridItem);
    LayoutUnit offsetFromStartPosition =
      ComputeOverflowAlignmentOffset(overflow, endOfColumn - startOfColumn, rowAxisGridItemSize);
    GridAxisPosition axisPosition = RowAxisPositionForGridItem(gridItem);
    switch (axisPosition)
    {
      case GridAxisPosition::GridAxisStart: return startPosition + rowAxisBaselineOffset + masonryOffset;
      case GridAxisPosition::GridAxisEnd:
        return startPosition + offsetFromStartPosition - rowAxisBaselineOffset;
      case GridAxisPosition::GridAxisCenter: return startPosition + offsetFromStartPosition / 2;
    }

    krys_debug_assert(false);
    return 0_lu;
  }

  bool RenderGrid::IsSubgrid() const noexcept
  {
    return IsSubgrid(GridTrackSizingDirection::Rows) || IsSubgrid(GridTrackSizingDirection::Columns);
  }

  bool RenderGrid::IsSubgrid(GridTrackSizingDirection direction) const noexcept
  {
    // If the grid container is forced to establish an independent formatting
    // context (like contain layout, or position:absolute), then the used value
    // of grid-template-rows/columns is 'none' and the container is not a subgrid.
    // https://drafts.csswg.org/css-grid-2/#subgrid-listing
    if (EstablishesIndependentFormattingContextIgnoringDisplayType(Style()))
    {
      return false;
    }
    if (!Style().GridTemplateList(direction).Subgrid)
    {
      return false;
    }
    auto *renderGrid = DynamicDowncast<RenderGrid>(Parent());
    if (!renderGrid)
    {
      return false;
    }
    return !renderGrid->IsMasonry(direction);
  }

  bool RenderGrid::IsSubgridInParentDirection(GridTrackSizingDirection parentDirection) const noexcept
  {
    auto *renderGrid = DynamicDowncast<RenderGrid>(Parent());
    if (!renderGrid)
    {
      return false;
    }
    auto direction = GridLayoutFunctions::FlowAwareDirectionForGridItem(*renderGrid, *this, parentDirection);
    return IsSubgrid(direction);
  }

  bool RenderGrid::IsSubgridOf(GridTrackSizingDirection direction, const RenderGrid &ancestor) const noexcept
  {
    if (!IsSubgrid(direction))
    {
      return false;
    }
    if (Parent() == &ancestor)
    {
      return true;
    }

    auto &parentGrid = *Downcast<RenderGrid>(Parent());
    auto parentDirection = GridLayoutFunctions::FlowAwareDirectionForParent(parentGrid, *this, direction);
    return parentGrid.IsSubgridOf(parentDirection, ancestor);
  }

  const Grid &RenderGrid::CurrentGrid() const noexcept
  {
    return _grid.CurrentGrid;
  }

  Grid &RenderGrid::CurrentGrid() noexcept
  {
    return _grid.CurrentGrid;
  }

  LayoutRange RenderGrid::GridAreaRangeForOutOfFlow(const RenderBox &gridItem,
                                                    GridTrackSizingDirection direction) const noexcept
  {
    krys_debug_assert(gridItem.IsOutOfFlowPositioned());
    bool isRowAxis = direction == GridTrackSizingDirection::Columns;
    LayoutUnit borderEdge = isRowAxis ? BorderStart() : BorderBefore();
    if (CurrentGrid().NeedsItemsPlacement())
    {
      // Haven't completed in-flow placement and grid sizing yet.
      // Return something basic that doesn't access unbuilt data structures.
      return LayoutRange(borderEdge, isRowAxis ? ClientLogicalWidth() : ClientLogicalHeight());
    }

    int startLine;
    int endLine;
    bool startIsAuto;
    bool endIsAuto;
    if (!ComputeGridPositionsForOutOfFlowGridItem(gridItem, direction, startLine, startIsAuto, endLine,
                                                  endIsAuto)
        || (startIsAuto && endIsAuto))
    {
      return LayoutRange(borderEdge, isRowAxis ? ClientLogicalWidth() : ClientLogicalHeight());
    }

    LayoutUnit start;
    LayoutUnit end;

    auto &positions = Positions(direction);
    if (positions.empty())
    {
      krys_debug_assert(false);
      return LayoutRange(borderEdge, isRowAxis ? ClientLogicalWidth() : ClientLogicalHeight());
    }

    if (startIsAuto)
    {
      start = borderEdge;
    }
    else
    {
      start = positions[startLine];
    }
    if (endIsAuto)
    {
      end = (isRowAxis ? ClientLogicalWidth() : ClientLogicalHeight()) + borderEdge;
    }
    else
    {
      end = positions[endLine];
      // These vectors store line positions including gaps, but we shouldn't consider them for the edges of
      // the grid.
      Maybe<LayoutUnit> availableSizeForGutters = AvailableSpaceForGutters(direction);
      auto lastLine = static_cast<int>(NumTracks(direction));
      if (endLine > 0 && endLine < lastLine)
      {
        end -= GuttersSize(direction, static_cast<uint32>(endLine - 1), 2, availableSizeForGutters);
        end -= isRowAxis ? _offsetBetweenColumns.DistributionOffset : _offsetBetweenRows.DistributionOffset;
      }
    }
    return LayoutRange(start, std::max(end - start, 0_lu));
  }

  Pair<LayoutUnit, LayoutUnit>
    RenderGrid::GridAreaPositionForInFlowGridItem(const RenderBox &gridItem,
                                                  GridTrackSizingDirection direction) const noexcept
  {
    krys_debug_assert(!gridItem.IsOutOfFlowPositioned());
    const auto &span = CurrentGrid().GridItemSpan(gridItem, direction);
    const auto &positions = Positions(direction);
    auto start = positions[span.StartLine()];
    auto end = positions[span.EndLine()];
    // The 'positions' vector includes distribution offset (because of content
    // alignment) and gutters, so we need to subtract them to get the actual
    // end position for a given track (this does not have to be done for the
    // last track as there are no more positions' elements after it, nor for
    // collapsed tracks).
    if (span.EndLine() < positions.size() - 1
        && !(CurrentGrid().HasAutoRepeatEmptyTracks(direction)
             && CurrentGrid().IsEmptyAutoRepeatTrack(direction, span.EndLine())))
    {
      end -= GridGap(direction) + GridItemOffset(direction);
    }
    return {start, end};
  }

  StyleContentAlignmentData RenderGrid::ContentAlignment(GridTrackSizingDirection direction) const noexcept
  {
    return direction == GridTrackSizingDirection::Columns
             ? Style().GetJustifyContent().Resolve(ContentAlignmentNormalBehaviorGrid())
             : Style().GetAlignContent().Resolve(ContentAlignmentNormalBehaviorGrid());
  }

  ContentAlignmentData
    RenderGrid::ComputeContentPositionAndDistributionOffset(GridTrackSizingDirection direction,
                                                            const LayoutUnit &availableFreeSpace,
                                                            uint32 numberOfGridTracks) const noexcept
  {
    if (IsSubgrid(direction))
    {
      return {};
    }

    auto contentAlignmentData = ContentAlignment(direction);
    auto contentAlignmentDistribution = contentAlignmentData.Distribution();

    // Apply <content-distribution> and return, or continue to fallback positioning if we can't distribute.
    if (contentAlignmentDistribution != ContentDistribution::Default)
    {
      if (availableFreeSpace > 0)
      {
        switch (contentAlignmentDistribution)
        {
          case ContentDistribution::SpaceBetween:
            if (numberOfGridTracks < 2)
            {
              break;
            }
            return {0_lu, availableFreeSpace / static_cast<int>(numberOfGridTracks - 1)};
          case ContentDistribution::SpaceAround:
          {
            if (numberOfGridTracks < 1)
            {
              break;
            }
            auto spaceBetweenTracks = availableFreeSpace / static_cast<int>(numberOfGridTracks);
            return {spaceBetweenTracks / 2, spaceBetweenTracks};
          }
          case ContentDistribution::SpaceEvenly:
          {
            auto spaceEvenlyDistribution = availableFreeSpace / static_cast<int>(numberOfGridTracks + 1);
            return {spaceEvenlyDistribution, spaceEvenlyDistribution};
          }
          case ContentDistribution::Stretch: break;
          default:                           krys_debug_assert(false);
        }
      }
    }

    auto [fallbackOverflow, fallbackContentPosition] =
      ResolveContentDistributionFallback(contentAlignmentDistribution);
    auto contentAlignmentOverflow = contentAlignmentData.Overflow();

    // Apply alignment safety.
    if (availableFreeSpace <= 0
        && (contentAlignmentOverflow == OverflowAlignment::Safe
            || fallbackOverflow == OverflowAlignment::Safe))
    {
      return {};
    }

    auto usedContentPosition = contentAlignmentDistribution == ContentDistribution::Default
                                 ? contentAlignmentData.Position()
                                 : fallbackContentPosition;
    // Apply <content-position> / fallback positioning.
    switch (usedContentPosition)
    {
      case ContentPosition::Left:
        krys_debug_assert(direction == GridTrackSizingDirection::Columns);
        if (!GetWritingMode().IsBidiLTR())
        {
          return {availableFreeSpace, 0_lu};
        }
        return {};
      case ContentPosition::Right:
        krys_debug_assert(direction == GridTrackSizingDirection::Columns);
        if (GetWritingMode().IsBidiLTR())
        {
          return {availableFreeSpace, 0_lu};
        }
        return {};
      case ContentPosition::Center:  return {availableFreeSpace / 2, 0_lu};
      case ContentPosition::FlexEnd: // Only used in flex layout, for other layout, it's equivalent to 'end'.
      case ContentPosition::End:     return {availableFreeSpace, 0_lu};
      case ContentPosition::FlexStart: // Only used in flex layout, for other layout, it's equivalent to
                                       // 'start'.
      case ContentPosition::Start:
      case ContentPosition::Baseline:
      case ContentPosition::LastBaseline:
        // FIXME: Implement the baseline values. For now, we always 'start' align.
        // http://webkit.org/b/145566
        return {};
      case ContentPosition::Normal:
      default:                      krys_debug_assert(false); return {};
    }
  }

  LayoutOptionalOutsets RenderGrid::AllowedLayoutOverflow() const noexcept
  {
    LayoutOptionalOutsets allowance = RenderBox::AllowedLayoutOverflow();
    if (_offsetBetweenColumns.PositionOffset < 0)
    {
      allowance.SetStart(-_offsetBetweenColumns.PositionOffset, GetWritingMode());
    }

    if (_offsetBetweenRows.PositionOffset < 0)
    {
      if (IsHorizontalWritingMode())
      {
        allowance.SetTop(-_offsetBetweenRows.PositionOffset);
      }
      else
      {
        allowance.SetLeft(-_offsetBetweenRows.PositionOffset);
      }
    }

    return allowance;
  }

  LayoutUnit RenderGrid::TranslateRTLCoordinate(LayoutUnit coordinate) const noexcept
  {
    LayoutUnit width = BorderLogicalLeft() + BorderLogicalRight() + ClientLogicalWidth();

    // If we are in horizontal writing mode and RTL direction the scrollbar is painted on the left,
    // so we need to take into account when computing the position of the columns.
    if (GetWritingMode().IsHorizontal())
    {
      width += LayoutUnit(ScrollbarLogicalWidth());
    }

    return width - coordinate;
  }

  // FIXME: SetLogicalPositionForGridItem has only one caller, consider its refactoring in the future.
  void RenderGrid::SetLogicalPositionForGridItem(RenderBox &gridItem) const noexcept
  {
    // "In the positioning phase [...] calculations are performed according to the writing mode of the
    // containing block of the box establishing the orthogonal flow." However, 'setLogicalLocation' will only
    // take into account the grid item's writing-mode, so the position may need to be transposed.
    LayoutPoint gridItemLocation(LogicalOffsetForGridItem(gridItem, GridTrackSizingDirection::Columns),
                                 LogicalOffsetForGridItem(gridItem, GridTrackSizingDirection::Rows));
    gridItem.SetLogicalLocation(GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem)
                                  ? gridItemLocation.TransposedPoint()
                                  : gridItemLocation);
  }

  LayoutUnit RenderGrid::LogicalOffsetForGridItem(const RenderBox &gridItem,
                                                  GridTrackSizingDirection direction) const noexcept
  {
    if (direction == GridTrackSizingDirection::Rows)
    {
      return ColumnAxisOffsetForGridItem(gridItem);
    }
    LayoutUnit rowAxisOffset = RowAxisOffsetForGridItem(gridItem);
    // We stored m_columnPositions's data ignoring the direction, hence we might need now
    // to translate positions from RTL to LTR, as it's more convenient for painting.
    if (GetWritingMode().IsInlineFlipped())
    {
      rowAxisOffset =
        TranslateRTLCoordinate(rowAxisOffset)
        - (GridLayoutFunctions::IsOrthogonalGridItem(*this, gridItem) ? gridItem.LogicalHeight()
                                                                      : gridItem.LogicalWidth());
    }
    return rowAxisOffset;
  }

  uint32 RenderGrid::NonCollapsedTracks(GridTrackSizingDirection direction) const noexcept
  {
    auto &tracks = _trackSizingAlgorithm.GetTracks(direction);
    size_t numberOfTracks = tracks.size();
    bool hasCollapsedTracks = CurrentGrid().HasAutoRepeatEmptyTracks(direction);
    size_t numberOfCollapsedTracks =
      hasCollapsedTracks ? CurrentGrid().AutoRepeatEmptyTracks(direction)->size() : 0;
    return static_cast<uint32>(numberOfTracks - numberOfCollapsedTracks);
  }

  uint32 RenderGrid::NumTracks(GridTrackSizingDirection direction) const noexcept
  {
    // Due to limitations in our internal representation, we cannot know the number of columns from
    // currentGrid *if* there is no row (because currentGrid would be empty). That's why in that case we need
    // to get it from the style. Note that we know for sure that there aren't any implicit tracks,
    // because not having rows implies that there are no "normal" grid items (out-of-flow grid items are
    // not stored in currentGrid).
    krys_debug_assert(!CurrentGrid().NeedsItemsPlacement());
    if (direction == GridTrackSizingDirection::Rows)
    {
      return CurrentGrid().NumTracks(GridTrackSizingDirection::Rows);
    }

    // FIXME: This still requires knowledge about currentGrid internals.
    return CurrentGrid().NumTracks(GridTrackSizingDirection::Rows)
             ? CurrentGrid().NumTracks(GridTrackSizingDirection::Columns)
             : style::GridPositionsResolver::ExplicitGridCount(*this, GridTrackSizingDirection::Columns);
  }

  utf8_stringview RenderGrid::RenderName() const noexcept
  {
    if (IsFloating())
    {
      return u8"RenderGrid (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderGrid (positioned)";
    }
    if (IsAnonymous())
    {
      return u8"RenderGrid (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderGrid (relative positioned)";
    }
    return u8"RenderGrid";
  }

  bool RenderGrid::ComputeGridPositionsForOutOfFlowGridItem(const RenderBox &gridItem,
                                                            GridTrackSizingDirection direction,
                                                            int &startLine, bool &startIsAuto, int &endLine,
                                                            bool &endIsAuto) const noexcept
  {
    krys_debug_assert(gridItem.IsOutOfFlowPositioned());
    auto lastLine = static_cast<int>(NumTracks(direction));
    auto span = style::GridPositionsResolver::ResolveGridPositionsFromStyle(*this, gridItem, direction);
    if (span.IsIndefinite())
    {
      return false;
    }

    auto explicitStart = static_cast<int>(CurrentGrid().ExplicitGridStart(direction));
    startLine = span.UntranslatedStartLine() + explicitStart;
    endLine = span.UntranslatedEndLine() + explicitStart;
    startIsAuto = gridItem.Style().GridItemStart(direction).IsAuto() || startLine < 0 || startLine > lastLine;
    endIsAuto = gridItem.Style().GridItemEnd(direction).IsAuto() || endLine < 0 || endLine > lastLine;
    return true;
  }

  GridSpan RenderGrid::GridSpanForOutOfFlowGridItem(const RenderBox &gridItem,
                                                    GridTrackSizingDirection direction) const noexcept
  {
    auto lastLine = static_cast<int>(NumTracks(direction));
    int startLine;
    int endLine;
    bool startIsAuto;
    bool endIsAuto;
    if (!ComputeGridPositionsForOutOfFlowGridItem(gridItem, direction, startLine, startIsAuto, endLine,
                                                  endIsAuto))
    {
      return GridSpan::TranslatedDefiniteGridSpan(0, static_cast<uint32>(lastLine));
    }
    return GridSpan::TranslatedDefiniteGridSpan(static_cast<uint32>(startIsAuto ? 0 : startLine),
                                                static_cast<uint32>(endIsAuto ? lastLine : endLine));
  }

  GridSpan RenderGrid::GridSpanForGridItem(const RenderBox &gridItem,
                                           GridTrackSizingDirection direction) const noexcept
  {
    auto *renderGrid = Downcast<RenderGrid>(gridItem.Parent());
    // |direction| is specified relative to this grid, switch it if |gridItem|'s direct parent grid
    // is using a different writing mode.
    direction = GridLayoutFunctions::FlowAwareDirectionForGridItem(*this, *renderGrid, direction);
    GridSpan span = gridItem.IsOutOfFlowPositioned()
                      ? renderGrid->GridSpanForOutOfFlowGridItem(gridItem, direction)
                      : renderGrid->CurrentGrid().GridItemSpan(gridItem, direction);

    while (renderGrid != this)
    {
      auto *parent = Downcast<RenderGrid>(renderGrid->Parent());

      bool isSubgrid = renderGrid->IsSubgrid(direction);

      direction = GridLayoutFunctions::FlowAwareDirectionForGridItem(*parent, *renderGrid, direction);

      GridSpan parentSpan = renderGrid->IsOutOfFlowPositioned()
                              ? parent->GridSpanForOutOfFlowGridItem(*renderGrid, direction)
                              : parent->CurrentGrid().GridItemSpan(*renderGrid, direction);
      if (isSubgrid)
      {
        span.TranslateTo(parentSpan,
                         GridLayoutFunctions::IsSubgridReversedDirection(*parent, direction, *renderGrid));
      }
      else
      {
        span = parentSpan;
      }
      renderGrid = parent;
    }
    return span;
  }

  RenderGrid::GridWrapper::GridWrapper(RenderGrid &renderGrid) noexcept : _layoutGrid(renderGrid)
  {
  }

  void RenderGrid::GridWrapper::ResetCurrentGrid() const noexcept
  {
    CurrentGrid = std::ref(const_cast<Grid &>(_layoutGrid));
  }

  void RenderGrid::ComputeOverflow(LayoutUnit oldClientAfterEdge,
                                   EnumSet<ComputeOverflowOptions> options) noexcept
  {
    RenderBlock::ComputeOverflow(oldClientAfterEdge, options);

    if (!HasPotentiallyScrollableOverflow() || IsMasonry() || IsSubgridRows() || IsSubgridColumns())
    {
      return;
    }

    // FIXME: We should handle RTL and other writing modes also.
    if (GetWritingMode().IsBidiLTR() && IsHorizontalWritingMode())
    {
      auto gridAreaSize = LayoutSize {_columnPositions.back(), _rowPositions.back()};
      gridAreaSize += LayoutSize {PaddingEnd(), PaddingAfter()};
      AddLayoutOverflow(LayoutRect {LayoutPoint {}, gridAreaSize});
    }
  }

  void RenderGrid::UpdateIntrinsicLogicalHeightsForRowSizingFirstPassCacheAvailability() noexcept
  {
    auto canCreateIntrinsicLogicalHeightsCacheForRowSizingFirstPass =
      CanCreateIntrinsicLogicalHeightsForRowSizingFirstPassCache();

    if (canCreateIntrinsicLogicalHeightsCacheForRowSizingFirstPass
        && _intrinsicLogicalHeightsForRowSizingFirstPass)
    {
      for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
      {
        if (gridItem.NeedsLayout())
        {
          _intrinsicLogicalHeightsForRowSizingFirstPass->InvalidateSizeForItem(gridItem);
        }
      }
    }
    else if (canCreateIntrinsicLogicalHeightsCacheForRowSizingFirstPass)
    {
      _intrinsicLogicalHeightsForRowSizingFirstPass.emplace();
    }
    else
    {
      _intrinsicLogicalHeightsForRowSizingFirstPass.reset();
    }
  }

  Maybe<GridItemSizeCache> &RenderGrid::IntrinsicLogicalHeightsForRowSizingFirstPass() const noexcept
  {
    krys_debug_assert(!_intrinsicLogicalHeightsForRowSizingFirstPass
                      || CanCreateIntrinsicLogicalHeightsForRowSizingFirstPassCache());
    return _intrinsicLogicalHeightsForRowSizingFirstPass;
  }

  bool RenderGrid::CanCreateIntrinsicLogicalHeightsForRowSizingFirstPassCache() const noexcept
  {
    if (IsMasonry())
    {
      return false;
    }

    if (IsSubgridRows())
    {
      return false;
    }

    // FIXME: No cache inside fragmented flows (enclosingFragmentedFlow), once there's pagination.

    for (auto &gridItem : ChildrenOfType<RenderBox>(*this))
    {
      if (auto *renderGrid = DynamicDowncast<RenderGrid>(gridItem))
      {
        if (renderGrid->IsSubgridRows())
        {
          return false;
        }

        if (renderGrid->IsSubgridColumns() && GridLayoutFunctions::IsOrthogonalGridItem(*this, *renderGrid))
        {
          return false;
        }
      }

      if (IsBaselineAlignmentForGridItem(gridItem))
      {
        return false;
      }
    }
    return true;
  }

  void GridItemSizeCache::SetSizeForGridItem(const RenderBox &gridItem, LayoutUnit size) noexcept
  {
    _sizes.insert_or_assign(&gridItem, size);
  }

  Maybe<LayoutUnit> GridItemSizeCache::SizeForItem(const RenderBox &gridItem) const noexcept
  {
    auto entry = _sizes.find(&gridItem);
    return entry == _sizes.end() ? null : entry->second;
  }

  void GridItemSizeCache::InvalidateSizeForItem(const RenderBox &gridItem) noexcept
  {
    _sizes.erase(&gridItem);
  }
  void RenderGrid::PaintChildren(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                 PaintInfo &forChild) noexcept
  {
    krys_debug_assert(!CurrentGrid().NeedsItemsPlacement());
    for (RenderBox *gridItem = CurrentGrid().GetOrderIterator().First(); gridItem;
         gridItem = CurrentGrid().GetOrderIterator().Next())
    {
      (void)PaintChild(*gridItem, paintInfo, paintOffset, forChild, PaintAsInlineBlock);
    }
  }
}
