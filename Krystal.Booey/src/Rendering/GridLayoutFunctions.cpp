#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/AncestorSubgridIterator.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Style/GridSpan.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"

namespace krys::boo::GridLayoutFunctions
{
  using style::GridTrackSizingDirection;

  namespace
  {
    KRYS_NODISCARD inline bool MarginStartIsAuto(const RenderBox &gridItem,
                                                 GridTrackSizingDirection direction) noexcept
    {
      return direction == GridTrackSizingDirection::Columns ? gridItem.Style().MarginStart().IsAuto()
                                                            : gridItem.Style().MarginBefore().IsAuto();
    }

    KRYS_NODISCARD inline bool MarginEndIsAuto(const RenderBox &gridItem,
                                               GridTrackSizingDirection direction) noexcept
    {
      return direction == GridTrackSizingDirection::Columns ? gridItem.Style().MarginEnd().IsAuto()
                                                            : gridItem.Style().MarginAfter().IsAuto();
    }

    KRYS_NODISCARD bool GridItemHasMargin(const RenderBox &gridItem,
                                          GridTrackSizingDirection direction) noexcept
    {
      auto hasMarginEdge = [](auto &edge)
      {
        return !edge.IsKnownZero() && !edge.IsAuto();
      };

      if (direction == GridTrackSizingDirection::Columns)
      {
        return hasMarginEdge(gridItem.Style().MarginStart()) || hasMarginEdge(gridItem.Style().MarginEnd());
      }
      return hasMarginEdge(gridItem.Style().MarginBefore()) || hasMarginEdge(gridItem.Style().MarginAfter());
    }

    KRYS_NODISCARD ExtraMarginsFromSubgrids ExtraMarginForSubgrid(const RenderGrid &parent, uint32 startLine,
                                                                  uint32 endLine,
                                                                  GridTrackSizingDirection direction) noexcept
    {
      uint32 numTracks = parent.NumTracks(direction);
      if (!numTracks || !parent.IsSubgrid(direction))
      {
        return {};
      }

      Maybe<LayoutUnit> availableSpace;
      if (!HasRelativeOrIntrinsicSizeForGridItem(parent, direction))
      {
        availableSpace = parent.AvailableSpaceForGutters(direction);
      }

      auto &grandParent = Downcast<RenderGrid>(*parent.Parent());
      ExtraMarginsFromSubgrids extraMargins;
      if (!startLine)
      {
        extraMargins.AddTrackStartMargin((direction == GridTrackSizingDirection::Columns)
                                           ? parent.MarginAndBorderAndPaddingStart()
                                           : parent.MarginAndBorderAndPaddingBefore());
      }
      else
      {
        extraMargins.AddTrackStartMargin(
          (parent.GridGap(direction, availableSpace) - grandParent.GridGap(direction)) / 2);
      }

      if (endLine == numTracks)
      {
        extraMargins.AddTrackEndMargin((direction == GridTrackSizingDirection::Columns)
                                         ? parent.MarginAndBorderAndPaddingEnd()
                                         : parent.MarginAndBorderAndPaddingAfter());
      }
      else
      {
        extraMargins.AddTrackEndMargin(
          (parent.GridGap(direction, availableSpace) - grandParent.GridGap(direction)) / 2);
      }

      return extraMargins;
    }
  }

  LayoutUnit ComputeMarginLogicalSizeForGridItem(const RenderGrid &grid, GridTrackSizingDirection direction,
                                                 const RenderBox &gridItem) noexcept
  {
    auto flowAwareDirection = FlowAwareDirectionForGridItem(grid, gridItem, direction);
    if (!GridItemHasMargin(gridItem, flowAwareDirection))
    {
      return 0_lu;
    }

    LayoutUnit marginStart;
    LayoutUnit marginEnd;
    if (direction == GridTrackSizingDirection::Columns)
    {
      gridItem.ComputeInlineDirectionMargins(grid, gridItem.ContainingBlockLogicalWidthForContent(), null,
                                             gridItem.LogicalWidth(), marginStart, marginEnd);
    }
    else
    {
      gridItem.ComputeBlockDirectionMargins(grid, marginStart, marginEnd);
    }
    return MarginStartIsAuto(gridItem, flowAwareDirection) ? marginEnd
           : MarginEndIsAuto(gridItem, flowAwareDirection) ? marginStart
                                                           : marginStart + marginEnd;
  }

