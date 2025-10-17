#include "Krystal.UI.Layout/Algorithm/AbsoluteLayout.hpp"
#include "Krystal.UI.Layout/Algorithm/Align.hpp"
#include "Krystal.UI.Layout/Algorithm/BoundAxis.hpp"
#include "Krystal.UI.Layout/Algorithm/CalculateLayout.hpp"
#include "Krystal.UI.Layout/Algorithm/TrailingPosition.hpp"

namespace Krys::UI::Layout
{
  static inline void SetFlexStartLayoutPosition(const Node *const parent, Node *child,
                                                const Direction direction, const FlexDirection axis,
                                                const float containingBlockWidth)
  {
    float position = child->GetStyle().ComputeFlexStartMargin(axis, direction, containingBlockWidth)
                     + parent->GetLayout().GetBorder(FlexStartEdge(axis));

    if (!child->HasErrata(Errata::AbsolutePositionWithoutInsetsExcludesPadding))
    {
      position += parent->GetLayout().GetPadding(FlexStartEdge(axis));
    }

    child->SetLayoutPosition(position, FlexStartEdge(axis));
  }

  static inline void SetFlexEndLayoutPosition(const Node *const parent, Node *child,
                                              const Direction direction, const FlexDirection axis,
                                              const float containingBlockWidth)
  {
    float flexEndPosition = parent->GetLayout().GetBorder(FlexEndEdge(axis))
                            + child->GetStyle().ComputeFlexEndMargin(axis, direction, containingBlockWidth);

    if (!child->HasErrata(Errata::AbsolutePositionWithoutInsetsExcludesPadding))
    {
      flexEndPosition += parent->GetLayout().GetPadding(FlexEndEdge(axis));
    }

    child->SetLayoutPosition(GetPositionOfOppositeEdge(flexEndPosition, axis, parent, child),
                             FlexStartEdge(axis));
  }

  static inline void SetCenterLayoutPosition(const Node *const parent, Node *child, const Direction direction,
                                             const FlexDirection axis, const float containingBlockWidth)
  {
    float parentContentBoxSize = parent->GetLayout().GetMeasuredDimension(ToDimension(axis))
                                 - parent->GetLayout().GetBorder(FlexStartEdge(axis))
                                 - parent->GetLayout().GetBorder(FlexEndEdge(axis));

    if (!child->HasErrata(Errata::AbsolutePositionWithoutInsetsExcludesPadding))
    {
      parentContentBoxSize -= parent->GetLayout().GetPadding(FlexStartEdge(axis));
      parentContentBoxSize -= parent->GetLayout().GetPadding(FlexEndEdge(axis));
    }

    const float childOuterSize = child->GetLayout().GetMeasuredDimension(ToDimension(axis))
                                 + child->GetStyle().ComputeMarginForAxis(axis, containingBlockWidth);

    float position = (parentContentBoxSize - childOuterSize) / 2.0f
                     + parent->GetLayout().GetBorder(FlexStartEdge(axis))
                     + child->GetStyle().ComputeFlexStartMargin(axis, direction, containingBlockWidth);

    if (!child->HasErrata(Errata::AbsolutePositionWithoutInsetsExcludesPadding))
    {
      position += parent->GetLayout().GetPadding(FlexStartEdge(axis));
    }

    child->SetLayoutPosition(position, FlexStartEdge(axis));
  }

  static void JustifyAbsoluteChild(const Node *const parent, Node *child, const Direction direction,
                                   const FlexDirection mainAxis, const float containingBlockWidth)
  {
    const Justify parentJustifyContent = parent->GetStyle().GetJustifyContent();
    switch (parentJustifyContent)
    {
      case Justify::FlexStart:
      case Justify::SpaceBetween:
        SetFlexStartLayoutPosition(parent, child, direction, mainAxis, containingBlockWidth);
        break;
      case Justify::FlexEnd:
        SetFlexEndLayoutPosition(parent, child, direction, mainAxis, containingBlockWidth);
        break;
      case Justify::Center:
      case Justify::SpaceAround:
      case Justify::SpaceEvenly:
        SetCenterLayoutPosition(parent, child, direction, mainAxis, containingBlockWidth);
        break;
    }
  }

