#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingUtils.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include <cmath>

namespace krys::boo::layout
{
  struct FlexBaseAndHypotheticalMainSize
  {
    LayoutUnit FlexBase {0.f};
    LayoutUnit HypotheticalMainSize {0.f};
  };

  struct PositionAndMargins
  {
    KRYS_NODISCARD LayoutUnit Margin() const noexcept
    {
      return MarginStart + MarginEnd;
    }

    LayoutUnit Position;
    LayoutUnit MarginStart;
    LayoutUnit MarginEnd;
  };

  namespace
  {
    KRYS_NODISCARD LayoutUnit OuterMainSize(const LogicalFlexItem &flexItem, LayoutUnit mainSize,
                                            Maybe<LayoutUnit> usedMargin = {}) noexcept
    {
      auto outerMainSize = usedMargin.value_or(flexItem.MainAxis().Margin());
      if (flexItem.IsContentBoxBased())
      {
        outerMainSize += flexItem.MainAxis().BorderAndPadding;
      }
      outerMainSize += mainSize;
      return outerMainSize;
    }

    KRYS_NODISCARD LayoutUnit OuterCrossSize(const LogicalFlexItem &flexItem, LayoutUnit crossSize,
                                             Maybe<LayoutUnit> usedMargin = {}) noexcept
    {
      auto outerCrossSize = usedMargin.value_or(flexItem.CrossAxis().Margin());
      if (flexItem.IsContentBoxBased())
      {
        outerCrossSize += flexItem.CrossAxis().BorderAndPadding;
      }
      outerCrossSize += crossSize;
      return outerCrossSize;
    }
  }

  FlexLayout::FlexLayout(FlexFormattingContext &flexFormattingContext) noexcept
      : _flexFormattingContext(flexFormattingContext)
  {
  }

  FlexLayout::LogicalFlexItemRects
    FlexLayout::Layout(const ConstraintsForFlexContent &flexContainerConstraints,
                       const LogicalFlexItems &flexItems) noexcept
  {
    // This follows https://www.w3.org/TR/css-flexbox-1/#layout-algorithm
    SizeList flexItemsMainSizeList(flexItems.size());
    SizeList flexItemsCrossSizeList(flexItems.size());
    LinesCrossSizeList flexLinesCrossSizeList;
    auto lineRanges = LineRanges {};

    auto performContentSizing = [&]
    {
      auto needsMainAxisLayout = true;

      while (needsMainAxisLayout)
      {
        auto performMainAxisSizing = [&]
        {
          // 9.2. (#3) Determine the flex base size and hypothetical main size of each item
          auto flexBaseAndHypotheticalMainSizeList = FlexBaseAndHypotheticalMainSizeForFlexItems(
            flexItems, flexContainerConstraints.IsSizedUnderMinMax());
          // 9.2. (#4) Determine the main size of the flex container
          auto flexContainerInnerMainSize = FlexContainerInnerMainSize(flexContainerConstraints.MainAxis());
          // 9.3. (#5) Collect flex items into flex lines
          lineRanges =
            ComputeFlexLines(flexItems, flexContainerInnerMainSize, flexBaseAndHypotheticalMainSizeList);
          // 9.3. (#6) Resolve the flexible lengths of all the flex items to find their used main size
          flexItemsMainSizeList = ComputeMainSizeForFlexItems(
            flexItems, lineRanges, flexContainerInnerMainSize, flexBaseAndHypotheticalMainSizeList);
        };
        performMainAxisSizing();

        auto performCrossAxisSizing = [&]
        {
          // 9.4. (#7) Determine the hypothetical cross size of each item
          auto hypotheticalCrossSizeList =
            HypotheticalCrossSizeForFlexItems(flexItems, flexItemsMainSizeList);
          // 9.4. (#8) Calculate the cross size of each flex line
          flexLinesCrossSizeList = CrossSizeForFlexLines(lineRanges, flexContainerConstraints.CrossAxis(),
                                                         flexItems, hypotheticalCrossSizeList);
          // 9.4. (#9) Handle 'align-content: stretch
          StretchFlexLines(flexLinesCrossSizeList, lineRanges.size(),
                           flexContainerConstraints.CrossAxis().AvailableSize);
          // 9.4. (#10) Collapse visibility:collapse items
          auto collapsedContentNeedsSecondLayout = CollapseNonVisibleFlexItems();
          if (collapsedContentNeedsSecondLayout)
          {
            return;
          }
          // 9.4. (#11) Determine the used cross size of each flex item
          flexItemsCrossSizeList = ComputeCrossSizeForFlexItems(flexItems, lineRanges, flexLinesCrossSizeList,
                                                                hypotheticalCrossSizeList);
          needsMainAxisLayout = false;
        };
        performCrossAxisSizing();
      }
    };
    performContentSizing();

    auto mainPositionAndMargins = PositionAndMarginsList(flexItems.size());
    auto crossPositionAndMargins = PositionAndMarginsList(flexItems.size());
    auto linesCrossPositionList = LinesCrossPositionList {};

    auto performContentAlignment = [&]
    {
      // 9.5. (#12) Main-Axis Alignment
      mainPositionAndMargins =
        HandleMainAxisAlignment(FlexContainerInnerMainSize(flexContainerConstraints.MainAxis()), lineRanges,
                                flexItems, flexItemsMainSizeList);
      // 9.6. (#13 - #16) Cross-Axis Alignment
      crossPositionAndMargins = HandleCrossAxisAlignmentForFlexItems(
        flexItems, lineRanges, flexItemsCrossSizeList, flexLinesCrossSizeList);
      linesCrossPositionList = HandleCrossAxisAlignmentForFlexLines(
        flexContainerConstraints.CrossAxis().AvailableSize, lineRanges, flexLinesCrossSizeList);
    };
    performContentAlignment();

    auto computeFlexItemRects = [&]
    {
      auto flexRects = LogicalFlexItemRects(flexItems.size());
      auto mainAxisGapValue = FlexFormattingUtils::MainAxisGapValue(
        FlexContainer(), flexContainerConstraints.MainAxis().AvailableSize.value_or(0_lu));
      auto crossAxisGapValue = FlexFormattingUtils::CrossAxisGapValue(
        FlexContainer(), flexContainerConstraints.CrossAxis().AvailableSize.value_or(0_lu));
      for (size_t lineIndex = 0; lineIndex < lineRanges.size(); ++lineIndex)
      {
        auto lineRange = lineRanges[lineIndex];
        for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
        {
          auto flexItemMainPosition =
            mainPositionAndMargins[flexItemIndex].Position
            + static_cast<int32>(flexItemIndex - lineRange.Begin()) * mainAxisGapValue;
          auto flexItemCrossPosition = linesCrossPositionList[lineIndex]
                                       + crossPositionAndMargins[flexItemIndex].Position
                                       + static_cast<int32>(lineIndex) * crossAxisGapValue;

          flexRects[flexItemIndex] = {LayoutRect {flexItemMainPosition, flexItemCrossPosition,
                                                  flexItemsMainSizeList[flexItemIndex],
                                                  flexItemsCrossSizeList[flexItemIndex]},
                                      {mainPositionAndMargins[flexItemIndex].MarginStart,
                                       mainPositionAndMargins[flexItemIndex].MarginEnd},
                                      {crossPositionAndMargins[flexItemIndex].MarginStart,
                                       crossPositionAndMargins[flexItemIndex].MarginEnd}};
        }
      }
      return flexRects;
    };
    return computeFlexItemRects();
  }

