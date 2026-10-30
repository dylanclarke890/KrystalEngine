#include "Krystal.Booey/Rendering/GridBaselineAlignment.hpp"
#include "Krystal.Booey/Rendering/AncestorSubgridIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  using style::GridTrackSizingDirection;

  LayoutUnit GridBaselineAlignment::LogicalAscentForGridItem(const RenderBox &gridItem,
                                                             GridTrackSizingDirection alignmentContextType,
                                                             ItemPosition position) const noexcept
  {
    auto hasOrthogonalAncestorSubgrids = [&]
    {
      for (auto &currentAncestorSubgrid :
           AncestorSubgridsOfGridItem(gridItem, GridTrackSizingDirection::Rows))
      {
        if (currentAncestorSubgrid.IsHorizontalWritingMode()
            != currentAncestorSubgrid.Parent()->IsHorizontalWritingMode())
        {
          return true;
        }
      }
      return false;
    };

    ExtraMarginsFromSubgrids extraMarginsFromAncestorSubgrids;
    if (alignmentContextType == GridTrackSizingDirection::Rows && !hasOrthogonalAncestorSubgrids())
    {
      extraMarginsFromAncestorSubgrids =
        GridLayoutFunctions::ExtraMarginForSubgridAncestors(GridTrackSizingDirection::Rows, gridItem);
    }

    LayoutUnit ascent = AscentForGridItem(gridItem, alignmentContextType, position)
                        + extraMarginsFromAncestorSubgrids.ExtraTrackStartMargin();
    return (IsDescentBaselineForGridItem(gridItem, alignmentContextType)
            || position == ItemPosition::LastBaseline)
             ? DescentForGridItem(gridItem, ascent, alignmentContextType, extraMarginsFromAncestorSubgrids)
             : ascent;
  }

  LayoutUnit GridBaselineAlignment::AscentForGridItem(const RenderBox &gridItem,
                                                      GridTrackSizingDirection alignmentContextType,
                                                      ItemPosition position) const noexcept
  {
    krys_debug_assert(position == ItemPosition::Baseline || position == ItemPosition::LastBaseline);
    auto gridItemMargin = alignmentContextType == GridTrackSizingDirection::Rows
                            ? gridItem.MarginBefore(_writingMode)
                            : gridItem.MarginStart(_writingMode);
    auto &gridStyle = gridItem.Parent()->Style();

    auto baseline = 0_lu;
    if (alignmentContextType == GridTrackSizingDirection::Rows)
    {
      auto alignmentContextDirection = [&]
      {
        return gridStyle.GetWritingMode().IsHorizontal() ? LineDirection::Horizontal
                                                         : LineDirection::Vertical;
      };

      if (!IsParallelToAlignmentAxisForGridItem(gridItem, alignmentContextType))
      {
        auto gridWritingMode = gridStyle.GetWritingMode();
        return gridItemMargin
               + BaselineAlignmentState::SynthesizedBaseline(
                 gridItem, BaselineAlignmentState::DominantBaseline(gridWritingMode), gridWritingMode,
                 alignmentContextDirection(), BaselineSynthesisEdge::BorderBox);
      }
      auto ascent =
        position == ItemPosition::Baseline ? gridItem.FirstLineBaseline() : gridItem.LastLineBaseline();
      if (!ascent)
      {
        auto gridWritingMode = gridStyle.GetWritingMode();
        return gridItemMargin
               + BaselineAlignmentState::SynthesizedBaseline(
                 gridItem, BaselineAlignmentState::DominantBaseline(gridWritingMode), gridWritingMode,
                 alignmentContextDirection(), BaselineSynthesisEdge::BorderBox);
      }
      baseline = *ascent;
    }
    else
    {
      auto firstOrLastLineBaseline = Maybe<LayoutUnit> {};
      if (IsParallelToAlignmentAxisForGridItem(gridItem, alignmentContextType))
      {
        firstOrLastLineBaseline =
          position == ItemPosition::Baseline ? gridItem.FirstLineBaseline() : gridItem.LastLineBaseline();
      }

      // We take border-box's under edge if no valid baseline.
      if (!firstOrLastLineBaseline)
      {
        krys_debug_assert(!gridItem.NeedsLayout());
        if (IsVerticalAlignmentContext(alignmentContextType))
        {
          return _writingMode.IsBlockFlipped() ? gridItemMargin + gridItem.Size().Width().ToInt()
                                               : gridItemMargin;
        }
        auto gridWritingMode = gridStyle.GetWritingMode();
        return gridItemMargin
               + BaselineAlignmentState::SynthesizedBaseline(
                 gridItem, BaselineAlignmentState::DominantBaseline(gridWritingMode), gridWritingMode,
                 LineDirection::Horizontal, BaselineSynthesisEdge::BorderBox);
      }
      baseline = *firstOrLastLineBaseline;
    }

    return gridItemMargin + baseline;
  }

  LayoutUnit GridBaselineAlignment::DescentForGridItem(
    const RenderBox &gridItem, LayoutUnit ascent, GridTrackSizingDirection alignmentContextType,
    ExtraMarginsFromSubgrids extraMarginsFromAncestorSubgrids) const noexcept
  {
    krys_debug_assert(!gridItem.NeedsLayout());
    if (IsParallelToAlignmentAxisForGridItem(gridItem, alignmentContextType))
    {
      return extraMarginsFromAncestorSubgrids.ExtraTotalMargin() + gridItem.MarginLogicalHeight()
             + gridItem.LogicalHeight() - ascent;
    }
    return gridItem.MarginLogicalWidth() + gridItem.LogicalWidth() - ascent;
  }

  bool GridBaselineAlignment::IsDescentBaselineForGridItem(
    const RenderBox &gridItem, GridTrackSizingDirection alignmentContextType) const noexcept
  {
    return IsVerticalAlignmentContext(alignmentContextType)
           && ((gridItem.GetWritingMode().IsBlockFlipped() && !_writingMode.IsBlockFlipped())
               || (gridItem.GetWritingMode().IsLineInverted() && _writingMode.IsBlockFlipped()));
  }

  bool GridBaselineAlignment::IsVerticalAlignmentContext(
    GridTrackSizingDirection alignmentContextType) const noexcept
  {
    return (alignmentContextType == GridTrackSizingDirection::Columns) == _writingMode.IsHorizontal();
  }

  bool GridBaselineAlignment::IsOrthogonalGridItemForBaseline(const RenderBox &gridItem) const noexcept
  {
    return _writingMode.IsOrthogonal(gridItem.GetWritingMode());
  }

  bool GridBaselineAlignment::IsParallelToAlignmentAxisForGridItem(
    const RenderBox &gridItem, GridTrackSizingDirection alignmentContextType) const noexcept
  {
    return alignmentContextType == GridTrackSizingDirection::Rows ? !IsOrthogonalGridItemForBaseline(gridItem)
                                                                  : IsOrthogonalGridItemForBaseline(gridItem);
  }

  const BaselineGroup &GridBaselineAlignment::BaselineGroupForGridItem(
    ItemPosition preference, uint32 sharedContext, const RenderBox &gridItem,
    GridTrackSizingDirection alignmentContextType) const noexcept
  {
    krys_debug_assert(IsBaselinePosition(preference));
    auto &baselineAlignmentStateMap = alignmentContextType == GridTrackSizingDirection::Rows
                                        ? _rowAlignmentContextStates
                                        : _columnAlignmentContextStates;
    auto entry = baselineAlignmentStateMap.find(sharedContext);
    krys_debug_assert(entry != baselineAlignmentStateMap.end());
    return entry->second->SharedGroup(gridItem, preference);
  }

  void GridBaselineAlignment::UpdateBaselineAlignmentContext(
    ItemPosition preference, uint32 sharedContext, const RenderBox &gridItem,
    GridTrackSizingDirection alignmentContextType) noexcept
  {
    krys_debug_assert(IsBaselinePosition(preference));
    krys_debug_assert(!gridItem.NeedsLayout());

    // Determine Ascent and Descent values of this grid item with respect to
    // its grid container.
    LayoutUnit ascent = LogicalAscentForGridItem(gridItem, alignmentContextType, preference);
    // Looking up for a shared alignment context perpendicular to the
    // alignment axis.
    auto &baselineAlignmentStateMap = alignmentContextType == GridTrackSizingDirection::Rows
                                        ? _rowAlignmentContextStates
                                        : _columnAlignmentContextStates;
    // Looking for a compatible baseline-sharing group.
    auto &state = baselineAlignmentStateMap[sharedContext];
    if (!state)
    {
      auto alignmentAxis = alignmentContextType == GridTrackSizingDirection::Columns ? LogicalBoxAxis::Block
                                                                                     : LogicalBoxAxis::Inline;
      state = CreateUnique<BaselineAlignmentState>(gridItem, preference, ascent, alignmentAxis, _writingMode);
    }
    state->UpdateSharedGroup(gridItem, preference, ascent);
  }

  LayoutUnit GridBaselineAlignment::BaselineOffsetForGridItem(
    ItemPosition preference, uint32 sharedContext, const RenderBox &gridItem,
    GridTrackSizingDirection alignmentContextType) const noexcept
  {
    krys_debug_assert(IsBaselinePosition(preference));
    auto &group = BaselineGroupForGridItem(preference, sharedContext, gridItem, alignmentContextType);
    if (group.ComputeSize() > 1)
    {
      return group.MaxAscent() - LogicalAscentForGridItem(gridItem, alignmentContextType, preference);
    }
    return LayoutUnit();
  }

  void GridBaselineAlignment::Clear(GridTrackSizingDirection alignmentContextType) noexcept
  {
    if (alignmentContextType == GridTrackSizingDirection::Rows)
    {
      _rowAlignmentContextStates.clear();
    }
    else
    {
      _columnAlignmentContextStates.clear();
    }
  }
}