  static void AlignAbsoluteChild(const Node *const parent, Node *child, const Direction direction,
                                 const FlexDirection crossAxis, const float containingBlockWidth)
  {
    Align itemAlign = ResolveChildAlignment(parent, child);
    const Wrap parentWrap = parent->GetStyle().GetFlexWrap();
    if (parentWrap == Wrap::WrapReverse)
    {
      if (itemAlign == Align::FlexEnd)
      {
        itemAlign = Align::FlexStart;
      }
      else if (itemAlign != Align::Center)
      {
        itemAlign = Align::FlexEnd;
      }
    }

    switch (itemAlign)
    {
      case Align::Auto:
      case Align::FlexStart:
      case Align::Baseline:
      case Align::SpaceAround:
      case Align::SpaceBetween:
      case Align::Stretch:
      case Align::SpaceEvenly:
        SetFlexStartLayoutPosition(parent, child, direction, crossAxis, containingBlockWidth);
        break;
      case Align::FlexEnd:
        SetFlexEndLayoutPosition(parent, child, direction, crossAxis, containingBlockWidth);
        break;
      case Align::Center:
        SetCenterLayoutPosition(parent, child, direction, crossAxis, containingBlockWidth);
        break;
    }
  }

  /*
   * Absolutely positioned nodes do not participate in flex layout and thus their
   * positions can be determined independently from the rest of their siblings.
   * For each axis there are essentially two cases:
   *
   * 1) The node has insets defined. In this case we can just use these to
   *    determine the position of the node.
   * 2) The node does not have insets defined. In this case we look at the GetStyle
   *    of the parent to position the node. Things like justify content and
   *    align content will move absolute children around. If none of these
   *    special properties are defined, the child is positioned at the start
   *    (defined by flex direction) of the leading flex line.
   *
   * This function does that positioning for the given axis. The spec has more
   * information on this topic: https://www.w3.org/TR/css-flexbox-1/#abspos-items
   */
  static void PositionAbsoluteChild(const Node *const containingNode, const Node *const parent, Node *child,
                                    const Direction direction, const FlexDirection axis,
                                    const bool isMainAxis, const float containingBlockWidth,
                                    const float containingBlockHeight)
  {
    const bool isAxisRow = IsRow(axis);
    const float containingBlockSize = isAxisRow ? containingBlockWidth : containingBlockHeight;

    // The inline-start position takes priority over the end position in the case
    // that they are both set and the node has a fixed width. Thus we only have 2
    // cases here: if inline-start is defined and if inline-end is defined.
    //
    // Despite checking inline-start to honor prioritization of insets, we write
    // to the flex-start edge because this algorithm works by positioning on the
    // flex-start edge and then filling in the flex-end direction at the end if
    // necessary.
    if (child->GetStyle().IsInlineStartPositionDefined(axis, direction)
        && !child->GetStyle().IsInlineStartPositionAuto(axis, direction))
    {
      const float positionRelativeToInlineStart =
        child->GetStyle().ComputeInlineStartPosition(axis, direction, containingBlockSize)
        + containingNode->GetStyle().ComputeInlineStartBorder(axis, direction)
        + child->GetStyle().ComputeInlineStartMargin(axis, direction, containingBlockSize);
      const float positionRelativeToFlexStart =
        InlineStartEdge(axis, direction) != FlexStartEdge(axis)
          ? GetPositionOfOppositeEdge(positionRelativeToInlineStart, axis, containingNode, child)
          : positionRelativeToInlineStart;

      child->SetLayoutPosition(positionRelativeToFlexStart, FlexStartEdge(axis));
    }
    else if (child->GetStyle().IsInlineEndPositionDefined(axis, direction)
             && !child->GetStyle().IsInlineEndPositionAuto(axis, direction))
    {
      const float positionRelativeToInlineStart =
        containingNode->GetLayout().GetMeasuredDimension(ToDimension(axis))
        - child->GetLayout().GetMeasuredDimension(ToDimension(axis))
        - containingNode->GetStyle().ComputeInlineEndBorder(axis, direction)
        - child->GetStyle().ComputeInlineEndMargin(axis, direction, containingBlockSize)
        - child->GetStyle().ComputeInlineEndPosition(axis, direction, containingBlockSize);
      const float positionRelativeToFlexStart =
        InlineStartEdge(axis, direction) != FlexStartEdge(axis)
          ? GetPositionOfOppositeEdge(positionRelativeToInlineStart, axis, containingNode, child)
          : positionRelativeToInlineStart;

      child->SetLayoutPosition(positionRelativeToFlexStart, FlexStartEdge(axis));
    }
    else
    {
      isMainAxis ? JustifyAbsoluteChild(parent, child, direction, axis, containingBlockWidth)
                 : AlignAbsoluteChild(parent, child, direction, axis, containingBlockWidth);
    }
  }

