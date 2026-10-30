#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Rendering/BaselineAlignment.hpp"
#include "Krystal.Booey/Rendering/GridLayoutFunctions.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderBox;

  // This is the class that implements the Baseline Alignment logic, using internally the
  // BaselineAlignmentState and BaselineGroup classes.
  //
  // The first phase is to collect the items that will participate in baseline alignment together. During this
  // phase the required baseline-sharing groups will be created for each Baseline alignment-context shared by
  // the items participating in the baseline alignment.
  //
  // Additionally, the baseline-sharing groups' offsets, max-ascend and max-descent will be computed and
  // stored. This class also computes the baseline offset for a particular item, based on the max-ascent for
  // its associated baseline-sharing group.
  /// @brief The baseline alignment contexts of a grid's rows and columns.
  class GridBaselineAlignment
  {
  private:
    using BaselineAlignmentStateMap = HashMap<uint32, UniquePtr<BaselineAlignmentState>>;

    // Grid Container's writing mode, used to determine grid item's orthogonality.
    WritingMode _writingMode;
    BaselineAlignmentStateMap _rowAlignmentContextStates;
    BaselineAlignmentStateMap _columnAlignmentContextStates;

  public:
    // Collects the items participating in baseline alignment and updates the corresponding baseline-sharing
    // group of the Baseline Context the items belongs to.
    // All the baseline offsets are updated accordingly based on the added item.
    void UpdateBaselineAlignmentContext(ItemPosition preference, uint32 sharedContext,
                                        const RenderBox &gridItem,
                                        style::GridTrackSizingDirection alignmentContextType) noexcept;

    // Returns the baseline offset of a particular item, based on the max-ascent for its associated
    // baseline-sharing group
    KRYS_NODISCARD LayoutUnit
      BaselineOffsetForGridItem(ItemPosition preference, uint32 sharedContext, const RenderBox &gridItem,
                                style::GridTrackSizingDirection alignmentContextType) const noexcept;

    // Sets the Grid Container's writing mode so that we can avoid the dependecy of the LayoutGrid class for
    // determining whether a grid item is orthogonal or not.
    void SetWritingMode(WritingMode writingMode) noexcept
    {
      _writingMode = writingMode;
    }

    // Clearing the Baseline Alignment context and their internal classes and data structures.
    void Clear(style::GridTrackSizingDirection alignmentContextType) noexcept;

  private:
    KRYS_NODISCARD const BaselineGroup &
      BaselineGroupForGridItem(ItemPosition preference, uint32 sharedContext, const RenderBox &gridItem,
                               style::GridTrackSizingDirection alignmentContextType) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalAscentForGridItem(const RenderBox &gridItem,
                                                       style::GridTrackSizingDirection alignmentContextType,
                                                       ItemPosition position) const noexcept;
    KRYS_NODISCARD LayoutUnit AscentForGridItem(const RenderBox &gridItem,
                                                style::GridTrackSizingDirection alignmentContextType,
                                                ItemPosition position) const noexcept;
    KRYS_NODISCARD LayoutUnit DescentForGridItem(
      const RenderBox &gridItem, LayoutUnit ascent, style::GridTrackSizingDirection alignmentContextType,
      ExtraMarginsFromSubgrids extraMarginsFromAncestorSubgrids) const noexcept;
    KRYS_NODISCARD bool
      IsDescentBaselineForGridItem(const RenderBox &gridItem,
                                   style::GridTrackSizingDirection alignmentContextType) const noexcept;
    KRYS_NODISCARD bool
      IsVerticalAlignmentContext(style::GridTrackSizingDirection alignmentContextType) const noexcept;
    KRYS_NODISCARD bool IsOrthogonalGridItemForBaseline(const RenderBox &gridItem) const noexcept;
    KRYS_NODISCARD bool IsParallelToAlignmentAxisForGridItem(
      const RenderBox &gridItem, style::GridTrackSizingDirection alignmentContextType) const noexcept;
  };
}