  FlexLayout::FlexBaseAndHypotheticalMainSizeList
    FlexLayout::FlexBaseAndHypotheticalMainSizeForFlexItems(const LogicalFlexItems &flexItems,
                                                            bool isSizedUnderMinMaxConstraints) const noexcept
  {
    auto flexBaseAndHypotheticalMainSizeList = FlexBaseAndHypotheticalMainSizeList {};
    for (auto &flexItem : flexItems)
    {
      // 3. Determine the flex base size and hypothetical main size of each item:
      auto computedFlexBase = [&]() -> LayoutUnit
      {
        // A. If the item has a definite used flex basis, that's the flex base size.
        if (auto definiteFlexBase = flexItem.MainAxis().DefiniteFlexBasis)
        {
          return *definiteFlexBase;
        }
        // B. If the flex item has...
        if (flexItem.HasAspectRatio() && flexItem.HasContentFlexBasis() && flexItem.CrossAxis().DefiniteSize)
        {
          // The flex base size is calculated from its inner cross size and the flex item's intrinsic aspect
          // ratio.
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return {};
        }
        // C. If the used flex basis is content or depends on its available space, and the flex container is
        // being sized under a min-content or max-content constraint, size the item under that constraint
        auto flexBasisContentOrAvailableSpaceDependent =
          flexItem.HasContentFlexBasis() || flexItem.HasAvailableSpaceDependentFlexBasis();
        if (flexBasisContentOrAvailableSpaceDependent && isSizedUnderMinMaxConstraints)
        {
          // Compute flex item's main size.
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return {};
        }
        // D. If the used flex basis is content or depends on its available space, the available main size is
        // infinite, and the flex item's inline axis is parallel to the main axis, lay the item out using the
        // rules for a box in an orthogonal flow. The flex base size is the item's max-content main size.
        if (flexBasisContentOrAvailableSpaceDependent && flexItem.IsOrthogonal())
        {
          // Lay the item out using the rules for a box in an orthogonal flow. The flex base size is the
          // item's max-content main size.
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return {};
        }
        // E Otherwise, size the item into the available space using its used flex basis in place of its main
        // size, treating a value of content as max-content. If a cross size is needed to determine the main
        // size (e.g. when the flex item's main size is in its block axis) and the flex item's cross size is
        // auto and not definite, in this calculation use fit-content as the flex item's cross size. The flex
        // base size is the item's resulting main size.
        if (flexItem.IsOrthogonal() && !flexItem.CrossAxis().DefiniteSize)
        {
          // FIXME: Not implemented yet.
          krys_debug_assert(false);
          return {};
        }
        return FormattingUtils().UsedMaxContentSizeInMainAxis(flexItem);
      };
      auto flexBaseSize = computedFlexBase();
      // The hypothetical main size is the item's flex base size clamped according to its used min and max
      // main sizes (and flooring the content box size at zero).
      auto hypotheticalMainSize =
        std::max(FormattingUtils().UsedMinimumSizeInMainAxis(flexItem), flexBaseSize);
      if (auto usedMaximumMainSize = FormattingUtils().UsedMaximumSizeInMainAxis(flexItem))
      {
        hypotheticalMainSize = std::min(*usedMaximumMainSize, hypotheticalMainSize);
      }
      flexBaseAndHypotheticalMainSizeList.push_back({flexBaseSize, hypotheticalMainSize});
    }
    return flexBaseAndHypotheticalMainSizeList;
  }

  LayoutUnit FlexLayout::FlexContainerInnerMainSize(
    const ConstraintsForFlexContent::AxisGeometry &mainAxisGeometry) const noexcept
  {
    // 4. Determine the main size of the flex container using the rules of the formatting context in which it
    // participates.
    // FIXME: above.
    return mainAxisGeometry.AvailableSize.value_or(LayoutUnit::Max());
  }

  FlexLayout::LineRanges FlexLayout::ComputeFlexLines(
    const LogicalFlexItems &flexItems, LayoutUnit flexContainerInnerMainSize,
    const FlexBaseAndHypotheticalMainSizeList &flexBaseAndHypotheticalMainSizeList) const noexcept
  {
    // Collect flex items into flex lines:
    // If the flex container is single-line, collect all the flex items into a single flex line.
    // Otherwise, starting from the first uncollected item, collect consecutive items one by one until the
    // first time that the next collected item would not fit into the flex container's inner main size. If the
    // very first uncollected item wouldn't fit, collect just it into the line. For this step, the size of a
    // flex item is its outer hypothetical main size. (Note: This can be negative.)
    if (IsSingleLineFlexContainer())
    {
      return {IndexRange {0, flexBaseAndHypotheticalMainSizeList.size()}};
    }

    auto mainAxisGapValue =
      FlexFormattingUtils::MainAxisGapValue(FlexContainer(), flexContainerInnerMainSize);
    auto lineRanges = LineRanges {};
    size_t lastWrapIndex = 0;
    auto flexItemsMainSize = LayoutUnit {};
    for (size_t flexItemIndex = 0; flexItemIndex < flexBaseAndHypotheticalMainSizeList.size();
         ++flexItemIndex)
    {
      auto flexItemHypotheticalOuterMainSize = OuterMainSize(
        flexItems[flexItemIndex], flexBaseAndHypotheticalMainSizeList[flexItemIndex].HypotheticalMainSize);
      auto numberOfFlexItemsOnLine = flexItemIndex - lastWrapIndex;
      auto mainAxisGapSize = mainAxisGapValue * static_cast<int32>(numberOfFlexItemsOnLine);
      if (!numberOfFlexItemsOnLine
          || flexItemsMainSize + flexItemHypotheticalOuterMainSize + mainAxisGapSize
               <= flexContainerInnerMainSize)
      {
        flexItemsMainSize += flexItemHypotheticalOuterMainSize;
        continue;
      }
      lineRanges.push_back({lastWrapIndex, flexItemIndex});
      flexItemsMainSize = flexItemHypotheticalOuterMainSize;
      lastWrapIndex = flexItemIndex;
    }
    lineRanges.push_back({lastWrapIndex, flexBaseAndHypotheticalMainSizeList.size()});
    return lineRanges;
  }

