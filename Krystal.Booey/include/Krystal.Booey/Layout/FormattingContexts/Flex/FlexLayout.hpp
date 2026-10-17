#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/LogicalFlexItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::layout
{
  class FlexFormattingContext;
  class FlexFormattingUtils;
  struct FlexBaseAndHypotheticalMainSize;
  struct PositionAndMargins;

  // This class implements the layout logic for flex formatting contexts.
  // https://www.w3.org/TR/css-flexbox-1/
  /// @brief Runs the flex layout algorithm over logical flex items and returns their logical rects.
  class FlexLayout
  {
  public:
    using LogicalFlexItems = List<LogicalFlexItem>;
    using LogicalFlexItemRects = List<FlexRect>;

  private:
    using FlexBaseAndHypotheticalMainSizeList = List<FlexBaseAndHypotheticalMainSize>;
    using LineRanges = List<IndexRange>;
    using SizeList = List<LayoutUnit>;
    using PositionAndMarginsList = List<PositionAndMargins>;
    using LinesCrossSizeList = List<LayoutUnit>;
    using LinesCrossPositionList = List<LayoutUnit>;

    FlexFormattingContext &_flexFormattingContext;

  public:
    explicit FlexLayout(FlexFormattingContext &flexFormattingContext) noexcept;

    KRYS_NODISCARD LogicalFlexItemRects Layout(const ConstraintsForFlexContent &flexContainerConstraints,
                                               const LogicalFlexItems &flexItems) noexcept;

  private:
    KRYS_NODISCARD FlexBaseAndHypotheticalMainSizeList FlexBaseAndHypotheticalMainSizeForFlexItems(
      const LogicalFlexItems &flexItems, bool isSizedUnderMinMaxConstraints) const noexcept;
    KRYS_NODISCARD LayoutUnit FlexContainerInnerMainSize(
      const ConstraintsForFlexContent::AxisGeometry &mainAxisGeometry) const noexcept;
    KRYS_NODISCARD LineRanges ComputeFlexLines(
      const LogicalFlexItems &flexItems, LayoutUnit flexContainerInnerMainSize,
      const FlexBaseAndHypotheticalMainSizeList &flexBaseAndHypotheticalMainSizeList) const noexcept;
    KRYS_NODISCARD SizeList ComputeMainSizeForFlexItems(
      const LogicalFlexItems &flexItems, const LineRanges &lineRanges, LayoutUnit flexContainerInnerMainSize,
      const FlexBaseAndHypotheticalMainSizeList &flexBaseAndHypotheticalMainSizeList) const noexcept;
    KRYS_NODISCARD SizeList HypotheticalCrossSizeForFlexItems(const LogicalFlexItems &flexItems,
                                                              const SizeList &flexItemsMainSizeList) noexcept;
    KRYS_NODISCARD LinesCrossSizeList CrossSizeForFlexLines(
      const LineRanges &lineRanges, const ConstraintsForFlexContent::AxisGeometry &crossAxis,
      const LogicalFlexItems &flexItems, const SizeList &flexItemsHypotheticalCrossSizeList) const noexcept;
    void StretchFlexLines(LinesCrossSizeList &flexLinesCrossSizeList, size_t numberOfLines,
                          Maybe<LayoutUnit> crossAxisAvailableSpace) const noexcept;
    KRYS_NODISCARD bool CollapseNonVisibleFlexItems() noexcept;
    KRYS_NODISCARD SizeList
      ComputeCrossSizeForFlexItems(const LogicalFlexItems &flexItems, const LineRanges &lineRanges,
                                   const LinesCrossSizeList &flexLinesCrossSizeList,
                                   const SizeList &flexItemsHypotheticalCrossSizeList) const noexcept;
    KRYS_NODISCARD PositionAndMarginsList HandleMainAxisAlignment(
      LayoutUnit availableMainSpace, const LineRanges &lineRanges, const LogicalFlexItems &flexItems,
      const SizeList &flexItemsMainSizeList) const noexcept;
    KRYS_NODISCARD PositionAndMarginsList HandleCrossAxisAlignmentForFlexItems(
      const LogicalFlexItems &flexItems, const LineRanges &lineRanges, const SizeList &flexItemsCrossSizeList,
      const LinesCrossSizeList &flexLinesCrossSizeList) const noexcept;
    KRYS_NODISCARD LinesCrossPositionList HandleCrossAxisAlignmentForFlexLines(
      Maybe<LayoutUnit> crossAxisAvailableSpace, const LineRanges &lineRanges,
      LinesCrossSizeList &flexLinesCrossSizeList) const noexcept;

    KRYS_NODISCARD LayoutUnit MainAxisAvailableSpaceForItemAlignment(LayoutUnit mainAxisAvailableSpace,
                                                                     size_t numberOfFlexItems) const noexcept;
    KRYS_NODISCARD LayoutUnit CrossAxisAvailableSpaceForLineSizingAndAlignment(
      LayoutUnit crossAxisAvailableSpace, size_t numberOfFlexLines) const noexcept;

    KRYS_NODISCARD bool IsSingleLineFlexContainer() const noexcept;
    KRYS_NODISCARD const ElementBox &FlexContainer() const noexcept;
    KRYS_NODISCARD const RenderStyle &FlexContainerStyle() const noexcept;
    KRYS_NODISCARD const FlexFormattingContext &FormattingContext() const noexcept;
    KRYS_NODISCARD const FlexFormattingUtils &FormattingUtils() const noexcept;
  };
}