  void LayoutAbsoluteChild(const Node *containingNode, const Node *node, Node *child,
                           float containingBlockWidth, float containingBlockHeight, SizingMode widthMode,
                           Direction direction, LayoutData &layoutMarkerData, uint32 depth,
                           uint32 generationCount)
  {
    const FlexDirection mainAxis = ResolveDirection(node->GetStyle().GetFlexDirection(), direction);
    const FlexDirection crossAxis = ResolveCrossDirection(mainAxis, direction);
    const bool isMainAxisRow = IsRow(mainAxis);

    float childWidth = std::numeric_limits<float>::quiet_NaN();
    float childHeight = std::numeric_limits<float>::quiet_NaN();
    SizingMode childWidthSizingMode = SizingMode::MaxContent;
    SizingMode childHeightSizingMode = SizingMode::MaxContent;

    auto marginRow = child->GetStyle().ComputeMarginForAxis(FlexDirection::Row, containingBlockWidth);
    auto marginColumn = child->GetStyle().ComputeMarginForAxis(FlexDirection::Column, containingBlockWidth);

    if (child->HasDefiniteLength(Dimension::Width, containingBlockWidth))
    {
      childWidth =
        child->GetResolvedDimension(direction, Dimension::Width, containingBlockWidth, containingBlockWidth)
          .Value()
        + marginRow;
    }
    else
    {
      // If the child doesn't have a specified width, compute the width based on
      // the left/right offsets if they're defined.
      if (child->GetStyle().IsFlexStartPositionDefined(FlexDirection::Row, direction)
          && child->GetStyle().IsFlexEndPositionDefined(FlexDirection::Row, direction)
          && !child->GetStyle().IsFlexStartPositionAuto(FlexDirection::Row, direction)
          && !child->GetStyle().IsFlexEndPositionAuto(FlexDirection::Row, direction))
      {
        childWidth =
          containingNode->GetLayout().GetMeasuredDimension(Dimension::Width)
          - (containingNode->GetStyle().ComputeFlexStartBorder(FlexDirection::Row, direction)
             + containingNode->GetStyle().ComputeFlexEndBorder(FlexDirection::Row, direction))
          - (child->GetStyle().ComputeFlexStartPosition(FlexDirection::Row, direction, containingBlockWidth)
             + child->GetStyle().ComputeFlexEndPosition(FlexDirection::Row, direction, containingBlockWidth));
        childWidth = BoundAxis(child, FlexDirection::Row, direction, childWidth, containingBlockWidth,
                               containingBlockWidth);
      }
    }

    if (child->HasDefiniteLength(Dimension::Height, containingBlockHeight))
    {
      childHeight =
        child->GetResolvedDimension(direction, Dimension::Height, containingBlockHeight, containingBlockWidth)
          .Value()
        + marginColumn;
    }
    else
    {
      // If the child doesn't have a specified height, compute the height based
      // on the top/bottom offsets if they're defined.
      if (child->GetStyle().IsFlexStartPositionDefined(FlexDirection::Column, direction)
          && child->GetStyle().IsFlexEndPositionDefined(FlexDirection::Column, direction)
          && !child->GetStyle().IsFlexStartPositionAuto(FlexDirection::Column, direction)
          && !child->GetStyle().IsFlexEndPositionAuto(FlexDirection::Column, direction))
      {
        childHeight = containingNode->GetLayout().GetMeasuredDimension(Dimension::Height)
                      - (containingNode->GetStyle().ComputeFlexStartBorder(FlexDirection::Column, direction)
                         + containingNode->GetStyle().ComputeFlexEndBorder(FlexDirection::Column, direction))
                      - (child->GetStyle().ComputeFlexStartPosition(FlexDirection::Column, direction,
                                                                    containingBlockHeight)
                         + child->GetStyle().ComputeFlexEndPosition(FlexDirection::Column, direction,
                                                                    containingBlockHeight));
        childHeight = BoundAxis(child, FlexDirection::Column, direction, childHeight, containingBlockHeight,
                                containingBlockWidth);
      }
    }

    // Exactly one dimension needs to be defined for us to be able to do aspect
    // ratio calculation. One dimension being the anchor and the other being
    // flexible.
    const auto &childStyle = child->GetStyle();
    if (Krys::IsUndefined(childWidth) ^ Krys::IsUndefined(childHeight))
    {
      if (childStyle.GetAspectRatio().HasValue())
      {
        if (Krys::IsUndefined(childWidth))
        {
          childWidth = marginRow + (childHeight - marginColumn) * childStyle.GetAspectRatio().Value();
        }
        else if (Krys::IsUndefined(childHeight))
        {
          childHeight = marginColumn + (childWidth - marginRow) / childStyle.GetAspectRatio().Value();
        }
      }
    }

    // If we're still missing one or the other dimension, measure the content.
    if (Krys::IsUndefined(childWidth) || Krys::IsUndefined(childHeight))
    {
      childWidthSizingMode = Krys::IsUndefined(childWidth) ? SizingMode::MaxContent : SizingMode::StretchFit;
      childHeightSizingMode =
        Krys::IsUndefined(childHeight) ? SizingMode::MaxContent : SizingMode::StretchFit;

      // If the size of the owner is defined then try to constrain the absolute
      // child to that size as well. This allows text within the absolute child
      // to wrap to the size of its owner. This is the same behavior as many
      // browsers implement.
      if (!isMainAxisRow && Krys::IsUndefined(childWidth) && widthMode != SizingMode::MaxContent
          && Krys::IsDefined(containingBlockWidth) && containingBlockWidth > 0)
      {
        childWidth = containingBlockWidth;
        childWidthSizingMode = SizingMode::FitContent;
      }

      CalculateLayoutInternal(child, childWidth, childHeight, direction, childWidthSizingMode,
                              childHeightSizingMode, containingBlockWidth, containingBlockHeight, false,
                              LayoutPassReason::kAbsMeasureChild, layoutMarkerData, depth, generationCount);
      childWidth = child->GetLayout().GetMeasuredDimension(Dimension::Width)
                   + child->GetStyle().ComputeMarginForAxis(FlexDirection::Row, containingBlockWidth);
      childHeight = child->GetLayout().GetMeasuredDimension(Dimension::Height)
                    + child->GetStyle().ComputeMarginForAxis(FlexDirection::Column, containingBlockWidth);
    }

    CalculateLayoutInternal(child, childWidth, childHeight, direction, SizingMode::StretchFit,
                            SizingMode::StretchFit, containingBlockWidth, containingBlockHeight, true,
                            LayoutPassReason::kAbsLayout, layoutMarkerData, depth, generationCount);

    PositionAbsoluteChild(containingNode, node, child, direction, mainAxis, true /*isMainAxis*/,
                          containingBlockWidth, containingBlockHeight);
    PositionAbsoluteChild(containingNode, node, child, direction, crossAxis, false /*isMainAxis*/,
                          containingBlockWidth, containingBlockHeight);
  }