  FlexLayout::SizeList FlexLayout::ComputeMainSizeForFlexItems(
    const LogicalFlexItems &flexItems, const LineRanges &lineRanges, LayoutUnit flexContainerInnerMainSize,
    const FlexBaseAndHypotheticalMainSizeList &flexBaseAndHypotheticalMainSizeList) const noexcept
  {
    SizeList mainSizeList(flexItems.size());

    for (auto lineRange : lineRanges)
    {
      auto nonFrozenSet = ListHashSet<size_t> {};
      auto availableMainSpaceForLineContent =
        MainAxisAvailableSpaceForItemAlignment(flexContainerInnerMainSize, lineRange.Distance());

      // 1. Determine the used flex factor. Sum the outer hypothetical main sizes of all items on the line.
      //    If the sum is less than the flex container's inner main size, use the flex grow factor for the
      //    rest of this algorithm; otherwise, use the flex shrink factor.
      auto shouldUseFlexGrowFactor = [&]
      {
        auto hypotheticalOuterMainSizes = LayoutUnit {};
        for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
        {
          auto flexItemHypotheticalOuterMainSize =
            OuterMainSize(flexItems[flexItemIndex],
                          flexBaseAndHypotheticalMainSizeList[flexItemIndex].HypotheticalMainSize);
          hypotheticalOuterMainSizes += flexItemHypotheticalOuterMainSize;
        }
        return hypotheticalOuterMainSizes < availableMainSpaceForLineContent;
      }();

      // 2. Size inflexible items. Freeze, setting its target main size to its hypothetical main size.
      //    any item that has a flex factor of zero
      //    if using the flex grow factor: any item that has a flex base size greater than its hypothetical
      //    main size if using the flex shrink factor: any item that has a flex base size smaller than its
      //    hypothetical main size
      for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
      {
        auto shouldFreeze = [&]
        {
          auto usedFlexFactor = shouldUseFlexGrowFactor ? flexItems[flexItemIndex].GrowFactor()
                                                        : flexItems[flexItemIndex].ShrinkFactor();
          if (!usedFlexFactor)
          {
            return true;
          }
          auto flexBaseAndHypotheticalMainSize = flexBaseAndHypotheticalMainSizeList[flexItemIndex];
          if (shouldUseFlexGrowFactor
              && flexBaseAndHypotheticalMainSize.FlexBase
                   > flexBaseAndHypotheticalMainSize.HypotheticalMainSize)
          {
            return true;
          }
          if (!shouldUseFlexGrowFactor
              && flexBaseAndHypotheticalMainSize.FlexBase
                   < flexBaseAndHypotheticalMainSize.HypotheticalMainSize)
          {
            return true;
          }
          return false;
        };
        if (shouldFreeze())
        {
          mainSizeList[flexItemIndex] =
            flexBaseAndHypotheticalMainSizeList[flexItemIndex].HypotheticalMainSize;
          continue;
        }
        nonFrozenSet.add(flexItemIndex);
      }

      // 3. Calculate initial free space. Sum the outer sizes of all items on the line, and subtract this from
      // the flex container's inner main size.
      //    For frozen items, use their outer target main size; for other items, use their outer flex base
      //    size.
      auto computedFreeSpace = [&]
      {
        auto lineContentMainSize = LayoutUnit {};
        for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
        {
          auto flexItemOuterMainSize = OuterMainSize(
            flexItems[flexItemIndex], nonFrozenSet.contains(flexItemIndex)
                                        ? flexBaseAndHypotheticalMainSizeList[flexItemIndex].FlexBase
                                        : mainSizeList[flexItemIndex]);
          lineContentMainSize += flexItemOuterMainSize;
        }
        return availableMainSpaceForLineContent - lineContentMainSize;
      };
      auto initialFreeSpace = computedFreeSpace();

      auto minimumViolationList = List<size_t> {};
      auto maximumViolationList = List<size_t> {};
      minimumViolationList.reserve(flexItems.size());
      maximumViolationList.reserve(flexItems.size());
      // 4. Loop:
      while (true)
      {
        // a. Check for flexible items. If all the flex items on the line are frozen, free space has been
        // distributed; exit this loop.
        if (nonFrozenSet.empty())
        {
          break;
        }

        // b. Calculate the remaining free space as for initial free space, above. If the sum of the unfrozen
        // flex items' flex factors
        //    is less than one, multiply the initial free space by this sum. If the magnitude of this value is
        //    less than the magnitude of the remaining free space, use this as the remaining free space.
        auto freeSpace = computedFreeSpace();
        auto adjustFreeSpaceWithFlexFactors = [&]
        {
          auto totalFlexFactor = 0.f;
          for (auto nonFrozenIndex : nonFrozenSet)
          {
            totalFlexFactor += shouldUseFlexGrowFactor ? flexItems[nonFrozenIndex].GrowFactor()
                                                       : flexItems[nonFrozenIndex].ShrinkFactor();
          }
          if (totalFlexFactor < 1)
          {
            auto freeSpaceCandidate = LayoutUnit {initialFreeSpace.ToFloat() * totalFlexFactor};
            if (freeSpaceCandidate.Abs() < freeSpace.Abs())
            {
              freeSpace = freeSpaceCandidate;
            }
          }
        };
        adjustFreeSpaceWithFlexFactors();

        // c. Distribute free space proportional to the flex factors.
        auto usedTotalFactor = 0.f;
        for (auto nonFrozenIndex : nonFrozenSet)
        {
          usedTotalFactor += shouldUseFlexGrowFactor
                               ? flexItems[nonFrozenIndex].GrowFactor()
                               : flexItems[nonFrozenIndex].ShrinkFactor()
                                   * flexBaseAndHypotheticalMainSizeList[nonFrozenIndex].FlexBase.ToFloat();
        }

        for (auto nonFrozenIndex : nonFrozenSet)
        {
          if (!usedTotalFactor || std::isinf(usedTotalFactor))
          {
            mainSizeList[nonFrozenIndex] = flexBaseAndHypotheticalMainSizeList[nonFrozenIndex].FlexBase;
            continue;
          }
          if (shouldUseFlexGrowFactor)
          {
            // If using the flex grow factor
            // Find the ratio of the item's flex grow factor to the sum of the flex grow factors of all
            // unfrozen items on the line. Set the item's target main size to its flex base size plus a
            // fraction of the remaining free space proportional to the ratio.
            auto growFactor = flexItems[nonFrozenIndex].GrowFactor() / usedTotalFactor;
            mainSizeList[nonFrozenIndex] = flexBaseAndHypotheticalMainSizeList[nonFrozenIndex].FlexBase
                                           + LayoutUnit {freeSpace.ToFloat() * growFactor};
            continue;
          }
          // If using the flex shrink factor
          // For every unfrozen item on the line, multiply its flex shrink factor by its inner flex base size,
          // and note this as its scaled flex shrink factor. Find the ratio of the item's scaled flex shrink
          // factor to the sum of the scaled flex shrink factors of all unfrozen items on the line. Set the
          // item's target main size to its flex base size minus a fraction of the absolute value of the
          // remaining free space proportional to the ratio. Note this may result in a negative inner main
          // size; it will be corrected in the next step.
          auto flexBaseSize = flexBaseAndHypotheticalMainSizeList[nonFrozenIndex].FlexBase;
          auto scaledShrinkFactor = flexItems[nonFrozenIndex].ShrinkFactor() * flexBaseSize.ToFloat();
          auto shrinkFactor = scaledShrinkFactor / usedTotalFactor;
          mainSizeList[nonFrozenIndex] =
            flexBaseSize - LayoutUnit {std::abs(freeSpace.ToFloat() * shrinkFactor)};
        }

        // d. Fix min/max violations. Clamp each non-frozen item's target main size by its used min and max
        // main sizes and floor
        //    its content-box size at zero. If the item's target main size was made smaller by this, it's a
        //    max violation. If the item's target main size was made larger by this, it's a min violation.
        auto totalViolation = LayoutUnit {};
        minimumViolationList.clear();
        maximumViolationList.clear();
        for (auto nonFrozenIndex : nonFrozenSet)
        {
          auto unclampedMainSize = mainSizeList[nonFrozenIndex];
          auto &flexItem = flexItems[nonFrozenIndex];
          auto clampedMainSize =
            std::max(FormattingUtils().UsedMinimumSizeInMainAxis(flexItem), unclampedMainSize);
          if (auto usedMaximumMainSize = FormattingUtils().UsedMaximumSizeInMainAxis(flexItem))
          {
            clampedMainSize = std::min(*usedMaximumMainSize, clampedMainSize);
          }
          // FIXME: ...and floor its content-box size at zero
          totalViolation += (clampedMainSize - unclampedMainSize);
          if (clampedMainSize < unclampedMainSize)
          {
            maximumViolationList.push_back(nonFrozenIndex);
          }
          else if (clampedMainSize > unclampedMainSize)
          {
            minimumViolationList.push_back(nonFrozenIndex);
          }
          mainSizeList[nonFrozenIndex] = clampedMainSize;
        }

        // e. Freeze over-flexed items. The total violation is the sum of the adjustments from the previous
        // step
        //    sum(clamped size - unclamped size). If the total violation is:
        //      Zero : Freeze all items.
        //      Positive: Freeze all the items with min violations.
        //      Negative: Freeze all the items with max violations.
        if (!totalViolation)
        {
          nonFrozenSet.clear();
        }
        else if (totalViolation > 0)
        {
          for (auto minimimViolationIndex : minimumViolationList)
          {
            nonFrozenSet.remove(minimimViolationIndex);
          }
        }
        else
        {
          for (auto maximumViolationIndex : maximumViolationList)
          {
            nonFrozenSet.remove(maximumViolationIndex);
          }
        }
      }
    }
    return mainSizeList;
  }

