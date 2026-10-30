#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/LogicalFlexItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::layout
{
  FlexFormattingUtils::FlexFormattingUtils(const FlexFormattingContext &flexFormattingContext) noexcept
      : _flexFormattingContext(flexFormattingContext)
  {
  }

  bool FlexFormattingUtils::IsMainAxisParallelWithInlineAxis(const ElementBox &flexContainer) noexcept
  {
    krys_debug_assert(flexContainer.IsFlexBox());
    auto &flexContainerStyle = flexContainer.Style();
    auto isHorizontalWritingMode = flexContainerStyle.GetWritingMode().IsHorizontal();
    auto flexDirection = flexContainerStyle.GetFlexDirection();
    return (isHorizontalWritingMode
            && (flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse))
           || (!isHorizontalWritingMode
               && (flexDirection == FlexDirection::Column || flexDirection == FlexDirection::ColumnReverse));
  }

  ContentPosition
    FlexFormattingUtils::LogicalJustifyContentPosition(const ElementBox &flexContainer,
                                                       ContentPosition justifyContentPosition) noexcept
  {
    krys_debug_assert(flexContainer.IsFlexBox());

    if (justifyContentPosition != ContentPosition::Right && justifyContentPosition != ContentPosition::Left)
    {
      return justifyContentPosition;
    }

    auto &flexContainerStyle = flexContainer.Style();
    // If the property's axis is not parallel with either left<->right axis, this value behaves as start
    // (https://drafts.csswg.org/css-align/#positional-values) Currently, the only case where the property's
    // axis is not parallel with either left<->right axis is in a column flexbox.
    // https://drafts.csswg.org/css-align/#positional-values
    if (flexContainerStyle.IsColumnFlexDirection() && flexContainerStyle.GetWritingMode().IsHorizontal())
    {
      return ContentPosition::Start;
    }

    auto isLeftToRightInAxisDirection = flexContainerStyle.IsRowFlexDirection()
                                          ? flexContainerStyle.GetWritingMode().IsLogicalLeftInlineStart()
                                          : flexContainerStyle.GetWritingMode().IsBlockLeftToRight();
    if (justifyContentPosition == ContentPosition::Left)
    {
      return isLeftToRightInAxisDirection ? ContentPosition::Start : ContentPosition::End;
    }
    return isLeftToRightInAxisDirection ? ContentPosition::End : ContentPosition::Start;
  }

  bool FlexFormattingUtils::IsMainReversedToContentDirection(const ElementBox &flexContainer) noexcept
  {
    krys_debug_assert(flexContainer.IsFlexBox());
    auto flexDirection = flexContainer.Style().GetFlexDirection();
    return flexDirection == FlexDirection::RowReverse || flexDirection == FlexDirection::ColumnReverse;
  }

  bool FlexFormattingUtils::AreFlexLinesReversedInCrossAxis(const ElementBox &flexContainer) noexcept
  {
    krys_debug_assert(flexContainer.IsFlexBox());
    return flexContainer.Style().GetFlexWrap() == FlexWrap::Reverse;
  }

  // The column-gap property specifies spacing between "columns", separating boxes in the container's inline
  // axis similar to inline-axis margin; while row-gap indicates spacing between "rows". separating boxes in
  // the container's block axis. horizontal row    : column gap vertical   row    : column gap horizontal
  // column : row gap vertical   column : row gap
  LayoutUnit FlexFormattingUtils::MainAxisGapValue(const ElementBox &flexContainer,
                                                   LayoutUnit flexContainerContentBoxWidth) noexcept
  {
    krys_debug_assert(flexContainer.IsFlexBox());
    auto flexDirection = flexContainer.Style().GetFlexDirection();
    auto isMainAxisInlineAxis =
      flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse;
    auto &gap = isMainAxisInlineAxis ? flexContainer.Style().ColumnGap() : flexContainer.Style().RowGap();
    return style::EvaluateMinimum<LayoutUnit>(gap, flexContainerContentBoxWidth, style::ZoomNeeded {});
  }

  LayoutUnit FlexFormattingUtils::CrossAxisGapValue(const ElementBox &flexContainer,
                                                    LayoutUnit flexContainerContentBoxHeight) noexcept
  {
    krys_debug_assert(flexContainer.IsFlexBox());
    auto flexDirection = flexContainer.Style().GetFlexDirection();
    auto isMainAxisInlineAxis =
      flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse;
    auto &gap = isMainAxisInlineAxis ? flexContainer.Style().RowGap() : flexContainer.Style().ColumnGap();
    return style::EvaluateMinimum<LayoutUnit>(gap, flexContainerContentBoxHeight, style::ZoomNeeded {});
  }

  // flex container  direction  flex item    main axis size
  // horizontal      row        horizontal : width
  // vertical        column     horizontal : width
  // vertical        row        vertical   : width
  // horizontal      column     vertical   : width
  //
  // horizontal      row        vertical   : height
  // horizontal      column     horizontal : height
  // vertical        row        horizontal : height
  // vertical        column     vertical   : height
  LayoutUnit FlexFormattingUtils::UsedMinimumSizeInMainAxis(const LogicalFlexItem &flexItem) const noexcept
  {
    if (auto mainAxisMinimumWidth = flexItem.MainAxis().MinimumSize)
    {
      return *mainAxisMinimumWidth;
    }

    auto &flexContainer = FormattingContext().Root();
    auto &flexItemBox = flexItem.LayoutBox();
    auto isMainAxisParallelWithInlineAxis = IsMainAxisParallelWithInlineAxis(flexContainer);

    auto minimumContentSize = LayoutUnit {};
    auto shouldUseFlexItemContentWidth =
      (isMainAxisParallelWithInlineAxis && flexItem.GetWritingMode().IsHorizontal())
      || (!isMainAxisParallelWithInlineAxis && flexItem.GetWritingMode().IsVertical());
    minimumContentSize = shouldUseFlexItemContentWidth
                           ? FormattingContext().GetIntegrationUtils().MinContentWidth(flexItemBox)
                           : FormattingContext().GetIntegrationUtils().MinContentHeight(flexItemBox);
    if (auto mainAxisWidth = flexItem.MainAxis().Size)
    {
      minimumContentSize = std::min(*mainAxisWidth, minimumContentSize);
    }
    return minimumContentSize;
  }

  Maybe<LayoutUnit>
    FlexFormattingUtils::UsedMaximumSizeInMainAxis(const LogicalFlexItem &flexItem) const noexcept
  {
    // Initial value of 'max-width: none' computes to used 'infinite'
    return flexItem.MainAxis().MaximumSize;
  }

  LayoutUnit FlexFormattingUtils::UsedMaxContentSizeInMainAxis(const LogicalFlexItem &flexItem) const noexcept
  {
    auto &flexContainer = FormattingContext().Root();
    auto &flexItemBox = flexItem.LayoutBox();
    auto isMainAxisParallelWithInlineAxis = IsMainAxisParallelWithInlineAxis(flexContainer);

    auto contentSize = LayoutUnit {};
    auto shouldUseFlexItemContentWidth =
      (isMainAxisParallelWithInlineAxis && flexItem.GetWritingMode().IsHorizontal())
      || (!isMainAxisParallelWithInlineAxis && flexItem.GetWritingMode().IsVertical());
    if (shouldUseFlexItemContentWidth)
    {
      contentSize = FormattingContext().GetIntegrationUtils().MaxContentWidth(flexItemBox);
    }
    else
    {
      FormattingContext().GetIntegrationUtils().LayoutWithFormattingContextForBox(flexItemBox);
      auto isOrthogonal = flexContainer.GetWritingMode().IsOrthogonal(flexItem.GetWritingMode());
      contentSize = !isOrthogonal ? FormattingContext().GeometryForFlexItem(flexItemBox).ContentBoxHeight()
                                  : FormattingContext().GeometryForFlexItem(flexItemBox).ContentBoxWidth();
    }

    if (!flexItem.IsContentBoxBased())
    {
      contentSize += flexItem.MainAxis().BorderAndPadding;
    }
    return contentSize;
  }

  LayoutUnit FlexFormattingUtils::UsedSizeInCrossAxis(const LogicalFlexItem &flexItem,
                                                      LayoutUnit maxAxisConstraint) const noexcept
  {
    if (auto definiteSize = flexItem.CrossAxis().DefiniteSize)
    {
      return *definiteSize;
    }

    auto &flexContainer = FormattingContext().Root();
    auto &flexItemBox = flexItem.LayoutBox();
    auto isMainAxisParallelWithInlineAxis = IsMainAxisParallelWithInlineAxis(flexContainer);

    auto widthConstraintForLayout =
      isMainAxisParallelWithInlineAxis ? Maybe<LayoutUnit>(maxAxisConstraint) : null;
    auto shouldUseFlexItemContentHeight =
      (isMainAxisParallelWithInlineAxis && flexItem.GetWritingMode().IsHorizontal())
      || (!isMainAxisParallelWithInlineAxis && flexItem.GetWritingMode().IsVertical());
    FormattingContext().GetIntegrationUtils().LayoutWithFormattingContextForBox(flexItemBox,
                                                                                widthConstraintForLayout);
    auto crossSize = shouldUseFlexItemContentHeight
                       ? FormattingContext().GeometryForFlexItem(flexItemBox).ContentBoxHeight()
                       : FormattingContext().GeometryForFlexItem(flexItemBox).ContentBoxWidth();
    if (!flexItem.IsContentBoxBased())
    {
      crossSize += flexItem.CrossAxis().BorderAndPadding;
    }
    return crossSize;
  }
}