  bool HasRelativeOrIntrinsicSizeForGridItem(const RenderBox &gridItem,
                                             GridTrackSizingDirection direction) noexcept
  {
    if (direction == GridTrackSizingDirection::Columns)
    {
      return gridItem.HasRelativeLogicalWidth()
             || gridItem.Style().LogicalWidth().IsIntrinsicOrLegacyIntrinsicOrAuto();
    }
    return gridItem.HasRelativeLogicalHeight()
           || gridItem.Style().LogicalHeight().IsIntrinsicOrLegacyIntrinsicOrAuto();
  }

  ExtraMarginsFromSubgrids ExtraMarginForSubgridAncestors(GridTrackSizingDirection direction,
                                                          const RenderBox &gridItem) noexcept
  {
    ExtraMarginsFromSubgrids extraMargins;
    for (auto &currentAncestorSubgrid : AncestorSubgridsOfGridItem(gridItem, direction))
    {
      GridSpan span = currentAncestorSubgrid.GridSpanForGridItem(gridItem, direction);
      extraMargins +=
        ExtraMarginForSubgrid(currentAncestorSubgrid, span.StartLine(), span.EndLine(), direction);
    }
    return extraMargins;
  }

  LayoutUnit MarginLogicalSizeForGridItem(const RenderGrid &grid, GridTrackSizingDirection direction,
                                          const RenderBox &gridItem) noexcept
  {
    auto margin = ComputeMarginLogicalSizeForGridItem(grid, direction, gridItem);

    if (&grid != gridItem.Parent())
    {
      auto subgridDirection =
        FlowAwareDirectionForGridItem(grid, *Downcast<RenderGrid>(gridItem.Parent()), direction);
      margin += ExtraMarginForSubgridAncestors(subgridDirection, gridItem).ExtraTotalMargin();
    }

    return margin;
  }

  bool IsOrthogonalGridItem(const RenderGrid &grid, const RenderBox &gridItem) noexcept
  {
    return gridItem.IsHorizontalWritingMode() != grid.IsHorizontalWritingMode();
  }

  bool IsOrthogonalParent(const RenderGrid &grid, const RenderElement &parent) noexcept
  {
    return parent.IsHorizontalWritingMode() != grid.IsHorizontalWritingMode();
  }

  bool IsAspectRatioBlockSizeDependentGridItem(const RenderBox &gridItem) noexcept
  {
    return (gridItem.Style().HasAspectRatio() || gridItem.HasIntrinsicAspectRatio())
           && (gridItem.HasRelativeLogicalHeight() || gridItem.HasStretchedLogicalHeight());
  }

  bool IsGridItemInlineSizeDependentOnBlockConstraints(const RenderBox &gridItem,
                                                       const RenderGrid &parentGrid,
                                                       ItemPosition gridItemAlignSelf) noexcept
  {
    krys_debug_assert(gridItem.Parent() == &parentGrid);

    if (IsOrthogonalGridItem(parentGrid, gridItem))
    {
      return true;
    }

    auto &gridItemStyle = gridItem.Style();
    auto gridItemFlexWrap = gridItemStyle.GetFlexWrap();
    if (gridItem.IsRenderFlexibleBox() && gridItem.Style().IsColumnFlexDirection()
        && (gridItemFlexWrap == FlexWrap::Wrap || gridItemFlexWrap == FlexWrap::Reverse))
    {
      return true;
    }

    // FIXME: Multi-column flows (isRenderMultiColumnFlow), once there are those.

    if (IsAspectRatioBlockSizeDependentGridItem(gridItem))
    {
      return true;
    }

    // Stretch alignment allows the grid item content to resolve against the stretched size.
    if (gridItemAlignSelf != ItemPosition::Stretch)
    {
      return false;
    }

    auto hasAspectRatioAndInlineSizeDependsOnBlockSize = [](auto &renderer)
    {
      auto &rendererStyle = renderer.Style();
      bool rendererHasAspectRatio = renderer.HasIntrinsicAspectRatio() || rendererStyle.HasAspectRatio();

      return rendererHasAspectRatio && rendererStyle.LogicalWidth().IsAuto()
             && !rendererStyle.LogicalHeight().IsIntrinsicOrLegacyIntrinsicOrAuto();
    };

    for (auto &gridItemChild : ChildrenOfType<RenderBox>(gridItem))
    {
      if (hasAspectRatioAndInlineSizeDependsOnBlockSize(gridItemChild))
      {
        return true;
      }
    }

    return false;
  }