  FlexLayout::SizeList
    FlexLayout::HypotheticalCrossSizeForFlexItems(const LogicalFlexItems &flexItems,
                                                  const SizeList &flexItemsMainSizeList) noexcept
  {
    SizeList hypotheticalCrossSizeList(flexItems.size());
    for (size_t flexItemIndex = 0; flexItemIndex < flexItems.size(); ++flexItemIndex)
    {
      hypotheticalCrossSizeList[flexItemIndex] =
        FormattingUtils().UsedSizeInCrossAxis(flexItems[flexItemIndex], flexItemsMainSizeList[flexItemIndex]);
    }
    return hypotheticalCrossSizeList;
  }

  FlexLayout::LinesCrossSizeList FlexLayout::CrossSizeForFlexLines(
    const LineRanges &lineRanges, const ConstraintsForFlexContent::AxisGeometry &crossAxis,
    const LogicalFlexItems &flexItems, const SizeList &flexItemsHypotheticalCrossSizeList) const noexcept
  {
    LinesCrossSizeList flexLinesCrossSizeList(lineRanges.size());
    // If the flex container is single-line and has a definite cross size, the cross size of the flex line is
    // the flex container's inner cross size.
    if (IsSingleLineFlexContainer() && crossAxis.AvailableSize)
    {
      krys_debug_assert(flexLinesCrossSizeList.size() == 1);
      flexLinesCrossSizeList[0] = *crossAxis.AvailableSize;
      return flexLinesCrossSizeList;
    }

    for (size_t lineIndex = 0; lineIndex < lineRanges.size(); ++lineIndex)
    {
      auto maximumAscent = LayoutUnit {};
      auto maximumDescent = LayoutUnit {};
      auto maximumHypotheticalOuterCrossSize = LayoutUnit {};
      for (size_t flexItemIndex = lineRanges[lineIndex].Begin(); flexItemIndex < lineRanges[lineIndex].End();
           ++flexItemIndex)
      {
        // Collect all the flex items whose inline-axis is parallel to the main-axis, whose align-self is
        // baseline, and whose cross-axis margins are both non-auto.
        auto &flexItem = flexItems[flexItemIndex];
        if (!flexItem.IsOrthogonal() && flexItem.Style().GetAlignSelf().IsFirstBaseline()
            && flexItem.CrossAxis().HasNonAutoMargins())
        {
          // Find the largest of the distances between each item's baseline and its hypothetical outer
          // cross-start edge, and the largest of the distances between each item's baseline and its
          // hypothetical outer cross-end edge, and sum these two values.
          maximumAscent = std::max(maximumAscent, flexItem.CrossAxis().Ascent);
          maximumDescent = std::max(maximumDescent, flexItem.CrossAxis().Descent);
          continue;
        }
        // Among all the items not collected by the previous step, find the largest outer hypothetical cross
        // size.
        auto flexItemOuterCrossSize =
          OuterCrossSize(flexItem, flexItemsHypotheticalCrossSizeList[flexItemIndex]);
        maximumHypotheticalOuterCrossSize =
          std::max(maximumHypotheticalOuterCrossSize, flexItemOuterCrossSize);
      }
      // The used cross-size of the flex line is the largest of the numbers found in the previous two steps
      // and zero. If the flex container is single-line, then clamp the line's cross-size to be within the
      // container's computed min and max cross sizes.
      flexLinesCrossSizeList[lineIndex] =
        std::max(maximumHypotheticalOuterCrossSize, maximumAscent + maximumDescent);
      if (IsSingleLineFlexContainer())
      {
        auto minimumCrossSize = crossAxis.MinimumSize.value_or(flexLinesCrossSizeList[lineIndex]);
        auto maximumCrossSize = crossAxis.MaximumSize.value_or(LayoutUnit::Max());
        flexLinesCrossSizeList[lineIndex] =
          std::min(maximumCrossSize, std::max(minimumCrossSize, flexLinesCrossSizeList[lineIndex]));
      }
    }
    return flexLinesCrossSizeList;
  }

