#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include <algorithm>

namespace krys::boo::layout
{
  FlexFormattingContext::FlexFormattingContext(const ElementBox &flexBox,
                                               LayoutState &globalLayoutState) noexcept
      : _flexBox(flexBox), _globalLayoutState(globalLayoutState), _flexFormattingUtils(*this),
        _integrationUtils(globalLayoutState)
  {
  }

  void FlexFormattingContext::Layout(const ConstraintsForFlexContent &constraints) noexcept
  {
    auto logicalFlexItems = ConvertFlexItemsToLogicalSpace(constraints);
    auto flexItemRects = FlexLayout {*this}.Layout(constraints, logicalFlexItems);
    SetFlexItemsGeometry(logicalFlexItems, flexItemRects, constraints);
    PositionOutOfFlowChildren();
  }

  IntrinsicWidthConstraints FlexFormattingContext::ComputedIntrinsicWidthConstraints() noexcept
  {
    return {};
  }

  FlexLayout::LogicalFlexItems FlexFormattingContext::ConvertFlexItemsToLogicalSpace(
    const ConstraintsForFlexContent &constraints) noexcept
  {
    struct FlexItem
    {
      LogicalFlexItem::MainAxisGeometry MainAxis;
      LogicalFlexItem::CrossAxisGeometry CrossAxis;
      int32 LogicalOrder {0};
      const ElementBox *LayoutBox {nullptr};
    };

    List<FlexItem> flexItemList;
    auto flexItemsNeedReordering = false;

    auto convertVisualToLogical = [&]
    {
      auto direction = Root().Style().GetFlexDirection();
      auto previousLogicalOrder = Maybe<int32> {};
      auto isMainAxisParallelWithInlineAxis = FlexFormattingUtils::IsMainAxisParallelWithInlineAxis(Root());
      auto isReversedInCrossAxis = FlexFormattingUtils::AreFlexLinesReversedInCrossAxis(Root());

      for (auto *flexItem = Root().FirstInFlowChild(); flexItem; flexItem = flexItem->NextInFlowSibling())
      {
        auto &flexItemGeometry = _globalLayoutState.GeometryForBox(*flexItem);
        auto &style = flexItem->Style();
        auto mainAxis = LogicalFlexItem::MainAxisGeometry {};
        auto crossAxis = LogicalFlexItem::CrossAxisGeometry {};

        auto propertyValueForLength = [&](const auto &propertyValue, auto availableSize) -> Maybe<LayoutUnit>
        {
          if (auto fixedPropertyValue = propertyValue.TryFixed())
          {
            return style::Evaluate<LayoutUnit>(*fixedPropertyValue, style.UsedZoomForLength());
          }

          if (propertyValue.IsSpecified() && availableSize)
          {
            return style::Evaluate<LayoutUnit>(propertyValue, *availableSize, style.UsedZoomForLength());
          }
          return {};
        };

        auto setMainAxisValues = [&]
        {
          auto flexContainerMainInnerSize = constraints.MainAxis().AvailableSize;

          mainAxis.Size = propertyValueForLength(
            isMainAxisParallelWithInlineAxis ? style.Width() : style.Height(), flexContainerMainInnerSize);
          mainAxis.MinimumSize =
            propertyValueForLength(isMainAxisParallelWithInlineAxis ? style.MinWidth() : style.MinHeight(),
                                   flexContainerMainInnerSize);
          mainAxis.MaximumSize =
            propertyValueForLength(isMainAxisParallelWithInlineAxis ? style.MaxWidth() : style.MaxHeight(),
                                   flexContainerMainInnerSize);
          // Auto keyword retrieves the value of the main size property as the used flex-basis.
          // If that value is itself auto, then the used value is content.
          mainAxis.DefiniteFlexBasis =
            style.GetFlexBasis().IsAuto()
              ? mainAxis.Size
              : propertyValueForLength(style.GetFlexBasis(), flexContainerMainInnerSize);

          auto marginStart = [&]() -> Maybe<LayoutUnit>
          {
            if (direction == FlexDirection::Row && !style.MarginStart().IsAuto())
            {
              return flexItemGeometry.MarginStart();
            }
            if (direction == FlexDirection::RowReverse && !style.MarginEnd().IsAuto())
            {
              return flexItemGeometry.MarginEnd();
            }
            if (direction == FlexDirection::Column && !style.MarginBefore().IsAuto())
            {
              return flexItemGeometry.MarginBefore();
            }
            if (direction == FlexDirection::ColumnReverse && !style.MarginAfter().IsAuto())
            {
              return flexItemGeometry.MarginAfter();
            }
            return {};
          };
          auto marginEnd = [&]() -> Maybe<LayoutUnit>
          {
            if (direction == FlexDirection::Row && !style.MarginEnd().IsAuto())
            {
              return flexItemGeometry.MarginEnd();
            }
            if (direction == FlexDirection::RowReverse && !style.MarginStart().IsAuto())
            {
              return flexItemGeometry.MarginStart();
            }
            if (direction == FlexDirection::Column && !style.MarginAfter().IsAuto())
            {
              return flexItemGeometry.MarginAfter();
            }
            if (direction == FlexDirection::ColumnReverse && !style.MarginBefore().IsAuto())
            {
              return flexItemGeometry.MarginBefore();
            }
            return {};
          };
          auto shouldFlipMargins =
            isMainAxisParallelWithInlineAxis && Root().GetWritingMode().IsLineInverted();
          mainAxis.MarginStart = !shouldFlipMargins ? marginStart() : marginEnd();
          mainAxis.MarginEnd = !shouldFlipMargins ? marginEnd() : marginStart();
          mainAxis.BorderAndPadding = isMainAxisParallelWithInlineAxis
                                        ? flexItemGeometry.HorizontalBorderAndPadding()
                                        : flexItemGeometry.VerticalBorderAndPadding();
        };
        setMainAxisValues();

        auto setCrossAxisValues = [&]
        {
          auto flexContainerCrossInnerSize = constraints.CrossAxis().AvailableSize;

          crossAxis.DefiniteSize = propertyValueForLength(
            isMainAxisParallelWithInlineAxis ? style.Height() : style.Width(), flexContainerCrossInnerSize);
          crossAxis.MinimumSize =
            propertyValueForLength(isMainAxisParallelWithInlineAxis ? style.MinHeight() : style.MinWidth(),
                                   flexContainerCrossInnerSize);
          crossAxis.MaximumSize =
            propertyValueForLength(isMainAxisParallelWithInlineAxis ? style.MaxHeight() : style.MaxWidth(),
                                   flexContainerCrossInnerSize);

          auto isRowDirection = direction == FlexDirection::Row || direction == FlexDirection::RowReverse;
          auto marginStart = [&]() -> Maybe<LayoutUnit>
          {
            if (!isReversedInCrossAxis)
            {
              if (isRowDirection)
              {
                return !style.MarginBefore().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginBefore())
                                                      : null;
              }
              return !style.MarginStart().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginStart()) : null;
            }
            if (isRowDirection)
            {
              return !style.MarginAfter().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginAfter()) : null;
            }
            return !style.MarginEnd().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginEnd()) : null;
          };
          auto marginEnd = [&]() -> Maybe<LayoutUnit>
          {
            if (!isReversedInCrossAxis)
            {
              if (isRowDirection)
              {
                return !style.MarginAfter().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginAfter())
                                                     : null;
              }
              return !style.MarginEnd().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginEnd()) : null;
            }
            if (isRowDirection)
            {
              return !style.MarginBefore().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginBefore())
                                                    : null;
            }
            return !style.MarginStart().IsAuto() ? Maybe<LayoutUnit>(flexItemGeometry.MarginStart()) : null;
          };
          auto shouldFlipMargins =
            !isMainAxisParallelWithInlineAxis && Root().GetWritingMode().IsLineInverted();
          crossAxis.MarginStart = !shouldFlipMargins ? marginStart() : marginEnd();
          crossAxis.MarginEnd = !shouldFlipMargins ? marginEnd() : marginStart();

          crossAxis.HasSizeAuto =
            isMainAxisParallelWithInlineAxis ? style.Height().IsAuto() : style.Width().IsAuto();
          crossAxis.BorderAndPadding = isMainAxisParallelWithInlineAxis
                                         ? flexItemGeometry.VerticalBorderAndPadding()
                                         : flexItemGeometry.HorizontalBorderAndPadding();
        };
        setCrossAxisValues();

        auto flexItemOrder = style.GetOrder().Value;
        flexItemsNeedReordering =
          flexItemsNeedReordering || flexItemOrder != previousLogicalOrder.value_or(0);
        previousLogicalOrder = flexItemOrder;

        flexItemList.push_back({mainAxis, crossAxis, flexItemOrder, &Downcast<ElementBox>(*flexItem)});
      }
    };
    convertVisualToLogical();

    auto reorderFlexItemsIfApplicable = [&]
    {
      if (!flexItemsNeedReordering)
      {
        return;
      }

      std::ranges::stable_sort(flexItemList, {}, &FlexItem::LogicalOrder);
    };
    reorderFlexItemsIfApplicable();

    auto logicalFlexItemList = FlexLayout::LogicalFlexItems {};
    logicalFlexItemList.reserve(flexItemList.size());
    for (auto &flexItem : flexItemList)
    {
      logicalFlexItemList.emplace_back(*flexItem.LayoutBox, flexItem.MainAxis, flexItem.CrossAxis, false,
                                       false);
    }
    return logicalFlexItemList;
  }

  void FlexFormattingContext::SetFlexItemsGeometry(const FlexLayout::LogicalFlexItems &logicalFlexItemList,
                                                   const FlexLayout::LogicalFlexItemRects &logicalRects,
                                                   const ConstraintsForFlexContent &constraints) noexcept
  {
    auto &flexBoxStyle = Root().Style();
    auto flexDirection = flexBoxStyle.GetFlexDirection();
    auto isLeftToRightDirection = flexBoxStyle.GetWritingMode().IsLogicalLeftInlineStart();
    auto isRowDirection = flexDirection == FlexDirection::Row || flexDirection == FlexDirection::RowReverse;
    auto flexContainerContentBoxPosition = LayoutPoint {
      isRowDirection ? constraints.MainAxis().StartPosition : constraints.CrossAxis().StartPosition,
      isRowDirection ? constraints.CrossAxis().StartPosition : constraints.MainAxis().StartPosition};
    auto flexContainerMainAxisSize = [&]
    {
      if (auto size = constraints.MainAxis().AvailableSize)
      {
        return *size;
      }
      // Let's use content size when available size is inf.
      auto &lastFlexItem = logicalFlexItemList.back();
      auto &lastRect = logicalRects.back();
      return lastRect.Right() + lastRect.MarginRight()
             + (lastFlexItem.IsContentBoxBased()
                  ? GeometryForFlexItem(lastFlexItem.LayoutBox()).HorizontalBorderAndPadding()
                  : 0_lu);
    }();
    auto flexContainerCrossAxisSize = [&]
    {
      if (auto crossAxisSize = constraints.CrossAxis().AvailableSize)
      {
        return *crossAxisSize;
      }
      // In case of content size driven height in reversed cross axis direction (content is upside down), the
      // height is just the farthest point with content.
      auto maximumHeight = LayoutUnit {};
      for (auto &logicalRect : logicalRects)
      {
        maximumHeight = std::max(maximumHeight, logicalRect.Bottom() + logicalRect.MarginBottom());
      }
      return maximumHeight;
    }();

    for (size_t index = 0; index < logicalFlexItemList.size(); ++index)
    {
      auto &logicalFlexItem = logicalFlexItemList[index];
      auto &flexItemGeometry = GeometryForFlexItem(logicalFlexItem.LayoutBox());
      auto logicalRect = [&]
      {
        // Note that flex rects are inner size based.
        if (flexBoxStyle.GetFlexWrap() != FlexWrap::Reverse)
        {
          return logicalRects[index];
        }
        auto rect = logicalRects[index];
        auto adjustedLogicalBorderBoxTop = flexContainerCrossAxisSize - rect.Bottom();
        if (logicalFlexItem.IsContentBoxBased())
        {
          adjustedLogicalBorderBoxTop -= isRowDirection ? flexItemGeometry.VerticalBorderAndPadding()
                                                        : flexItemGeometry.HorizontalBorderAndPadding();
        }
        rect.SetTop(adjustedLogicalBorderBoxTop);
        return rect;
      }();

      auto borderBoxTop = LayoutUnit {};
      auto borderBoxLeft = LayoutUnit {};
      if (isRowDirection)
      {
        borderBoxTop += logicalRect.Top();
        if (flexDirection == FlexDirection::Row)
        {
          borderBoxLeft = logicalRect.Left();
        }
        else
        {
          borderBoxLeft = flexContainerMainAxisSize - logicalRect.Right();
          if (logicalFlexItem.IsContentBoxBased())
          {
            borderBoxLeft -= flexItemGeometry.HorizontalBorderAndPadding();
          }
        }
      }
      else
      {
        // Let's flip x and y to go from column to row.
        borderBoxLeft = logicalRect.Top();
        if (flexDirection == FlexDirection::Column)
        {
          borderBoxTop = logicalRect.Left();
        }
        else
        {
          borderBoxTop = flexContainerMainAxisSize - logicalRect.Right();
          if (logicalFlexItem.IsContentBoxBased())
          {
            borderBoxTop -= flexItemGeometry.VerticalBorderAndPadding();
          }
        }
      }
      auto contentBoxWidth = isRowDirection ? logicalRect.Width() : logicalRect.Height();
      auto contentBoxHeight = isRowDirection ? logicalRect.Height() : logicalRect.Width();
      if (!logicalFlexItem.IsContentBoxBased())
      {
        contentBoxWidth -= flexItemGeometry.HorizontalBorderAndPadding();
        contentBoxHeight -= flexItemGeometry.VerticalBorderAndPadding();
      }
      flexItemGeometry.SetContentBoxWidth(contentBoxWidth);
      flexItemGeometry.SetContentBoxHeight(contentBoxHeight);

      if (!isLeftToRightDirection)
      {
        borderBoxLeft = (isRowDirection ? flexContainerMainAxisSize : flexContainerCrossAxisSize)
                        - (borderBoxLeft + flexItemGeometry.BorderBoxWidth());
      }
      flexItemGeometry.SetTopLeft({flexContainerContentBoxPosition.X() + borderBoxLeft,
                                   flexContainerContentBoxPosition.Y() + borderBoxTop});
    }
  }

  void FlexFormattingContext::PositionOutOfFlowChildren() noexcept
  {
    // FIXME: Implement out-of-flow positioning.
    for (auto *outOfFlowChild = Root().FirstOutOfFlowChild(); outOfFlowChild;
         outOfFlowChild = outOfFlowChild->NextOutOfFlowSibling())
    {
      _globalLayoutState.EnsureGeometryForBox(*outOfFlowChild).SetTopLeft({});
    }
  }

  const BoxGeometry &FlexFormattingContext::GeometryForFlexItem(const Box &flexItem) const noexcept
  {
    krys_debug_assert(flexItem.IsFlexItem());
    return _globalLayoutState.GeometryForBox(flexItem);
  }

  BoxGeometry &FlexFormattingContext::GeometryForFlexItem(const Box &flexItem) noexcept
  {
    krys_debug_assert(flexItem.IsFlexItem());
    return _globalLayoutState.EnsureGeometryForBox(flexItem);
  }
}