  GridTrackSizingDirection FlowAwareDirectionForGridItem(const RenderGrid &grid, const RenderBox &gridItem,
                                                         GridTrackSizingDirection direction) noexcept
  {
    return !IsOrthogonalGridItem(grid, gridItem) ? direction : style::OrthogonalDirection(direction);
  }

  GridTrackSizingDirection FlowAwareDirectionForParent(const RenderGrid &grid, const RenderElement &parent,
                                                       GridTrackSizingDirection direction) noexcept
  {
    return IsOrthogonalParent(grid, parent) ? style::OrthogonalDirection(direction) : direction;
  }

  Maybe<RenderBox::GridAreaSize>
    OverridingContainingBlockContentSizeForGridItem(const RenderBox &gridItem,
                                                    GridTrackSizingDirection direction) noexcept
  {
    return direction == GridTrackSizingDirection::Columns ? gridItem.GridAreaContentLogicalWidth()
                                                          : gridItem.GridAreaContentLogicalHeight();
  }

  bool IsFlippedDirection(const RenderGrid &grid, GridTrackSizingDirection direction) noexcept
  {
    if (direction == GridTrackSizingDirection::Columns)
    {
      return grid.GetWritingMode().IsBidiRTL();
    }
    return grid.GetWritingMode().IsBlockFlipped();
  }

  bool IsSubgridReversedDirection(const RenderGrid &grid, GridTrackSizingDirection outerDirection,
                                  const RenderGrid &subgrid) noexcept
  {
    auto subgridDirection = FlowAwareDirectionForGridItem(grid, subgrid, outerDirection);
    krys_debug_assert(subgrid.IsSubgrid(subgridDirection));
    return IsFlippedDirection(grid, outerDirection) != IsFlippedDirection(subgrid, subgridDirection);
  }

  uint32 AlignmentContextForBaselineAlignment(const GridSpan &span, const ItemPosition &alignment) noexcept
  {
    krys_debug_assert(alignment == ItemPosition::Baseline || alignment == ItemPosition::LastBaseline);
    if (alignment == ItemPosition::Baseline)
    {
      return span.StartLine();
    }
    return span.EndLine() - 1;
  }

  void SetOverridingContentSizeForGridItem(const RenderGrid &renderGrid, RenderBox &gridItem,
                                           LayoutUnit logicalSize,
                                           GridTrackSizingDirection direction) noexcept
  {
    if ((direction == GridTrackSizingDirection::Columns) != IsOrthogonalGridItem(renderGrid, gridItem))
    {
      gridItem.SetOverridingBorderBoxLogicalWidth(logicalSize);
    }
    else
    {
      gridItem.SetOverridingBorderBoxLogicalHeight(logicalSize);
    }
  }

  void ClearOverridingContentSizeForGridItem(const RenderGrid &renderGrid, RenderBox &gridItem,
                                             GridTrackSizingDirection direction) noexcept
  {
    if ((direction == GridTrackSizingDirection::Columns) != IsOrthogonalGridItem(renderGrid, gridItem))
    {
      gridItem.ClearOverridingBorderBoxLogicalWidth();
    }
    else
    {
      gridItem.ClearOverridingBorderBoxLogicalHeight();
    }
  }

  bool HasAutoMarginsInColumnAxis(const RenderBox &gridItem, WritingMode parentWritingMode) noexcept
  {
    if (parentWritingMode.IsHorizontal())
    {
      return gridItem.Style().MarginTop().IsAuto() || gridItem.Style().MarginBottom().IsAuto();
    }
    return gridItem.Style().MarginLeft().IsAuto() || gridItem.Style().MarginRight().IsAuto();
  }