  void FlexLayout::StretchFlexLines(LinesCrossSizeList &flexLinesCrossSizeList, size_t numberOfLines,
                                    Maybe<LayoutUnit> crossAxisAvailableSpace) const noexcept
  {
    // Handle 'align-content: stretch'.
    // If the flex container has a definite cross size, align-content is stretch, and the sum of the flex
    // lines' cross sizes is less than the flex container's inner cross size, increase the cross size of each
    // flex line by equal amounts such that the sum of their cross sizes exactly equals the flex container's
    // inner cross size.
    auto linesMayStretch = [&]
    {
      auto alignContent = FlexContainerStyle().GetAlignContent();
      return alignContent.IsStretch() || alignContent.IsNormal();
    };
    if (!linesMayStretch() || !crossAxisAvailableSpace)
    {
      return;
    }

    auto linesCrossSize = [&]
    {
      auto size = LayoutUnit {};
      for (auto lineCrossSize : flexLinesCrossSizeList)
      {
        size += lineCrossSize;
      }
      return size;
    }();
    auto flexContainerUsedCrossSize =
      CrossAxisAvailableSpaceForLineSizingAndAlignment(*crossAxisAvailableSpace, numberOfLines);
    if (flexContainerUsedCrossSize <= linesCrossSize)
    {
      return;
    }

    auto extraSpace = (flexContainerUsedCrossSize - linesCrossSize) / static_cast<int32>(numberOfLines);
    for (auto &lineCrossSize : flexLinesCrossSizeList)
    {
      lineCrossSize += extraSpace;
    }
  }

  bool FlexLayout::CollapseNonVisibleFlexItems() noexcept
  {
    // Collapse visibility:collapse items. If any flex items have visibility: collapse,
    // note the cross size of the line they're in as the item's strut size, and restart layout from the
    // beginning.
    // FIXME: Not supported yet.
    return false;
  }

  FlexLayout::SizeList FlexLayout::ComputeCrossSizeForFlexItems(
    const LogicalFlexItems &flexItems, const LineRanges &lineRanges,
    const LinesCrossSizeList &flexLinesCrossSizeList,
    const SizeList &flexItemsHypotheticalCrossSizeList) const noexcept
  {
    SizeList crossSizeList(flexItems.size());
    for (size_t lineIndex = 0; lineIndex < lineRanges.size(); ++lineIndex)
    {
      for (auto flexItemIndex = lineRanges[lineIndex].Begin(); flexItemIndex < lineRanges[lineIndex].End();
           ++flexItemIndex)
      {
        auto &flexItem = flexItems[flexItemIndex];
        auto &crossAxis = flexItem.CrossAxis();
        auto flexItemAlignSelf = flexItem.Style().GetAlignSelf();
        auto alignValue = !flexItemAlignSelf.IsAuto()
                            ? flexItemAlignSelf.Resolve().Position()
                            : FlexContainerStyle().GetAlignItems().Resolve().Position();
        // If a flex item has align-self: stretch, its computed cross size property is auto, and neither of
        // its cross-axis margins are auto, the used outer cross size is the used cross size of its flex line,
        // clamped according to the item's used min and max cross sizes. Otherwise, the used cross size is the
        // item's hypothetical cross size.
        if ((alignValue == ItemPosition::Stretch || alignValue == ItemPosition::Normal)
            && crossAxis.HasSizeAuto && crossAxis.HasNonAutoMargins())
        {
          auto stretchedInnerCrossSize = [&]
          {
            auto stretchedInnerCrossSize = flexLinesCrossSizeList[lineIndex] - crossAxis.Margin();
            if (flexItem.IsContentBoxBased())
            {
              stretchedInnerCrossSize -= crossAxis.BorderAndPadding;
            }
            auto maximum = crossAxis.MaximumSize.value_or(stretchedInnerCrossSize);
            auto minimum = crossAxis.MinimumSize.value_or(stretchedInnerCrossSize);
            return std::min(maximum, std::max(minimum, stretchedInnerCrossSize));
          };
          crossSizeList[flexItemIndex] = stretchedInnerCrossSize();
          // FIXME: This requires re-layout to get percentage-sized descendants updated.
        }
        else
        {
          crossSizeList[flexItemIndex] = flexItemsHypotheticalCrossSizeList[flexItemIndex];
        }
      }
    }
    return crossSizeList;
  }