  bool LayoutAbsoluteDescendants(Node *containingNode, Node *currentNode, SizingMode widthSizingMode,
                                 Direction currentNodeDirection, LayoutData &layoutMarkerData,
                                 uint32 currentDepth, uint32 generationCount,
                                 float currentNodeLeftOffsetFromContainingBlock,
                                 float currentNodeTopOffsetFromContainingBlock,
                                 float containingNodeAvailableInnerWidth,
                                 float containingNodeAvailableInnerHeight)
  {
    bool hasNewLayout = false;
    for (auto child : currentNode->GetLayoutChildren())
    {
      if (child->GetStyle().GetDisplay() == DisplayType::None)
      {
        continue;
      }
      else if (child->GetStyle().GetPositionType() == PositionType::Absolute)
      {
        const bool absoluteErrata = currentNode->HasErrata(Errata::AbsolutePercentAgainstInnerSize);
        const float containingBlockWidth =
          absoluteErrata ? containingNodeAvailableInnerWidth
                         : containingNode->GetLayout().GetMeasuredDimension(Dimension::Width)
                             - containingNode->GetStyle().ComputeBorderForAxis(FlexDirection::Row);
        const float containingBlockHeight =
          absoluteErrata ? containingNodeAvailableInnerHeight
                         : containingNode->GetLayout().GetMeasuredDimension(Dimension::Height)
                             - containingNode->GetStyle().ComputeBorderForAxis(FlexDirection::Column);

        LayoutAbsoluteChild(containingNode, currentNode, child, containingBlockWidth, containingBlockHeight,
                            widthSizingMode, currentNodeDirection, layoutMarkerData, currentDepth,
                            generationCount);

        hasNewLayout = hasNewLayout || child->GetHasNewLayout();

        /*
         * At this point the child has its position set but only on its the
         * parent's flexStart edge. Additionally, this position should be
         * interpreted relative to the containing block of the child if it had
         * insets defined. So we need to adjust the position by subtracting the
         * the parents offset from the containing block. However, getting that
         * offset is complicated since the two nodes can have different main/cross
         * axes.
         */
        const FlexDirection parentMainAxis =
          ResolveDirection(currentNode->GetStyle().GetFlexDirection(), currentNodeDirection);
        const FlexDirection parentCrossAxis = ResolveCrossDirection(parentMainAxis, currentNodeDirection);

        if (NeedsTrailingPosition(parentMainAxis))
        {
          const bool mainInsetsDefined = IsRow(parentMainAxis) ? child->GetStyle().HorizontalInsetsDefined()
                                                               : child->GetStyle().VerticalInsetsDefined();
          SetChildTrailingPosition(mainInsetsDefined ? containingNode : currentNode, child, parentMainAxis);
        }
        if (NeedsTrailingPosition(parentCrossAxis))
        {
          const bool crossInsetsDefined = IsRow(parentCrossAxis) ? child->GetStyle().HorizontalInsetsDefined()
                                                                 : child->GetStyle().VerticalInsetsDefined();
          SetChildTrailingPosition(crossInsetsDefined ? containingNode : currentNode, child, parentCrossAxis);
        }

        /*
         * At this point we know the left and top physical edges of the child are
         * set with positions that are relative to the containing block if insets
         * are defined
         */
        const float childLeftPosition = child->GetLayout().GetPosition(PhysicalEdge::Left);
        const float childTopPosition = child->GetLayout().GetPosition(PhysicalEdge::Top);

        const float childLeftOffsetFromParent =
          child->GetStyle().HorizontalInsetsDefined()
            ? (childLeftPosition - currentNodeLeftOffsetFromContainingBlock)
            : childLeftPosition;
        const float childTopOffsetFromParent =
          child->GetStyle().VerticalInsetsDefined()
            ? (childTopPosition - currentNodeTopOffsetFromContainingBlock)
            : childTopPosition;

        child->SetLayoutPosition(childLeftOffsetFromParent, PhysicalEdge::Left);
        child->SetLayoutPosition(childTopOffsetFromParent, PhysicalEdge::Top);
      }
      else if (child->GetStyle().GetPositionType() == PositionType::Static
               && !child->AlwaysFormsContainingBlock())
      {
        // We may write new layout results for absolute descendants of "child"
        // which are positioned relative to the current containing block instead
        // of their parent. "child" may not be dirty, or have new constraints, so
        // absolute positioning may be the first time during this layout pass that
        // we need to mutate these descendents. Make sure the path of
        // nodes to them is mutable before positioning.
        child->CloneChildrenIfNeeded();
        const Direction childDirection = child->ResolveDirection(currentNodeDirection);
        // By now all descendants of the containing block that are not absolute
        // will have their positions set for left and top.
        const float childLeftOffsetFromContainingBlock =
          currentNodeLeftOffsetFromContainingBlock + child->GetLayout().GetPosition(PhysicalEdge::Left);
        const float childTopOffsetFromContainingBlock =
          currentNodeTopOffsetFromContainingBlock + child->GetLayout().GetPosition(PhysicalEdge::Top);

        hasNewLayout =
          LayoutAbsoluteDescendants(containingNode, child, widthSizingMode, childDirection, layoutMarkerData,
                                    currentDepth + 1, generationCount, childLeftOffsetFromContainingBlock,
                                    childTopOffsetFromContainingBlock, containingNodeAvailableInnerWidth,
                                    containingNodeAvailableInnerHeight)
          || hasNewLayout;

        if (hasNewLayout)
        {
          child->SetHasNewLayout(hasNewLayout);
        }
      }
    }
    return hasNewLayout;
  }
}