  bool HasAutoMarginsInRowAxis(const RenderBox &gridItem, WritingMode parentWritingMode) noexcept
  {
    if (parentWritingMode.IsHorizontal())
    {
      return gridItem.Style().MarginLeft().IsAuto() || gridItem.Style().MarginRight().IsAuto();
    }
    return gridItem.Style().MarginTop().IsAuto() || gridItem.Style().MarginBottom().IsAuto();
  }

  bool HasAutoSizeInColumnAxis(const RenderBox &gridItem, WritingMode parentWritingMode) noexcept
  {
    if (gridItem.Style().HasAspectRatio())
    {
      // FIXME: should align-items + align-self: auto/justify-items + justify-self: auto be taken into
      // account?
      if (parentWritingMode.IsHorizontal() == gridItem.IsHorizontalWritingMode()
          && !gridItem.Style().GetAlignSelf().IsStretch())
      {
        // A non-auto inline size means the same for block size (column axis size) because of the aspect
        // ratio.
        if (!gridItem.Style().LogicalWidth().IsAuto())
        {
          return false;
        }
      }
      else if (!gridItem.Style().GetJustifySelf().IsStretch())
      {
        auto &logicalHeight = gridItem.Style().LogicalHeight();
        if (logicalHeight.IsFixed()
            || (logicalHeight.IsPercentOrCalculated() && gridItem.PercentageLogicalHeightIsResolvable()))
        {
          return false;
        }
      }
    }
    return parentWritingMode.IsHorizontal() ? gridItem.Style().Height().IsAuto()
                                            : gridItem.Style().Width().IsAuto();
  }

  bool HasAutoSizeInRowAxis(const RenderBox &gridItem, WritingMode parentWritingMode) noexcept
  {
    if (gridItem.Style().HasAspectRatio())
    {
      // FIXME: should align-items + align-self: auto/justify-items + justify-self: auto be taken into
      // account?
      if (parentWritingMode.IsHorizontal() == gridItem.IsHorizontalWritingMode()
          && !gridItem.Style().GetAlignSelf().IsStretch())
      {
        // A non-auto block size means the same for inline size (row axis size) because of the aspect ratio.
        auto &logicalHeight = gridItem.Style().LogicalHeight();
        if (logicalHeight.IsFixed()
            || (logicalHeight.IsPercentOrCalculated() && gridItem.PercentageLogicalHeightIsResolvable()))
        {
          return false;
        }
      }
      else if (!gridItem.Style().GetJustifySelf().IsStretch())
      {
        if (!gridItem.Style().LogicalWidth().IsAuto())
        {
          return false;
        }
      }
    }
    return parentWritingMode.IsHorizontal() ? gridItem.Style().Width().IsAuto()
                                            : gridItem.Style().Height().IsAuto();
  }

  bool AllowedToStretchGridItemAlongColumnAxis(const RenderBox &gridItem, ItemPosition alignSelf,
                                               WritingMode writingMode) noexcept
  {
    return alignSelf == ItemPosition::Stretch && HasAutoSizeInColumnAxis(gridItem, writingMode)
           && !HasAutoMarginsInColumnAxis(gridItem, writingMode);
  }

  bool AllowedToStretchGridItemAlongRowAxis(const RenderBox &gridItem, ItemPosition justifySelf,
                                            WritingMode writingMode) noexcept
  {
    return justifySelf == ItemPosition::Stretch && HasAutoSizeInRowAxis(gridItem, writingMode)
           && !HasAutoMarginsInRowAxis(gridItem, writingMode);
  }

  LayoutUnit AvailableAlignmentSpaceForGridItemBeforeStretching(const RenderGrid &grid,
                                                                LayoutUnit gridAreaBreadthForGridItem,
                                                                const RenderBox &gridItem,
                                                                GridTrackSizingDirection direction) noexcept
  {
    // Because we want to avoid multiple layouts, stretching logic might be performed before
    // grid items are laid out, so we can't use the grid item cached values. Hence, we need to
    // compute margins in order to determine the available height before stretching.
    auto gridItemFlowDirection = FlowAwareDirectionForGridItem(grid, gridItem, direction);
    return std::max(0_lu, gridAreaBreadthForGridItem
                            - MarginLogicalSizeForGridItem(grid, gridItemFlowDirection, gridItem));
  }