  FlexLayout::PositionAndMarginsList
    FlexLayout::HandleMainAxisAlignment(LayoutUnit availableMainSpace, const LineRanges &lineRanges,
                                        const LogicalFlexItems &flexItems,
                                        const SizeList &flexItemsMainSizeList) const noexcept
  {
    // Distribute any remaining free space. For each flex line:
    auto mainPositionAndMargins = PositionAndMarginsList(flexItems.size());

    for (auto lineRange : lineRanges)
    {
      auto lineContentOuterMainSize = LayoutUnit {};
      auto availableMainSpaceForLineContent =
        MainAxisAvailableSpaceForItemAlignment(availableMainSpace, lineRange.Distance());
      auto itemCount = static_cast<int32>(lineRange.Distance());

      auto resolveMarginAuto = [&]
      {
        // 1. If the remaining free space is positive and at least one main-axis margin on this line is auto,
        // distribute the free space equally among these margins.
        //    Otherwise, set all auto margins to zero.
        auto flexItemsWithMarginAuto = List<size_t> {};
        int32 autoMarginCount = 0;

        for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
        {
          auto &flexItem = flexItems[flexItemIndex];
          auto marginStart = flexItem.MainAxis().MarginStart;
          auto marginEnd = flexItem.MainAxis().MarginEnd;

          if (!marginStart || !marginEnd)
          {
            flexItemsWithMarginAuto.push_back(flexItemIndex);
            if (!marginStart)
            {
              ++autoMarginCount;
            }
            if (!marginEnd)
            {
              ++autoMarginCount;
            }
          }
          mainPositionAndMargins[flexItemIndex].MarginStart = marginStart.value_or(0_lu);
          mainPositionAndMargins[flexItemIndex].MarginEnd = marginEnd.value_or(0_lu);
          lineContentOuterMainSize += OuterMainSize(flexItem, flexItemsMainSizeList[flexItemIndex],
                                                    mainPositionAndMargins[flexItemIndex].Margin());
        }

        auto spaceToDistrubute = availableMainSpaceForLineContent - lineContentOuterMainSize;
        if (!autoMarginCount || spaceToDistrubute <= 0)
        {
          return;
        }

        lineContentOuterMainSize = availableMainSpaceForLineContent;
        auto extraMarginSpace = spaceToDistrubute / autoMarginCount;

        for (auto flexItemIndex : flexItemsWithMarginAuto)
        {
          auto &flexItem = flexItems[flexItemIndex];

          if (!flexItem.MainAxis().MarginStart)
          {
            mainPositionAndMargins[flexItemIndex].MarginStart = extraMarginSpace;
          }
          if (!flexItem.MainAxis().MarginEnd)
          {
            mainPositionAndMargins[flexItemIndex].MarginEnd = extraMarginSpace;
          }
        }
      };
      resolveMarginAuto();

      auto justifyContentValue = FlexContainerStyle().GetJustifyContent().Resolve();
      auto justifyContentDistribution = justifyContentValue.Distribution();
      auto justifyContentPosition = justifyContentValue.Position();

      auto setFallbackValuesIfApplicable = [&]
      {
        auto hasOverflow = lineContentOuterMainSize > availableMainSpaceForLineContent;
        if (!hasOverflow && itemCount > 1)
        {
          return;
        }

        switch (justifyContentDistribution)
        {
          case ContentDistribution::SpaceBetween:
            justifyContentPosition = hasOverflow ? ContentPosition::Start : ContentPosition::FlexStart;
            break;
          case ContentDistribution::SpaceEvenly:
          case ContentDistribution::SpaceAround:
            justifyContentPosition = hasOverflow ? ContentPosition::Start : ContentPosition::Center;
            break;
          default: break;
        }
        justifyContentPosition = justifyContentValue.Overflow() == OverflowAlignment::Safe && hasOverflow
                                   ? ContentPosition::Start
                                   : justifyContentPosition;
        justifyContentDistribution = ContentDistribution::Default;
      };
      setFallbackValuesIfApplicable();

      auto justifyContent = [&]
      {
        // 2. Align the items along the main-axis per justify-content.
        auto initialOffset = [&]
        {
          // ContentDistribution::Default handles fallback to justifyContentPosition
          if (justifyContentDistribution != ContentDistribution::Default)
          {
            switch (justifyContentDistribution)
            {
              case ContentDistribution::SpaceBetween: return LayoutUnit {};
              case ContentDistribution::SpaceAround:
                return (availableMainSpaceForLineContent - lineContentOuterMainSize) / itemCount / 2;
              case ContentDistribution::SpaceEvenly:
                return (availableMainSpaceForLineContent - lineContentOuterMainSize) / (itemCount + 1);
              default:
                // FIXME: Not implemented yet.
                krys_debug_assert(false);
                break;
            }
          }

          switch (FlexFormattingUtils::LogicalJustifyContentPosition(FlexContainer(), justifyContentPosition))
          {
            case ContentPosition::Normal:
            case ContentPosition::FlexStart: return LayoutUnit {};
            case ContentPosition::FlexEnd:   return availableMainSpaceForLineContent - lineContentOuterMainSize;
            case ContentPosition::Center:
              return availableMainSpaceForLineContent / 2 - lineContentOuterMainSize / 2;
            case ContentPosition::Start:
              if (FlexFormattingUtils::IsMainReversedToContentDirection(FlexContainer()))
              {
                return availableMainSpaceForLineContent - lineContentOuterMainSize;
              }
              return LayoutUnit {};
            case ContentPosition::End:
              if (FlexFormattingUtils::IsMainReversedToContentDirection(FlexContainer()))
              {
                return LayoutUnit {};
              }
              return availableMainSpaceForLineContent - lineContentOuterMainSize;
            default:
              // FIXME: Not implemented yet.
              krys_debug_assert(false);
              break;
          }
          return LayoutUnit {};
        };

        auto gapBetweenItems = [&]
        {
          switch (justifyContentDistribution)
          {
            case ContentDistribution::Default: return LayoutUnit {};
            case ContentDistribution::SpaceBetween:
              return std::max(0_lu, availableMainSpaceForLineContent - lineContentOuterMainSize)
                     / (itemCount - 1);
            case ContentDistribution::SpaceAround:
              return std::max(0_lu, availableMainSpaceForLineContent - lineContentOuterMainSize) / itemCount;
            case ContentDistribution::SpaceEvenly:
              return std::max(0_lu, availableMainSpaceForLineContent - lineContentOuterMainSize)
                     / (itemCount + 1);
            default:
              // FIXME: Not implemented yet.
              krys_debug_assert(false);
              break;
          }
          return LayoutUnit {};
        };

        auto flexItemOuterEnd = [&](size_t flexItemIndex)
        {
          auto flexItemOuterMainSize =
            OuterMainSize(flexItems[flexItemIndex], flexItemsMainSizeList[flexItemIndex],
                          mainPositionAndMargins[flexItemIndex].Margin());
          // Note that position here means border box position.
          auto flexItemEnd = flexItemOuterMainSize - mainPositionAndMargins[flexItemIndex].MarginStart;
          return mainPositionAndMargins[flexItemIndex].Position + flexItemEnd;
        };

        auto startIndex = lineRange.Begin();
        mainPositionAndMargins[startIndex].Position =
          initialOffset() + mainPositionAndMargins[startIndex].MarginStart;
        auto previousFlexItemOuterEnd = flexItemOuterEnd(startIndex);
        auto gap = gapBetweenItems();
        for (auto index = startIndex + 1; index < lineRange.End(); ++index)
        {
          mainPositionAndMargins[index].Position =
            previousFlexItemOuterEnd + gap + mainPositionAndMargins[index].MarginStart;
          previousFlexItemOuterEnd = flexItemOuterEnd(index);
        }
      };
      justifyContent();
    }
    return mainPositionAndMargins;
  }

  FlexLayout::PositionAndMarginsList FlexLayout::HandleCrossAxisAlignmentForFlexItems(
    const LogicalFlexItems &flexItems, const LineRanges &lineRanges, const SizeList &flexItemsCrossSizeList,
    const LinesCrossSizeList &flexLinesCrossSizeList) const noexcept
  {
    auto crossPositionAndMargins = PositionAndMarginsList(flexItems.size());

    for (size_t lineIndex = 0; lineIndex < lineRanges.size(); ++lineIndex)
    {
      auto lineRange = lineRanges[lineIndex];

      auto resolveMarginAuto = [&]
      {
        for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
        {
          auto &flexItem = flexItems[flexItemIndex];
          auto marginStart = flexItem.CrossAxis().MarginStart;
          auto marginEnd = flexItem.CrossAxis().MarginEnd;

          // Resolve cross-axis auto margins. If a flex item has auto cross-axis margins:
          if (!marginStart || !marginEnd)
          {
            auto flexItemOuterCrossSize = OuterCrossSize(flexItem, flexItemsCrossSizeList[flexItemIndex]);
            auto extraCrossSpace = flexLinesCrossSizeList[lineIndex] - flexItemOuterCrossSize;
            // If its outer cross size (treating those auto margins as zero) is less than the cross size of
            // its flex line, distribute the difference in those sizes equally to the auto margins. Otherwise,
            // if the block-start or inline-start margin (whichever is in the cross axis) is auto, set it to
            // zero. Set the opposite margin so that the outer cross size of the item equals the cross size of
            // its flex line.
            if (extraCrossSpace > 0)
            {
              if (!marginStart && !marginEnd)
              {
                marginStart = extraCrossSpace / 2;
                marginEnd = extraCrossSpace / 2;
              }
              else if (!marginStart)
              {
                marginStart = extraCrossSpace;
              }
              else
              {
                marginEnd = extraCrossSpace;
              }
            }
            else
            {
              auto marginCrossSpace = flexLinesCrossSizeList[lineIndex] - flexItemOuterCrossSize;
              if (!marginStart)
              {
                marginStart = 0_lu;
                marginEnd = marginCrossSpace;
              }
              else
              {
                marginStart = marginCrossSpace;
                marginEnd = 0_lu;
              }
            }
          }
          crossPositionAndMargins[flexItemIndex].MarginStart = *marginStart;
          crossPositionAndMargins[flexItemIndex].MarginEnd = *marginEnd;
        }
      };
      resolveMarginAuto();

      auto alignSelf = [&]
      {
        // Align all flex items along the cross-axis per align-self, if neither of the item's cross-axis
        // margins are auto.
        for (auto flexItemIndex = lineRange.Begin(); flexItemIndex < lineRange.End(); ++flexItemIndex)
        {
          auto &flexItem = flexItems[flexItemIndex];
          auto flexItemOuterCrossSize = OuterCrossSize(flexItem, flexItemsCrossSizeList[flexItemIndex],
                                                       crossPositionAndMargins[flexItemIndex].Margin());
          auto flexItemOuterCrossPosition = LayoutUnit {};

          auto flexItemAlign = !flexItem.Style().GetAlignSelf().IsAuto()
                                 ? flexItem.Style().GetAlignSelf().Resolve()
                                 : FlexContainerStyle().GetAlignItems().Resolve();
          auto alignSelfPosition = flexItemAlign.Position();
          auto setFallbackValuesIfApplicable = [&]
          {
            if (flexItemOuterCrossSize > flexLinesCrossSizeList[lineIndex]
                && flexItemAlign.Overflow() == OverflowAlignment::Safe)
            {
              alignSelfPosition = ItemPosition::FlexStart;
            }
          };
          setFallbackValuesIfApplicable();

          switch (alignSelfPosition)
          {
            case ItemPosition::Stretch:
            case ItemPosition::Normal:
              // This is taken care of at 9.4.11 see computeCrossSizeForFlexItems.
              flexItemOuterCrossPosition = LayoutUnit {};
              break;
            case ItemPosition::Center:
              flexItemOuterCrossPosition = flexLinesCrossSizeList[lineIndex] / 2 - flexItemOuterCrossSize / 2;
              break;
            case ItemPosition::Start:
            case ItemPosition::SelfStart:
            case ItemPosition::FlexStart: flexItemOuterCrossPosition = LayoutUnit {}; break;
            case ItemPosition::End:
            case ItemPosition::SelfEnd:
            case ItemPosition::FlexEnd:
              flexItemOuterCrossPosition = flexLinesCrossSizeList[lineIndex] - flexItemOuterCrossSize;
              break;
            default:
              // FIXME: Not implemented yet.
              krys_debug_assert(false);
              break;
          }
          crossPositionAndMargins[flexItemIndex].Position =
            flexItemOuterCrossPosition + crossPositionAndMargins[flexItemIndex].MarginStart;
        }
      };
      alignSelf();
    }
    return crossPositionAndMargins;
  }