  void UpdateAutoMarginsIfNeeded(RenderBox &gridItem, WritingMode writingMode) noexcept
  {
    UpdateAutoMarginsInRowAxisIfNeeded(gridItem, writingMode);
    UpdateAutoMarginsInColumnAxisIfNeeded(gridItem, writingMode);
  }

  void UpdateAutoMarginsInRowAxisIfNeeded(RenderBox &gridItem, WritingMode writingMode) noexcept
  {
    krys_debug_assert(!gridItem.IsOutOfFlowPositioned());

    auto &marginStart = gridItem.Style().MarginStart(writingMode);
    auto &marginEnd = gridItem.Style().MarginEnd(writingMode);
    LayoutUnit marginLogicalWidth;
    // We should only consider computed margins if their specified value isn't
    // 'auto', since such computed value may come from a previous layout and may
    // be incorrect now.
    if (!marginStart.IsAuto())
    {
      marginLogicalWidth += gridItem.MarginStart();
    }
    if (!marginEnd.IsAuto())
    {
      marginLogicalWidth += gridItem.MarginEnd();
    }

    auto availableAlignmentSpace =
      gridItem.GridAreaContentLogicalWidth()->value() - gridItem.LogicalWidth() - marginLogicalWidth;
    if (availableAlignmentSpace <= 0)
    {
      return;
    }

    if (marginStart.IsAuto() && marginEnd.IsAuto())
    {
      gridItem.SetMarginStart(availableAlignmentSpace / 2, writingMode);
      gridItem.SetMarginEnd(availableAlignmentSpace / 2, writingMode);
    }
    else if (marginStart.IsAuto())
    {
      gridItem.SetMarginStart(availableAlignmentSpace, writingMode);
    }
    else if (marginEnd.IsAuto())
    {
      gridItem.SetMarginEnd(availableAlignmentSpace, writingMode);
    }
  }

  void UpdateAutoMarginsInColumnAxisIfNeeded(RenderBox &gridItem, WritingMode writingMode) noexcept
  {
    krys_debug_assert(!gridItem.IsOutOfFlowPositioned());

    auto &marginBefore = gridItem.Style().MarginBefore(writingMode);
    auto &marginAfter = gridItem.Style().MarginAfter(writingMode);
    LayoutUnit marginLogicalHeight;
    // We should only consider computed margins if their specified value isn't
    // 'auto', since such computed value may come from a previous layout and may
    // be incorrect now.
    if (!marginBefore.IsAuto())
    {
      marginLogicalHeight += gridItem.MarginBefore();
    }
    if (!marginAfter.IsAuto())
    {
      marginLogicalHeight += gridItem.MarginAfter();
    }

    auto availableAlignmentSpace =
      gridItem.GridAreaContentLogicalHeight()->value() - gridItem.LogicalHeight() - marginLogicalHeight;
    if (availableAlignmentSpace <= 0)
    {
      return;
    }

    if (marginBefore.IsAuto() && marginAfter.IsAuto())
    {
      gridItem.SetMarginBefore(availableAlignmentSpace / 2, writingMode);
      gridItem.SetMarginAfter(availableAlignmentSpace / 2, writingMode);
    }
    else if (marginBefore.IsAuto())
    {
      gridItem.SetMarginBefore(availableAlignmentSpace, writingMode);
    }
    else if (marginAfter.IsAuto())
    {
      gridItem.SetMarginAfter(availableAlignmentSpace, writingMode);
    }
  }

  bool IsRelativeGridTrackBreadthAsAuto(const style::GridTrackFitContentLength &length,
                                        Maybe<LayoutUnit> availableSpace) noexcept
  {
    return length.IsPercentOrCalculated() && !availableSpace;
  }

  bool IsRelativeGridTrackBreadthAsAuto(const style::GridTrackBreadth &length,
                                        Maybe<LayoutUnit> availableSpace) noexcept
  {
    return length.IsPercentOrCalculated() && !availableSpace;
  }
}