  FlexLayout::LinesCrossPositionList FlexLayout::HandleCrossAxisAlignmentForFlexLines(
    Maybe<LayoutUnit> crossAxisAvailableSpace, const LineRanges &lineRanges,
    LinesCrossSizeList &flexLinesCrossSizeList) const noexcept
  {
    // If the cross size property is a definite size, use that, clamped by the used min and max cross sizes of
    // the flex container. Otherwise, use the sum of the flex lines' cross sizes, clamped by the used min and
    // max cross sizes of the flex container.
    auto flexLinesCrossSize = [&]
    {
      auto linesCrossSize = LayoutUnit {};
      for (auto crossSize : flexLinesCrossSizeList)
      {
        linesCrossSize += crossSize;
      }
      return linesCrossSize;
    }();
    auto isSingleLineFlexContainer = IsSingleLineFlexContainer() || lineRanges.size() == 1;
    auto numberOfLines = static_cast<int32>(lineRanges.size());
    auto flexContainerUsedCrossSize = CrossAxisAvailableSpaceForLineSizingAndAlignment(
      crossAxisAvailableSpace.value_or(0_lu), lineRanges.size());

    // Align all flex lines per align-content.
    auto distributableCrossSpace = flexContainerUsedCrossSize - flexLinesCrossSize;
    auto initialOffset = [&]() -> LayoutUnit
    {
      auto alignContentValue = FlexContainerStyle().GetAlignContent().Resolve();
      auto alignContentPosition = alignContentValue.Position();
      auto alignContentDistribution = alignContentValue.Distribution();

      auto setFallbackValuesIfApplicable = [&]
      {
        auto hasOverflow = distributableCrossSpace < 0;
        if (!hasOverflow && !isSingleLineFlexContainer)
        {
          return;
        }
        switch (alignContentDistribution)
        {
          case ContentDistribution::SpaceBetween:
            alignContentPosition = hasOverflow ? ContentPosition::Start : ContentPosition::FlexStart;
            break;
          case ContentDistribution::SpaceEvenly:
          case ContentDistribution::SpaceAround:
            alignContentPosition = hasOverflow ? ContentPosition::Start : ContentPosition::Center;
            break;
          case ContentDistribution::Stretch: alignContentPosition = ContentPosition::FlexStart; break;
          default:                           break;
        }
        alignContentPosition = alignContentValue.Overflow() == OverflowAlignment::Safe && hasOverflow
                                 ? ContentPosition::Start
                                 : alignContentPosition;
        alignContentDistribution = ContentDistribution::Default;
      };
      setFallbackValuesIfApplicable();

      auto flipStartEndContentPositionIfApplicable = [&]
      {
        auto isWrapReversed = FlexFormattingUtils::AreFlexLinesReversedInCrossAxis(FlexContainer());
        if (alignContentPosition == ContentPosition::Start)
        {
          alignContentPosition = isWrapReversed ? ContentPosition::FlexEnd : ContentPosition::FlexStart;
        }
        else if (alignContentPosition == ContentPosition::End)
        {
          alignContentPosition = isWrapReversed ? ContentPosition::FlexStart : ContentPosition::FlexEnd;
        }
      };
      flipStartEndContentPositionIfApplicable();

      switch (alignContentPosition)
      {
        case ContentPosition::FlexStart: return {};
        case ContentPosition::Center:    return flexContainerUsedCrossSize / 2 - flexLinesCrossSize / 2;
        case ContentPosition::FlexEnd:   return flexContainerUsedCrossSize - flexLinesCrossSize;
        default:
          switch (alignContentDistribution)
          {
            case ContentDistribution::Default:
            case ContentDistribution::SpaceBetween:
            case ContentDistribution::Stretch:      return {};
            case ContentDistribution::SpaceAround:  return distributableCrossSpace / numberOfLines / 2;
            case ContentDistribution::SpaceEvenly:  return distributableCrossSpace / (numberOfLines + 1);
          }
          krys_debug_assert(false);
          return {};
      }
    };

    LinesCrossPositionList linesCrossPositionList(lineRanges.size());
    linesCrossPositionList[0] = initialOffset();
    if (isSingleLineFlexContainer)
    {
      return linesCrossPositionList;
    }

    auto gap = [&]() -> LayoutUnit
    {
      if (distributableCrossSpace <= 0)
      {
        return {};
      }
      return FlexContainerStyle().GetAlignContent().SwitchOn(
        [&](const css::keywords::SpaceBetween &) -> LayoutUnit
        { return distributableCrossSpace / (numberOfLines - 1); },
        [&](const css::keywords::SpaceAround &) -> LayoutUnit
        { return distributableCrossSpace / numberOfLines; },
        [&](const css::keywords::Stretch &) -> LayoutUnit
        {
          // Lines stretch to take up the remaining space. If the leftover free-space is negative,
          // this value is identical to flex-start. Otherwise, the free-space is split equally between all of
          // the lines, increasing their cross size.
          auto extraCrossSpaceForEachLine =
            distributableCrossSpace / static_cast<int32>(flexLinesCrossSizeList.size());
          for (auto &lineCrossSize : flexLinesCrossSizeList)
          {
            lineCrossSize += extraCrossSpaceForEachLine;
          }
          return {};
        },
        [&](const css::keywords::SpaceEvenly &) -> LayoutUnit
        { return distributableCrossSpace / (numberOfLines + 1); },
        [](const auto &) -> LayoutUnit { return {}; });
    }();
    for (size_t lineIndex = 1; lineIndex < lineRanges.size(); ++lineIndex)
    {
      linesCrossPositionList[lineIndex] =
        (linesCrossPositionList[lineIndex - 1] + flexLinesCrossSizeList[lineIndex - 1]) + gap;
    }
    return linesCrossPositionList;
  }

  LayoutUnit FlexLayout::MainAxisAvailableSpaceForItemAlignment(LayoutUnit mainAxisAvailableSpace,
                                                                size_t numberOfFlexItems) const noexcept
  {
    if (numberOfFlexItems == 1)
    {
      return mainAxisAvailableSpace;
    }
    return mainAxisAvailableSpace
           - (FlexFormattingUtils::MainAxisGapValue(FlexContainer(), mainAxisAvailableSpace)
              * static_cast<int32>(numberOfFlexItems - 1));
  }

  LayoutUnit
    FlexLayout::CrossAxisAvailableSpaceForLineSizingAndAlignment(LayoutUnit crossAxisAvailableSpace,
                                                                 size_t numberOfFlexLines) const noexcept
  {
    if (numberOfFlexLines == 1)
    {
      return crossAxisAvailableSpace;
    }
    return crossAxisAvailableSpace
           - (FlexFormattingUtils::CrossAxisGapValue(FlexContainer(), crossAxisAvailableSpace)
              * static_cast<int32>(numberOfFlexLines - 1));
  }

  bool FlexLayout::IsSingleLineFlexContainer() const noexcept
  {
    return FlexContainerStyle().GetFlexWrap() == FlexWrap::NoWrap;
  }

  const ElementBox &FlexLayout::FlexContainer() const noexcept
  {
    return _flexFormattingContext.Root();
  }

  const RenderStyle &FlexLayout::FlexContainerStyle() const noexcept
  {
    return FlexContainer().Style();
  }

  const FlexFormattingContext &FlexLayout::FormattingContext() const noexcept
  {
    return _flexFormattingContext;
  }

  const FlexFormattingUtils &FlexLayout::FormattingUtils() const noexcept
  {
    return FormattingContext().FormattingUtils();
  }
}
