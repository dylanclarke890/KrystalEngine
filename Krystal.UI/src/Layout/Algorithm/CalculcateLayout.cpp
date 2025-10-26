#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.UI/Layout/Algorithm/AbsoluteLayout.hpp"
#include "Krystal.UI/Layout/Algorithm/Align.hpp"
#include "Krystal.UI/Layout/Algorithm/Baseline.hpp"
#include "Krystal.UI/Layout/Algorithm/BoundAxis.hpp"
#include "Krystal.UI/Layout/Algorithm/Cache.hpp"
#include "Krystal.UI/Layout/Algorithm/CalculateLayout.hpp"
#include "Krystal.UI/Layout/Algorithm/FlexLine.hpp"
#include "Krystal.UI/Layout/Algorithm/PixelGrid.hpp"
#include "Krystal.UI/Layout/Algorithm/SizingMode.hpp"
#include "Krystal.UI/Layout/Algorithm/TrailingPosition.hpp"
#include "Krystal.UI/Layout/Event/Event.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Helpers/FlexDirection.hpp"
#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cmath>
#include <cstring>

namespace Krys::UI
{
  std::atomic<uint16> gCurrentGenerationCount(0);

  static void ConstrainMaxSizeForMode(const Node *node, Direction direction, FlexDirection axis,
                                      float ownerAxisSize, float ownerWidth,
                                      /*in_out*/ SizingMode *mode,
                                      /*in_out*/ float *size)
  {
    const NullableFloat maxSize =
      node->GetStyle().ResolvedMaxDimension(direction, ToDimension(axis), ownerAxisSize, ownerWidth)
      + NullableFloat(node->GetStyle().ComputeMarginForAxis(axis, ownerWidth));
    switch (*mode)
    {
      case SizingMode::StretchFit:
      case SizingMode::FitContent:
        *size = (!maxSize.HasValue() || *size < maxSize.Value()) ? *size : maxSize.Value();
        break;
      case SizingMode::MaxContent:
        if (maxSize.HasValue())
        {
          *mode = SizingMode::FitContent;
          *size = maxSize.Value();
        }
        break;
    }
  }

  static void ComputeFlexBasisForChild(const Node *const node, Node *const child, const float width,
                                       const SizingMode widthMode, const float height,
                                       const float ownerWidth, const float ownerHeight,
                                       const SizingMode heightMode, const Direction direction,
                                       LayoutData &layoutMarkerData, const uint32_t depth,
                                       const uint16 generationCount)
  {
    const FlexDirection mainAxis = ResolveDirection(node->GetStyle().GetFlexDirection(), direction);
    const bool isMainAxisRow = IsRow(mainAxis);
    const float mainAxisSize = isMainAxisRow ? width : height;
    const float mainAxisOwnerSize = isMainAxisRow ? ownerWidth : ownerHeight;

    float childWidth = std::numeric_limits<float>::quiet_NaN();
    float childHeight = std::numeric_limits<float>::quiet_NaN();
    SizingMode childWidthSizingMode;
    SizingMode childHeightSizingMode;

    const NullableFloat resolvedFlexBasis =
      child->ResolveFlexBasis(direction, mainAxis, mainAxisOwnerSize, ownerWidth);
    const bool isRowStyleDimDefined = child->HasDefiniteLength(Dimension::Width, ownerWidth);
    const bool isColumnStyleDimDefined = child->HasDefiniteLength(Dimension::Height, ownerHeight);

    if (resolvedFlexBasis.HasValue() && Krys::IsDefined(mainAxisSize))
    {
      if (!child->GetLayout().ComputedFlexBasis.HasValue()
          || (child->GetConfig()->IsExperimentalFeatureEnabled(ExperimentalFeature::WebFlexBasis)
              && child->GetLayout().ComputedFlexBasisGeneration != generationCount))
      {
        const NullableFloat GetPaddingAndBorder =
          NullableFloat(PaddingAndBorderForAxis(child, mainAxis, direction, ownerWidth));
        child->SetLayoutComputedFlexBasis(Krys::MaxOrDefined(resolvedFlexBasis, GetPaddingAndBorder));
      }
    }
    else if (isMainAxisRow && isRowStyleDimDefined)
    {
      // The width is definite, so use that as the flex basis.
      const NullableFloat GetPaddingAndBorder =
        NullableFloat(PaddingAndBorderForAxis(child, FlexDirection::Row, direction, ownerWidth));

      child->SetLayoutComputedFlexBasis(
        Krys::MaxOrDefined(child->GetResolvedDimension(direction, Dimension::Width, ownerWidth, ownerWidth),
                           GetPaddingAndBorder));
    }
    else if (!isMainAxisRow && isColumnStyleDimDefined)
    {
      // The height is definite, so use that as the flex basis.
      const NullableFloat GetPaddingAndBorder =
        NullableFloat(PaddingAndBorderForAxis(child, FlexDirection::Column, direction, ownerWidth));
      child->SetLayoutComputedFlexBasis(
        Krys::MaxOrDefined(child->GetResolvedDimension(direction, Dimension::Height, ownerHeight, ownerWidth),
                           GetPaddingAndBorder));
    }
    else
    {
      // Compute the flex basis and hypothetical main size (i.e. the clamped flex
      // basis).
      childWidthSizingMode = SizingMode::MaxContent;
      childHeightSizingMode = SizingMode::MaxContent;

      auto marginRow = child->GetStyle().ComputeMarginForAxis(FlexDirection::Row, ownerWidth);
      auto marginColumn = child->GetStyle().ComputeMarginForAxis(FlexDirection::Column, ownerWidth);

      if (isRowStyleDimDefined)
      {
        childWidth = child->GetResolvedDimension(direction, Dimension::Width, ownerWidth, ownerWidth).Value()
                     + marginRow;
        childWidthSizingMode = SizingMode::StretchFit;
      }
      if (isColumnStyleDimDefined)
      {
        childHeight =
          child->GetResolvedDimension(direction, Dimension::Height, ownerHeight, ownerWidth).Value()
          + marginColumn;
        childHeightSizingMode = SizingMode::StretchFit;
      }

      // The W3C spec doesn't say anything about the 'overflow' property, but all
      // major browsers appear to implement the following logic.
      if ((!isMainAxisRow && node->GetStyle().GetOverflow() == Overflow::Scroll)
          || node->GetStyle().GetOverflow() != Overflow::Scroll)
      {
        if (Krys::IsUndefined(childWidth) && Krys::IsDefined(width))
        {
          childWidth = width;
          childWidthSizingMode = SizingMode::FitContent;
        }
      }

      if ((isMainAxisRow && node->GetStyle().GetOverflow() == Overflow::Scroll)
          || node->GetStyle().GetOverflow() != Overflow::Scroll)
      {
        if (Krys::IsUndefined(childHeight) && Krys::IsDefined(height))
        {
          childHeight = height;
          childHeightSizingMode = SizingMode::FitContent;
        }
      }

      const auto &childStyle = child->GetStyle();
      if (childStyle.GetAspectRatio().HasValue())
      {
        if (!isMainAxisRow && childWidthSizingMode == SizingMode::StretchFit)
        {
          childHeight = marginColumn + (childWidth - marginRow) / childStyle.GetAspectRatio().Value();
          childHeightSizingMode = SizingMode::StretchFit;
        }
        else if (isMainAxisRow && childHeightSizingMode == SizingMode::StretchFit)
        {
          childWidth = marginRow + (childHeight - marginColumn) * childStyle.GetAspectRatio().Value();
          childWidthSizingMode = SizingMode::StretchFit;
        }
      }

      // If child has no defined size in the cross axis and is set to stretch, set
      // the cross axis to be measured exactly with the available inner width

      const bool hasExactWidth = Krys::IsDefined(width) && widthMode == SizingMode::StretchFit;
      const bool childWidthStretch = ResolveChildAlignment(node, child) == Align::Stretch
                                     && childWidthSizingMode != SizingMode::StretchFit;
      if (!isMainAxisRow && !isRowStyleDimDefined && hasExactWidth && childWidthStretch)
      {
        childWidth = width;
        childWidthSizingMode = SizingMode::StretchFit;
        if (childStyle.GetAspectRatio().HasValue())
        {
          childHeight = (childWidth - marginRow) / childStyle.GetAspectRatio().Value();
          childHeightSizingMode = SizingMode::StretchFit;
        }
      }

      const bool hasExactHeight = Krys::IsDefined(height) && heightMode == SizingMode::StretchFit;
      const bool childHeightStretch = ResolveChildAlignment(node, child) == Align::Stretch
                                      && childHeightSizingMode != SizingMode::StretchFit;
      if (isMainAxisRow && !isColumnStyleDimDefined && hasExactHeight && childHeightStretch)
      {
        childHeight = height;
        childHeightSizingMode = SizingMode::StretchFit;

        if (childStyle.GetAspectRatio().HasValue())
        {
          childWidth = (childHeight - marginColumn) * childStyle.GetAspectRatio().Value();
          childWidthSizingMode = SizingMode::StretchFit;
        }
      }

      ConstrainMaxSizeForMode(child, direction, FlexDirection::Row, ownerWidth, ownerWidth,
                              &childWidthSizingMode, &childWidth);
      ConstrainMaxSizeForMode(child, direction, FlexDirection::Column, ownerHeight, ownerWidth,
                              &childHeightSizingMode, &childHeight);

      // Measure the child
      CalculateLayoutInternal(child, childWidth, childHeight, direction, childWidthSizingMode,
                              childHeightSizingMode, ownerWidth, ownerHeight, false,
                              LayoutPassReason::kMeasureChild, layoutMarkerData, depth, generationCount);

      child->SetLayoutComputedFlexBasis(
        NullableFloat(Krys::MaxOrDefined(child->GetLayout().GetMeasuredDimension(ToDimension(mainAxis)),
                                         PaddingAndBorderForAxis(child, mainAxis, direction, ownerWidth))));
    }
    child->SetLayoutComputedFlexBasisGeneration(generationCount);
  }

  static void MeasureNodeWithMeasureFunc(Node *const node, const Direction direction, float availableWidth,
                                         float availableHeight, const SizingMode widthSizingMode,
                                         const SizingMode heightSizingMode, const float ownerWidth,
                                         const float ownerHeight, LayoutData &layoutMarkerData,
                                         const LayoutPassReason reason)
  {
    if (!node->HasMeasureFunc())
    {
      throw std::runtime_error("Expected node to have custom measure function");
    }

    if (widthSizingMode == SizingMode::MaxContent)
    {
      availableWidth = std::numeric_limits<float>::quiet_NaN();
    }
    if (heightSizingMode == SizingMode::MaxContent)
    {
      availableHeight = std::numeric_limits<float>::quiet_NaN();
    }

    const auto &layout = node->GetLayout();
    const float GetPaddingAndBorderAxisRow =
      layout.GetPadding(PhysicalEdge::Left) + layout.GetPadding(PhysicalEdge::Right)
      + layout.GetBorder(PhysicalEdge::Left) + layout.GetBorder(PhysicalEdge::Right);
    const float GetPaddingAndBorderAxisColumn =
      layout.GetPadding(PhysicalEdge::Top) + layout.GetPadding(PhysicalEdge::Bottom)
      + layout.GetBorder(PhysicalEdge::Top) + layout.GetBorder(PhysicalEdge::Bottom);

    // We want to make sure we don't call measure with negative size
    const float innerWidth = Krys::IsUndefined(availableWidth)
                               ? availableWidth
                               : Krys::MaxOrDefined(0.0f, availableWidth - GetPaddingAndBorderAxisRow);
    const float innerHeight = Krys::IsUndefined(availableHeight)
                                ? availableHeight
                                : Krys::MaxOrDefined(0.0f, availableHeight - GetPaddingAndBorderAxisColumn);

    if (widthSizingMode == SizingMode::StretchFit && heightSizingMode == SizingMode::StretchFit)
    {
      // Don't bother sizing the text if both dimensions are already defined.
      node->SetLayoutMeasuredDimension(
        BoundAxis(node, FlexDirection::Row, direction, availableWidth, ownerWidth, ownerWidth),
        Dimension::Width);
      node->SetLayoutMeasuredDimension(
        BoundAxis(node, FlexDirection::Column, direction, availableHeight, ownerHeight, ownerWidth),
        Dimension::Height);
    }
    else
    {
      Event::Publish<Event::MeasureCallbackStart>(node);

      // Measure the text under the current constraints.
      const Size measuredSize = node->Measure(innerWidth, ToMeasureMode(widthSizingMode), innerHeight,
                                              ToMeasureMode(heightSizingMode));

      layoutMarkerData.MeasureCallbacks += 1;
      layoutMarkerData.MeasureCallbackReasonsCount[static_cast<size_t>(reason)] += 1;

      Event::Publish<Event::MeasureCallbackEnd>(node, {innerWidth, ToMeasureMode(widthSizingMode),
                                                       innerHeight, ToMeasureMode(heightSizingMode),
                                                       measuredSize.Width, measuredSize.Height, reason});

      node->SetLayoutMeasuredDimension(
        BoundAxis(node, FlexDirection::Row, direction,
                  (widthSizingMode == SizingMode::MaxContent || widthSizingMode == SizingMode::FitContent)
                    ? measuredSize.Width + GetPaddingAndBorderAxisRow
                    : availableWidth,
                  ownerWidth, ownerWidth),
        Dimension::Width);

      node->SetLayoutMeasuredDimension(
        BoundAxis(node, FlexDirection::Column, direction,
                  (heightSizingMode == SizingMode::MaxContent || heightSizingMode == SizingMode::FitContent)
                    ? measuredSize.Height + GetPaddingAndBorderAxisColumn
                    : availableHeight,
                  ownerHeight, ownerWidth),
        Dimension::Height);
    }
  }

  // For nodes with no children, use the available values if they were provided,
  // or the minimum size as indicated by the GetPadding and GetBorder sizes.
  static void MeasureNodeWithoutChildren(Node *const node, const Direction direction,
                                         const float availableWidth, const float availableHeight,
                                         const SizingMode widthSizingMode,
                                         const SizingMode heightSizingMode, const float ownerWidth,
                                         const float ownerHeight)
  {
    const auto &layout = node->GetLayout();

    float width = availableWidth;
    if (widthSizingMode == SizingMode::MaxContent || widthSizingMode == SizingMode::FitContent)
    {
      width = layout.GetPadding(PhysicalEdge::Left) + layout.GetPadding(PhysicalEdge::Right)
              + layout.GetBorder(PhysicalEdge::Left) + layout.GetBorder(PhysicalEdge::Right);
    }
    node->SetLayoutMeasuredDimension(
      BoundAxis(node, FlexDirection::Row, direction, width, ownerWidth, ownerWidth), Dimension::Width);

    float height = availableHeight;
    if (heightSizingMode == SizingMode::MaxContent || heightSizingMode == SizingMode::FitContent)
    {
      height = layout.GetPadding(PhysicalEdge::Top) + layout.GetPadding(PhysicalEdge::Bottom)
               + layout.GetBorder(PhysicalEdge::Top) + layout.GetBorder(PhysicalEdge::Bottom);
    }
    node->SetLayoutMeasuredDimension(
      BoundAxis(node, FlexDirection::Column, direction, height, ownerHeight, ownerWidth), Dimension::Height);
  }

  static bool IsFixedSize(float dim, SizingMode sizingMode)
  {
    return sizingMode == SizingMode::StretchFit
           || (Krys::IsDefined(dim) && sizingMode == SizingMode::FitContent && dim <= 0.0);
  }

  static bool MeasureNodeWithFixedSize(Node *const node, const Direction direction,
                                       const float availableWidth, const float availableHeight,
                                       const SizingMode widthSizingMode,
                                       const SizingMode heightSizingMode, const float ownerWidth,
                                       const float ownerHeight)
  {
    if (IsFixedSize(availableWidth, widthSizingMode) && IsFixedSize(availableHeight, heightSizingMode))
    {
      node->SetLayoutMeasuredDimension(
        BoundAxis(node, FlexDirection::Row, direction,
                  Krys::IsUndefined(availableWidth)
                      || (widthSizingMode == SizingMode::FitContent && availableWidth < 0.0f)
                    ? 0.0f
                    : availableWidth,
                  ownerWidth, ownerWidth),
        Dimension::Width);

      node->SetLayoutMeasuredDimension(
        BoundAxis(node, FlexDirection::Column, direction,
                  Krys::IsUndefined(availableHeight)
                      || (heightSizingMode == SizingMode::FitContent && availableHeight < 0.0f)
                    ? 0.0f
                    : availableHeight,
                  ownerHeight, ownerWidth),
        Dimension::Height);
      return true;
    }

    return false;
  }

  static void ZeroOutLayoutRecursively(Node *const node)
  {
    node->GetLayout() = {};
    node->SetLayoutDimension(0, Dimension::Width);
    node->SetLayoutDimension(0, Dimension::Height);
    node->SetHasNewLayout(true);

    node->CloneChildrenIfNeeded();
    for (const auto child : node->GetChildren())
    {
      ZeroOutLayoutRecursively(child);
    }
  }

  static void CleanupContentsNodesRecursively(Node *const node)
  {
    if (node->HasContentsChildren()) [[unlikely]]
    {
      node->CloneContentsChildrenIfNeeded();
      for (auto child : node->GetChildren())
      {
        if (child->GetStyle().GetDisplay() == Display::Contents)
        {
          child->GetLayout() = {};
          child->SetLayoutDimension(0, Dimension::Width);
          child->SetLayoutDimension(0, Dimension::Height);
          child->SetHasNewLayout(true);
          child->SetDirty(false);
          child->CloneChildrenIfNeeded();

          CleanupContentsNodesRecursively(child);
        }
      }
    }
  }

  static float CalculateAvailableInnerDimension(const Node *const node, const Direction direction,
                                                const Dimension dimension, const float availableDim,
                                                const float GetPaddingAndBorder, const float ownerDim,
                                                const float ownerWidth)
  {
    float availableInnerDim = availableDim - GetPaddingAndBorder;
    // Max dimension overrides predefined dimension value; Min dimension in turn
    // overrides both of the above
    if (Krys::IsDefined(availableInnerDim))
    {
      // We want to make sure our available height does not violate min and max
      // constraints
      const NullableFloat minDimensionOptional =
        node->GetStyle().ResolvedMinDimension(direction, dimension, ownerDim, ownerWidth);
      const float minInnerDim =
        !minDimensionOptional.HasValue() ? 0.0f : minDimensionOptional.Value() - GetPaddingAndBorder;

      const NullableFloat maxDimensionOptional =
        node->GetStyle().ResolvedMaxDimension(direction, dimension, ownerDim, ownerWidth);

      const float maxInnerDim =
        !maxDimensionOptional.HasValue() ? FLT_MAX : maxDimensionOptional.Value() - GetPaddingAndBorder;
      availableInnerDim = Krys::MaxOrDefined(Krys::MinOrDefined(availableInnerDim, maxInnerDim), minInnerDim);
    }

    return availableInnerDim;
  }

  static float ComputeFlexBasisForChildren(Node *const node, const float availableInnerWidth,
                                           const float availableInnerHeight,
                                           SizingMode widthSizingMode,
                                           SizingMode heightSizingMode, Direction direction,
                                           FlexDirection mainAxis, bool performLayout,
                                           LayoutData &layoutMarkerData, const uint32_t depth,
                                           const uint16 generationCount)
  {
    float totalOuterFlexBasis = 0.0f;
    NodeRef singleFlexChild = nullptr;
    auto children = node->GetLayoutChildren();
    SizingMode sizingModeMainDim = IsRow(mainAxis) ? widthSizingMode : heightSizingMode;
    // If there is only one child with flexGrow + flexShrink it means we can set
    // the ComputedFlexBasis to 0 instead of measuring and shrinking / flexing the
    // child to exactly match the remaining space
    if (sizingModeMainDim == SizingMode::StretchFit)
    {
      for (auto child : children)
      {
        if (child->IsNodeFlexible())
        {
          if (singleFlexChild != nullptr || Krys::InexactEquals(child->ResolveFlexGrow(), 0.0f)
              || Krys::InexactEquals(child->ResolveFlexShrink(), 0.0f))
          {
            // There is already a flexible child, or this flexible child doesn't
            // have flexGrow and flexShrink, abort
            singleFlexChild = nullptr;
            break;
          }
          else
          {
            singleFlexChild = child;
          }
        }
      }
    }

    for (auto child : children)
    {
      child->ProcessDimensions();
      if (child->GetStyle().GetDisplay() == Display::None)
      {
        ZeroOutLayoutRecursively(child);
        child->SetHasNewLayout(true);
        child->SetDirty(false);
        continue;
      }
      if (performLayout)
      {
        // Set the initial position (relative to the owner).
        const Direction childDirection = child->ResolveDirection(direction);
        child->SetPosition(childDirection, availableInnerWidth, availableInnerHeight);
      }

      if (child->GetStyle().GetPositionType() == PositionType::Absolute)
      {
        continue;
      }
      if (child == singleFlexChild)
      {
        child->SetLayoutComputedFlexBasisGeneration(generationCount);
        child->SetLayoutComputedFlexBasis(NullableFloat(0));
      }
      else
      {
        ComputeFlexBasisForChild(node, child, availableInnerWidth, widthSizingMode, availableInnerHeight,
                                 availableInnerWidth, availableInnerHeight, heightSizingMode, direction,
                                 layoutMarkerData, depth, generationCount);
      }

      totalOuterFlexBasis += (child->GetLayout().ComputedFlexBasis.Value()
                              + child->GetStyle().ComputeMarginForAxis(mainAxis, availableInnerWidth));
    }

    return totalOuterFlexBasis;
  }

  // It distributes the free space to the flexible items and ensures that the size
  // of the flex items abide the min and max constraints. At the end of this
  // function the child nodes would have proper size. Prior using this function
  // please ensure that distributeFreeSpaceFirstPass is called.
  static float DistributeFreeSpaceSecondPass(
    FlexLine &flexLine, Node *const node, const FlexDirection mainAxis, const FlexDirection crossAxis,
    const Direction direction, const float ownerWidth, const float mainAxisOwnerSize,
    const float availableInnerMainDim, const float availableInnerCrossDim, const float availableInnerWidth,
    const float availableInnerHeight, const bool mainAxisOverflows,
    const SizingMode sizingModeCrossDim, const bool performLayout, LayoutData &layoutMarkerData,
    const uint32 depth, const uint16 generationCount)
  {
    float childFlexBasis = 0;
    float flexShrinkScaledFactor = 0;
    float flexGrowFactor = 0;
    float deltaFreeSpace = 0;
    const bool isMainAxisRow = IsRow(mainAxis);
    const bool isNodeFlexWrap = node->GetStyle().GetFlexWrap() != Wrap::NoWrap;

    for (auto currentLineChild : flexLine.ItemsInFlow)
    {
      childFlexBasis = BoundAxisWithinMinAndMax(currentLineChild, direction, mainAxis,
                                                currentLineChild->GetLayout().ComputedFlexBasis,
                                                mainAxisOwnerSize, ownerWidth)
                         .Value();
      float updatedMainSize = childFlexBasis;

      if (Krys::IsDefined(flexLine.Layout.RemainingFreeSpace) && flexLine.Layout.RemainingFreeSpace < 0)
      {
        flexShrinkScaledFactor = -currentLineChild->ResolveFlexShrink() * childFlexBasis;
        // Is this child able to shrink?
        if (flexShrinkScaledFactor != 0)
        {
          float childSize = std::numeric_limits<float>::quiet_NaN();

          if (Krys::IsDefined(flexLine.Layout.TotalFlexShrinkScaledFactors)
              && flexLine.Layout.TotalFlexShrinkScaledFactors == 0)
          {
            childSize = childFlexBasis + flexShrinkScaledFactor;
          }
          else
          {
            childSize = childFlexBasis
                        + (flexLine.Layout.RemainingFreeSpace / flexLine.Layout.TotalFlexShrinkScaledFactors)
                            * flexShrinkScaledFactor;
          }

          updatedMainSize = BoundAxis(currentLineChild, mainAxis, direction, childSize, availableInnerMainDim,
                                      availableInnerWidth);
        }
      }
      else if (Krys::IsDefined(flexLine.Layout.RemainingFreeSpace) && flexLine.Layout.RemainingFreeSpace > 0)
      {
        flexGrowFactor = currentLineChild->ResolveFlexGrow();

        // Is this child able to grow?
        if (!std::isnan(flexGrowFactor) && flexGrowFactor != 0)
        {
          updatedMainSize = BoundAxis(currentLineChild, mainAxis, direction,
                                      childFlexBasis
                                        + flexLine.Layout.RemainingFreeSpace
                                            / flexLine.Layout.TotalFlexGrowFactors * flexGrowFactor,
                                      availableInnerMainDim, availableInnerWidth);
        }
      }

      deltaFreeSpace += updatedMainSize - childFlexBasis;

      const float marginMain =
        currentLineChild->GetStyle().ComputeMarginForAxis(mainAxis, availableInnerWidth);
      const float marginCross =
        currentLineChild->GetStyle().ComputeMarginForAxis(crossAxis, availableInnerWidth);

      float childCrossSize = std::numeric_limits<float>::quiet_NaN();
      float childMainSize = updatedMainSize + marginMain;
      SizingMode childCrossSizingMode;
      SizingMode childMainSizingMode = SizingMode::StretchFit;

      const auto &childStyle = currentLineChild->GetStyle();
      if (childStyle.GetAspectRatio().HasValue())
      {
        childCrossSize = isMainAxisRow ? (childMainSize - marginMain) / childStyle.GetAspectRatio().Value()
                                       : (childMainSize - marginMain) * childStyle.GetAspectRatio().Value();
        childCrossSizingMode = SizingMode::StretchFit;

        childCrossSize += marginCross;
      }
      else if (!std::isnan(availableInnerCrossDim)
               && !currentLineChild->HasDefiniteLength(ToDimension(crossAxis), availableInnerCrossDim)
               && sizingModeCrossDim == SizingMode::StretchFit && !(isNodeFlexWrap && mainAxisOverflows)
               && ResolveChildAlignment(node, currentLineChild) == Align::Stretch
               && !currentLineChild->GetStyle().IsFlexStartMarginAuto(crossAxis, direction)
               && !currentLineChild->GetStyle().IsFlexEndMarginAuto(crossAxis, direction))
      {
        childCrossSize = availableInnerCrossDim;
        childCrossSizingMode = SizingMode::StretchFit;
      }
      else if (!currentLineChild->HasDefiniteLength(ToDimension(crossAxis), availableInnerCrossDim))
      {
        childCrossSize = availableInnerCrossDim;
        childCrossSizingMode =
          Krys::IsUndefined(childCrossSize) ? SizingMode::MaxContent : SizingMode::FitContent;
      }
      else
      {
        childCrossSize = currentLineChild
                           ->GetResolvedDimension(direction, ToDimension(crossAxis), availableInnerCrossDim,
                                                  availableInnerWidth)
                           .Value()
                         + marginCross;
        const bool isLoosePercentageMeasurement =
          currentLineChild->GetProcessedDimension(ToDimension(crossAxis)).IsPercent()
          && sizingModeCrossDim != SizingMode::StretchFit;
        childCrossSizingMode = Krys::IsUndefined(childCrossSize) || isLoosePercentageMeasurement
                                 ? SizingMode::MaxContent
                                 : SizingMode::StretchFit;
      }

      ConstrainMaxSizeForMode(currentLineChild, direction, mainAxis, availableInnerMainDim,
                              availableInnerWidth, &childMainSizingMode, &childMainSize);
      ConstrainMaxSizeForMode(currentLineChild, direction, crossAxis, availableInnerCrossDim,
                              availableInnerWidth, &childCrossSizingMode, &childCrossSize);

      const bool requiresStretchLayout =
        !currentLineChild->HasDefiniteLength(ToDimension(crossAxis), availableInnerCrossDim)
        && ResolveChildAlignment(node, currentLineChild) == Align::Stretch
        && !currentLineChild->GetStyle().IsFlexStartMarginAuto(crossAxis, direction)
        && !currentLineChild->GetStyle().IsFlexEndMarginAuto(crossAxis, direction);

      const float childWidth = isMainAxisRow ? childMainSize : childCrossSize;
      const float childHeight = !isMainAxisRow ? childMainSize : childCrossSize;

      const SizingMode childWidthSizingMode =
        isMainAxisRow ? childMainSizingMode : childCrossSizingMode;
      const SizingMode childHeightSizingMode =
        !isMainAxisRow ? childMainSizingMode : childCrossSizingMode;

      const bool isLayoutPass = performLayout && !requiresStretchLayout;
      // Recursively call the layout algorithm for this child with the updated
      // main size.
      CalculateLayoutInternal(currentLineChild, childWidth, childHeight, node->GetLayout().GetDirection(),
                              childWidthSizingMode, childHeightSizingMode, availableInnerWidth,
                              availableInnerHeight, isLayoutPass,
                              isLayoutPass ? LayoutPassReason::kFlexLayout : LayoutPassReason::kFlexMeasure,
                              layoutMarkerData, depth, generationCount);
      node->SetLayoutHadOverflow(node->GetLayout().GetHadOverflow()
                                 || currentLineChild->GetLayout().GetHadOverflow());
    }
    return deltaFreeSpace;
  }

  // It distributes the free space to the flexible items.For those flexible items
  // whose min and max constraints are triggered, those flex item's clamped size
  // is removed from the remaingfreespace.
  static void DistributeFreeSpaceFirstPass(FlexLine &flexLine, const Direction direction,
                                           const FlexDirection mainAxis, const float ownerWidth,
                                           const float mainAxisOwnerSize, const float availableInnerMainDim,
                                           const float availableInnerWidth)
  {
    float flexShrinkScaledFactor = 0;
    float flexGrowFactor = 0;
    float baseMainSize = 0;
    float boundMainSize = 0;
    float deltaFreeSpace = 0;

    for (auto currentLineChild : flexLine.ItemsInFlow)
    {
      float childFlexBasis = BoundAxisWithinMinAndMax(currentLineChild, direction, mainAxis,
                                                      currentLineChild->GetLayout().ComputedFlexBasis,
                                                      mainAxisOwnerSize, ownerWidth)
                               .Value();

      if (flexLine.Layout.RemainingFreeSpace < 0)
      {
        flexShrinkScaledFactor = -currentLineChild->ResolveFlexShrink() * childFlexBasis;

        // Is this child able to shrink?
        if (Krys::IsDefined(flexShrinkScaledFactor) && flexShrinkScaledFactor != 0)
        {
          baseMainSize = childFlexBasis
                         + flexLine.Layout.RemainingFreeSpace / flexLine.Layout.TotalFlexShrinkScaledFactors
                             * flexShrinkScaledFactor;
          boundMainSize = BoundAxis(currentLineChild, mainAxis, direction, baseMainSize,
                                    availableInnerMainDim, availableInnerWidth);
          if (Krys::IsDefined(baseMainSize) && Krys::IsDefined(boundMainSize)
              && baseMainSize != boundMainSize)
          {
            // By excluding this item's size and flex factor from remaining, this
            // item's min/max constraints should also trigger in the second pass
            // resulting in the item's size calculation being identical in the
            // first and second passes.
            deltaFreeSpace += boundMainSize - childFlexBasis;
            flexLine.Layout.TotalFlexShrinkScaledFactors -=
              (-currentLineChild->ResolveFlexShrink()
               * currentLineChild->GetLayout().ComputedFlexBasis.Value());
          }
        }
      }
      else if (Krys::IsDefined(flexLine.Layout.RemainingFreeSpace) && flexLine.Layout.RemainingFreeSpace > 0)
      {
        flexGrowFactor = currentLineChild->ResolveFlexGrow();

        // Is this child able to grow?
        if (Krys::IsDefined(flexGrowFactor) && flexGrowFactor != 0)
        {
          baseMainSize =
            childFlexBasis
            + flexLine.Layout.RemainingFreeSpace / flexLine.Layout.TotalFlexGrowFactors * flexGrowFactor;
          boundMainSize = BoundAxis(currentLineChild, mainAxis, direction, baseMainSize,
                                    availableInnerMainDim, availableInnerWidth);

          if (Krys::IsDefined(baseMainSize) && Krys::IsDefined(boundMainSize)
              && baseMainSize != boundMainSize)
          {
            // By excluding this item's size and flex factor from remaining, this
            // item's min/max constraints should also trigger in the second pass
            // resulting in the item's size calculation being identical in the
            // first and second passes.
            deltaFreeSpace += boundMainSize - childFlexBasis;
            flexLine.Layout.TotalFlexGrowFactors -= flexGrowFactor;
          }
        }
      }
    }
    flexLine.Layout.RemainingFreeSpace -= deltaFreeSpace;
  }

  // Do two passes over the flex items to figure out how to distribute the
  // remaining space.
  //
  // The first pass finds the items whose min/max constraints trigger, freezes
  // them at those sizes, and excludes those sizes from the remaining space.
  //
  // The second pass sets the size of each flexible item. It distributes the
  // remaining space amongst the items whose min/max constraints didn't trigger in
  // the first pass. For the other items, it sets their sizes by forcing their
  // min/max constraints to trigger again.
  //
  // This two pass approach for resolving min/max constraints deviates from the
  // spec. The spec
  // (https://www.w3.org/TR/CSS-flexbox-1/#resolve-flexible-lengths) describes a
  // process that needs to be repeated a variable number of times. The algorithm
  // implemented here won't handle all cases but it was simpler to implement and
  // it mitigates performance concerns because we know exactly how many passes
  // it'll do.
  //
  // At the end of this function the child nodes would have the proper size
  // assigned to them.
  //
  static void ResolveFlexibleLength(Node *const node, FlexLine &flexLine, const FlexDirection mainAxis,
                                    const FlexDirection crossAxis, const Direction direction,
                                    const float ownerWidth, const float mainAxisOwnerSize,
                                    const float availableInnerMainDim, const float availableInnerCrossDim,
                                    const float availableInnerWidth, const float availableInnerHeight,
                                    const bool mainAxisOverflows, const SizingMode sizingModeCrossDim,
                                    const bool performLayout, LayoutData &layoutMarkerData,
                                    const uint32 depth, const uint16 generationCount)
  {
    const float originalFreeSpace = flexLine.Layout.RemainingFreeSpace;
    // First pass: detect the flex items whose min/max constraints trigger
    DistributeFreeSpaceFirstPass(flexLine, direction, mainAxis, ownerWidth, mainAxisOwnerSize,
                                 availableInnerMainDim, availableInnerWidth);

    // Second pass: resolve the sizes of the flexible items
    const float distributedFreeSpace = DistributeFreeSpaceSecondPass(
      flexLine, node, mainAxis, crossAxis, direction, ownerWidth, mainAxisOwnerSize, availableInnerMainDim,
      availableInnerCrossDim, availableInnerWidth, availableInnerHeight, mainAxisOverflows,
      sizingModeCrossDim, performLayout, layoutMarkerData, depth, generationCount);

    flexLine.Layout.RemainingFreeSpace = originalFreeSpace - distributedFreeSpace;
  }

  static void JustifyMainAxis(Node *const node, FlexLine &flexLine, const FlexDirection mainAxis,
                              const FlexDirection crossAxis, const Direction direction,
                              const SizingMode sizingModeMainDim,
                              const SizingMode sizingModeCrossDim, const float mainAxisOwnerSize,
                              const float ownerWidth, const float availableInnerMainDim,
                              const float availableInnerCrossDim, const float availableInnerWidth,
                              const bool performLayout)
  {
    const auto &GetStyle = node->GetStyle();

    const float leadingPaddingAndBorderMain =
      node->GetStyle().ComputeFlexStartPaddingAndBorder(mainAxis, direction, ownerWidth);
    const float trailingPaddingAndBorderMain =
      node->GetStyle().ComputeFlexEndPaddingAndBorder(mainAxis, direction, ownerWidth);

    const float gap = node->GetStyle().ComputeGapForAxis(mainAxis, availableInnerMainDim);
    // If we are using "at most" rules in the main axis, make sure that
    // RemainingFreeSpace is 0 when min main dimension is not given
    if (sizingModeMainDim == SizingMode::FitContent && flexLine.Layout.RemainingFreeSpace > 0)
    {
      if (GetStyle.GetMinDimension(ToDimension(mainAxis)).IsDefined()
          && GetStyle.ResolvedMinDimension(direction, ToDimension(mainAxis), mainAxisOwnerSize, ownerWidth)
               .HasValue())
      {
        // This condition makes sure that if the size of main dimension(after
        // considering child nodes main dim, leading and trailing GetPadding etc)
        // falls below min dimension, then the RemainingFreeSpace is reassigned
        // considering the min dimension

        // `minAvailableMainDim` denotes minimum available space in which child
        // can be laid out, it will exclude space consumed by GetPadding and GetBorder.
        const float minAvailableMainDim =
          GetStyle.ResolvedMinDimension(direction, ToDimension(mainAxis), mainAxisOwnerSize, ownerWidth)
            .Value()
          - leadingPaddingAndBorderMain - trailingPaddingAndBorderMain;
        const float occupiedSpaceByChildNodes = availableInnerMainDim - flexLine.Layout.RemainingFreeSpace;
        flexLine.Layout.RemainingFreeSpace =
          Krys::MaxOrDefined(0.0f, minAvailableMainDim - occupiedSpaceByChildNodes);
      }
      else
      {
        flexLine.Layout.RemainingFreeSpace = 0;
      }
    }

    // In order to position the elements in the main axis, we have two controls.
    // The space between the beginning and the first element and the space between
    // each two elements.
    float leadingMainDim = 0;
    float betweenMainDim = gap;
    const Justify justifyContent = flexLine.Layout.RemainingFreeSpace >= 0
                                     ? node->GetStyle().GetJustifyContent()
                                     : FallbackAlignment(node->GetStyle().GetJustifyContent());

    if (flexLine.NumberOfAutoMargins == 0)
    {
      switch (justifyContent)
      {
        case Justify::Center:  leadingMainDim = flexLine.Layout.RemainingFreeSpace / 2; break;
        case Justify::FlexEnd: leadingMainDim = flexLine.Layout.RemainingFreeSpace; break;
        case Justify::SpaceBetween:
          if (flexLine.ItemsInFlow.size() > 1)
          {
            betweenMainDim +=
              flexLine.Layout.RemainingFreeSpace / static_cast<float>(flexLine.ItemsInFlow.size() - 1);
          }
          break;
        case Justify::SpaceEvenly:
          // Space is distributed evenly across all elements
          leadingMainDim =
            flexLine.Layout.RemainingFreeSpace / static_cast<float>(flexLine.ItemsInFlow.size() + 1);
          betweenMainDim += leadingMainDim;
          break;
        case Justify::SpaceAround:
          // Space on the edges is half of the space between elements
          leadingMainDim =
            0.5f * flexLine.Layout.RemainingFreeSpace / static_cast<float>(flexLine.ItemsInFlow.size());
          betweenMainDim += leadingMainDim * 2;
          break;
        case Justify::FlexStart: break;
      }
    }

    flexLine.Layout.MainDim = leadingPaddingAndBorderMain + leadingMainDim;
    flexLine.Layout.CrossDim = 0;

    float maxAscentForCurrentLine = 0;
    float maxDescentForCurrentLine = 0;
    bool isNodeBaselineLayout = IsBaselineLayout(node);
    for (auto child : flexLine.ItemsInFlow)
    {
      const LayoutResults &childLayout = child->GetLayout();
      if (child->GetStyle().IsFlexStartMarginAuto(mainAxis, direction)
          && flexLine.Layout.RemainingFreeSpace > 0.0f)
      {
        flexLine.Layout.MainDim +=
          flexLine.Layout.RemainingFreeSpace / static_cast<float>(flexLine.NumberOfAutoMargins);
      }

      if (performLayout)
      {
        child->SetLayoutPosition(childLayout.GetPosition(FlexStartEdge(mainAxis)) + flexLine.Layout.MainDim,
                                 FlexStartEdge(mainAxis));
      }

      if (child != flexLine.ItemsInFlow.back())
      {
        flexLine.Layout.MainDim += betweenMainDim;
      }

      if (child->GetStyle().IsFlexEndMarginAuto(mainAxis, direction)
          && flexLine.Layout.RemainingFreeSpace > 0.0f)
      {
        flexLine.Layout.MainDim +=
          flexLine.Layout.RemainingFreeSpace / static_cast<float>(flexLine.NumberOfAutoMargins);
      }
      bool canSkipFlex = !performLayout && sizingModeCrossDim == SizingMode::StretchFit;
      if (canSkipFlex)
      {
        // If we skipped the flex step, then we can't rely on the measuredDims
        // because they weren't computed. This means we can't call
        // dimensionWithMargin.
        flexLine.Layout.MainDim += child->GetStyle().ComputeMarginForAxis(mainAxis, availableInnerWidth)
                                   + childLayout.ComputedFlexBasis.Value();
        flexLine.Layout.CrossDim = availableInnerCrossDim;
      }
      else
      {
        // The main dimension is the sum of all the elements dimension plus
        // the spacing.
        flexLine.Layout.MainDim += child->DimensionWithMargin(mainAxis, availableInnerWidth);

        if (isNodeBaselineLayout)
        {
          // If the child is baseline aligned then the cross dimension is
          // calculated by adding maxAscent and maxDescent from the baseline.
          const float ascent =
            CalculateBaseline(child)
            + child->GetStyle().ComputeFlexStartMargin(FlexDirection::Column, direction, availableInnerWidth);
          const float descent =
            child->GetLayout().GetMeasuredDimension(Dimension::Height)
            + child->GetStyle().ComputeMarginForAxis(FlexDirection::Column, availableInnerWidth) - ascent;

          maxAscentForCurrentLine = Krys::MaxOrDefined(maxAscentForCurrentLine, ascent);
          maxDescentForCurrentLine = Krys::MaxOrDefined(maxDescentForCurrentLine, descent);
        }
        else
        {
          // The cross dimension is the max of the elements dimension since
          // there can only be one element in that cross dimension in the case
          // when the items are not baseline aligned
          flexLine.Layout.CrossDim = Krys::MaxOrDefined(
            flexLine.Layout.CrossDim, child->DimensionWithMargin(crossAxis, availableInnerWidth));
        }
      }
    }
    flexLine.Layout.MainDim += trailingPaddingAndBorderMain;

    if (isNodeBaselineLayout)
    {
      flexLine.Layout.CrossDim = maxAscentForCurrentLine + maxDescentForCurrentLine;
    }
  }

  //
  // This is the main routine that implements a subset of the flexbox layout
  // algorithm described in the W3C CSS documentation:
  // https://www.w3.org/TR/CSS3-flexbox/.
  //
  // Limitations of this algorithm, compared to the full standard:
  //  * Display property is always assumed to be 'flex' except for Text nodes,
  //    which are assumed to be 'inline-flex'.
  //  * The 'zIndex' property (or any form of z ordering) is not supported. Nodes
  //    are stacked in document order.
  //  * The 'order' property is not supported. The order of flex items is always
  //    defined by document order.
  //  * The 'visibility' property is always assumed to be 'visible'. Values of
  //    'collapse' and 'hidden' are not supported.
  //  * There is no support for forced breaks.
  //  * It does not support vertical inline directions (top-to-bottom or
  //    bottom-to-top text).
  //
  // Deviations from standard:
  //  * Section 4.5 of the spec indicates that all flex items have a default
  //    minimum main size. For text blocks, for example, this is the width of the
  //    widest word. Calculating the minimum width is expensive, so we forego it
  //    and assume a default minimum main size of 0.
  //  * Min/Max sizes in the main axis are not honored when resolving flexible
  //    lengths.
  //  * The spec indicates that the default value for 'flexDirection' is 'row',
  //    but the algorithm below assumes a default of 'column'.
  //
  // Input parameters:
  //    - node: current node to be sized and laid out
  //    - availableWidth & availableHeight: available size to be used for sizing
  //      the node or YGUndefined if the size is not available; interpretation
  //      depends on layout flags
  //    - ownerDirection: the inline (text) direction within the owner
  //      (left-to-right or right-to-left)
  //    - widthSizingMode: indicates the sizing rules for the width (see below
  //      for explanation)
  //    - heightSizingMode: indicates the sizing rules for the height (see below
  //      for explanation)
  //    - performLayout: specifies whether the caller is interested in just the
  //      dimensions of the node or it requires the entire node and its subtree to
  //      be laid out (with final positions)
  //
  // Details:
  //    This routine is called recursively to lay out subtrees of flexbox
  //    elements. It uses the information in node.GetStyle, which is treated as a
  //    read-only input. It is responsible for setting the layout.direction and
  //    layout.GetMeasuredDimensions fields for the input node as well as the
  //    layout.position and layout.lineIndex fields for its child nodes. The
  //    layout.GetMeasuredDimensions field includes any GetBorder or GetPadding for the
  //    node but does not include margins.
  //
  //    When calling calculateLayoutImpl and calculateLayoutInternal, if the
  //    caller passes an available size of undefined then it must also pass a
  //    measure mode of SizingMode::MaxContent in that dimension.
  //
  static void CalculateLayoutImpl(Node *const node, const float availableWidth, const float availableHeight,
                                  const Direction ownerDirection, const SizingMode widthSizingMode,
                                  const SizingMode heightSizingMode, const float ownerWidth,
                                  const float ownerHeight, const bool performLayout,
                                  const LayoutPassReason reason, LayoutData &layoutMarkerData,
                                  const uint32 depth, const uint16 generationCount)
  {
    if (Krys::IsUndefined(availableWidth) && widthSizingMode != SizingMode::MaxContent)
    {
      throw std::invalid_argument(
        "availableWidth is indefinite so widthSizingMode must be SizingMode::MaxContent");
    }

    if (Krys::IsUndefined(availableHeight) && heightSizingMode != SizingMode::MaxContent)
    {
      throw std::invalid_argument(
        "availableHeight is indefinite so heightSizingMode must be SizingMode::MaxContent");
    }

    (performLayout ? layoutMarkerData.Layouts : layoutMarkerData.Measures) += 1;

    // Set the resolved resolution in the node's layout.
    const Direction direction = node->ResolveDirection(ownerDirection);
    node->SetLayoutDirection(direction);

    const FlexDirection flexRowDirection = ResolveDirection(FlexDirection::Row, direction);
    const FlexDirection flexColumnDirection = ResolveDirection(FlexDirection::Column, direction);

    const auto startEdge = direction == Direction::LTR ? PhysicalEdge::Left : PhysicalEdge::Right;
    const auto endEdge = direction == Direction::LTR ? PhysicalEdge::Right : PhysicalEdge::Left;

    const float marginRowLeading =
      node->GetStyle().ComputeInlineStartMargin(flexRowDirection, direction, ownerWidth);
    node->SetLayoutMargin(marginRowLeading, startEdge);
    const float marginRowTrailing =
      node->GetStyle().ComputeInlineEndMargin(flexRowDirection, direction, ownerWidth);
    node->SetLayoutMargin(marginRowTrailing, endEdge);
    const float marginColumnLeading =
      node->GetStyle().ComputeInlineStartMargin(flexColumnDirection, direction, ownerWidth);
    node->SetLayoutMargin(marginColumnLeading, PhysicalEdge::Top);
    const float marginColumnTrailing =
      node->GetStyle().ComputeInlineEndMargin(flexColumnDirection, direction, ownerWidth);
    node->SetLayoutMargin(marginColumnTrailing, PhysicalEdge::Bottom);

    const float marginAxisRow = marginRowLeading + marginRowTrailing;
    const float marginAxisColumn = marginColumnLeading + marginColumnTrailing;

    node->SetLayoutBorder(node->GetStyle().ComputeInlineStartBorder(flexRowDirection, direction), startEdge);
    node->SetLayoutBorder(node->GetStyle().ComputeInlineEndBorder(flexRowDirection, direction), endEdge);
    node->SetLayoutBorder(node->GetStyle().ComputeInlineStartBorder(flexColumnDirection, direction),
                          PhysicalEdge::Top);
    node->SetLayoutBorder(node->GetStyle().ComputeInlineEndBorder(flexColumnDirection, direction),
                          PhysicalEdge::Bottom);

    node->SetLayoutPadding(
      node->GetStyle().ComputeInlineStartPadding(flexRowDirection, direction, ownerWidth), startEdge);
    node->SetLayoutPadding(node->GetStyle().ComputeInlineEndPadding(flexRowDirection, direction, ownerWidth),
                           endEdge);
    node->SetLayoutPadding(
      node->GetStyle().ComputeInlineStartPadding(flexColumnDirection, direction, ownerWidth),
      PhysicalEdge::Top);
    node->SetLayoutPadding(
      node->GetStyle().ComputeInlineEndPadding(flexColumnDirection, direction, ownerWidth),
      PhysicalEdge::Bottom);

    if (node->HasMeasureFunc())
    {
      MeasureNodeWithMeasureFunc(node, direction, availableWidth - marginAxisRow,
                                 availableHeight - marginAxisColumn, widthSizingMode, heightSizingMode,
                                 ownerWidth, ownerHeight, layoutMarkerData, reason);

      // Clean and update all display: contents nodes with a direct path to the
      // current node as they will not be traversed
      CleanupContentsNodesRecursively(node);
      return;
    }

    const auto childCount = node->GetLayoutChildCount();
    if (childCount == 0)
    {
      MeasureNodeWithoutChildren(node, direction, availableWidth - marginAxisRow,
                                 availableHeight - marginAxisColumn, widthSizingMode, heightSizingMode,
                                 ownerWidth, ownerHeight);

      // Clean and update all display: contents nodes with a direct path to the
      // current node as they will not be traversed
      CleanupContentsNodesRecursively(node);
      return;
    }

    // If we're not being asked to perform a full layout we can skip the algorithm
    // if we already know the size
    if (!performLayout
        && MeasureNodeWithFixedSize(node, direction, availableWidth - marginAxisRow,
                                    availableHeight - marginAxisColumn, widthSizingMode, heightSizingMode,
                                    ownerWidth, ownerHeight))
    {
      // Clean and update all display: contents nodes with a direct path to the
      // current node as they will not be traversed
      CleanupContentsNodesRecursively(node);
      return;
    }

    // At this point we know we're going to perform work. Ensure that each child
    // has a mutable copy.
    node->CloneChildrenIfNeeded();
    // Reset layout flags, as they could have changed.
    node->SetLayoutHadOverflow(false);

    // Clean and update all display: contents nodes with a direct path to the
    // current node as they will not be traversed
    CleanupContentsNodesRecursively(node);

    // STEP 1: CALCULATE VALUES FOR REMAINDER OF ALGORITHM
    const FlexDirection mainAxis = ResolveDirection(node->GetStyle().GetFlexDirection(), direction);
    const FlexDirection crossAxis = ResolveCrossDirection(mainAxis, direction);
    const bool isMainAxisRow = IsRow(mainAxis);
    const bool isNodeFlexWrap = node->GetStyle().GetFlexWrap() != Wrap::NoWrap;

    const float mainAxisOwnerSize = isMainAxisRow ? ownerWidth : ownerHeight;
    const float crossAxisOwnerSize = isMainAxisRow ? ownerHeight : ownerWidth;

    const float GetPaddingAndBorderAxisMain = PaddingAndBorderForAxis(node, mainAxis, direction, ownerWidth);
    const float GetPaddingAndBorderAxisCross =
      PaddingAndBorderForAxis(node, crossAxis, direction, ownerWidth);
    const float leadingPaddingAndBorderCross =
      node->GetStyle().ComputeFlexStartPaddingAndBorder(crossAxis, direction, ownerWidth);

    SizingMode sizingModeMainDim = isMainAxisRow ? widthSizingMode : heightSizingMode;
    SizingMode sizingModeCrossDim = isMainAxisRow ? heightSizingMode : widthSizingMode;

    const float GetPaddingAndBorderAxisRow =
      isMainAxisRow ? GetPaddingAndBorderAxisMain : GetPaddingAndBorderAxisCross;
    const float GetPaddingAndBorderAxisColumn =
      isMainAxisRow ? GetPaddingAndBorderAxisCross : GetPaddingAndBorderAxisMain;

    // STEP 2: DETERMINE AVAILABLE SIZE IN MAIN AND CROSS DIRECTIONS

    float availableInnerWidth =
      CalculateAvailableInnerDimension(node, direction, Dimension::Width, availableWidth - marginAxisRow,
                                       GetPaddingAndBorderAxisRow, ownerWidth, ownerWidth);
    float availableInnerHeight =
      CalculateAvailableInnerDimension(node, direction, Dimension::Height, availableHeight - marginAxisColumn,
                                       GetPaddingAndBorderAxisColumn, ownerHeight, ownerWidth);

    float availableInnerMainDim = isMainAxisRow ? availableInnerWidth : availableInnerHeight;
    const float availableInnerCrossDim = isMainAxisRow ? availableInnerHeight : availableInnerWidth;

    // STEP 3: DETERMINE FLEX BASIS FOR EACH ITEM

    // Computed basis + margins + gap
    float totalMainDim = 0;
    totalMainDim += ComputeFlexBasisForChildren(node, availableInnerWidth, availableInnerHeight,
                                                widthSizingMode, heightSizingMode, direction, mainAxis,
                                                performLayout, layoutMarkerData, depth, generationCount);

    if (childCount > 1)
    {
      totalMainDim += node->GetStyle().ComputeGapForAxis(mainAxis, availableInnerMainDim)
                      * static_cast<float>(childCount - 1);
    }

    const bool mainAxisOverflows =
      (sizingModeMainDim != SizingMode::MaxContent) && totalMainDim > availableInnerMainDim;

    if (isNodeFlexWrap && mainAxisOverflows && sizingModeMainDim == SizingMode::FitContent)
    {
      sizingModeMainDim = SizingMode::StretchFit;
    }
    // STEP 4: COLLECT FLEX ITEMS INTO FLEX LINES

    // Iterator representing the beginning of the current line
    Node::LayoutableChildren::Iterator startOfLineIterator = node->GetLayoutChildren().begin();

    // Number of lines.
    uint16 lineCount = 0;

    // Accumulated cross dimensions of all lines so far.
    float totalLineCrossDim = 0;

    const float crossAxisGap = node->GetStyle().ComputeGapForAxis(crossAxis, availableInnerCrossDim);

    // Max main dimension of all the lines.
    float maxLineMainDim = 0;
    for (; startOfLineIterator != node->GetLayoutChildren().end(); lineCount++)
    {
      auto flexLine =
        CalculateFlexLine(node, ownerDirection, ownerWidth, mainAxisOwnerSize, availableInnerWidth,
                          availableInnerMainDim, startOfLineIterator, lineCount);

      // If we don't need to measure the cross axis, we can skip the entire flex
      // step.
      const bool canSkipFlex = !performLayout && sizingModeCrossDim == SizingMode::StretchFit;

      // STEP 5: RESOLVING FLEXIBLE LENGTHS ON MAIN AXIS
      // Calculate the remaining available space that needs to be allocated. If
      // the main dimension size isn't known, it is computed based on the line
      // length, so there's no more space left to distribute.

      bool sizeBasedOnContent = false;
      // If we don't measure with exact main dimension we want to ensure we don't
      // violate min and max
      if (sizingModeMainDim != SizingMode::StretchFit)
      {
        const auto &GetStyle = node->GetStyle();
        const float minInnerWidth =
          GetStyle.ResolvedMinDimension(direction, Dimension::Width, ownerWidth, ownerWidth).Value()
          - GetPaddingAndBorderAxisRow;
        const float maxInnerWidth =
          GetStyle.ResolvedMaxDimension(direction, Dimension::Width, ownerWidth, ownerWidth).Value()
          - GetPaddingAndBorderAxisRow;
        const float minInnerHeight =
          GetStyle.ResolvedMinDimension(direction, Dimension::Height, ownerHeight, ownerWidth).Value()
          - GetPaddingAndBorderAxisColumn;
        const float maxInnerHeight =
          GetStyle.ResolvedMaxDimension(direction, Dimension::Height, ownerHeight, ownerWidth).Value()
          - GetPaddingAndBorderAxisColumn;

        const float minInnerMainDim = isMainAxisRow ? minInnerWidth : minInnerHeight;
        const float maxInnerMainDim = isMainAxisRow ? maxInnerWidth : maxInnerHeight;

        if (Krys::IsDefined(minInnerMainDim) && flexLine.SizeConsumed < minInnerMainDim)
        {
          availableInnerMainDim = minInnerMainDim;
        }
        else if (Krys::IsDefined(maxInnerMainDim) && flexLine.SizeConsumed > maxInnerMainDim)
        {
          availableInnerMainDim = maxInnerMainDim;
        }
        else
        {
          bool useLegacyStretchBehaviour = node->HasErrata(Errata::StretchFlexBasis);

          if (!useLegacyStretchBehaviour
              && ((Krys::IsDefined(flexLine.Layout.TotalFlexGrowFactors)
                   && flexLine.Layout.TotalFlexGrowFactors == 0)
                  || (Krys::IsDefined(node->ResolveFlexGrow()) && node->ResolveFlexGrow() == 0)))
          {
            // If we don't have any children to flex or we can't flex the node
            // itself, space we've used is all space we need. Root node also
            // should be shrunk to minimum
            availableInnerMainDim = flexLine.SizeConsumed;
          }

          sizeBasedOnContent = !useLegacyStretchBehaviour;
        }
      }

      if (!sizeBasedOnContent && Krys::IsDefined(availableInnerMainDim))
      {
        flexLine.Layout.RemainingFreeSpace = availableInnerMainDim - flexLine.SizeConsumed;
      }
      else if (flexLine.SizeConsumed < 0)
      {
        // availableInnerMainDim is indefinite which means the node is being sized
        // based on its content. sizeConsumed is negative which means
        // the node will allocate 0 points for its content. Consequently,
        // RemainingFreeSpace is 0 - sizeConsumed.
        flexLine.Layout.RemainingFreeSpace = -flexLine.SizeConsumed;
      }

      if (!canSkipFlex)
      {
        ResolveFlexibleLength(node, flexLine, mainAxis, crossAxis, direction, ownerWidth, mainAxisOwnerSize,
                              availableInnerMainDim, availableInnerCrossDim, availableInnerWidth,
                              availableInnerHeight, mainAxisOverflows, sizingModeCrossDim, performLayout,
                              layoutMarkerData, depth, generationCount);
      }

      node->SetLayoutHadOverflow(node->GetLayout().GetHadOverflow()
                                 || (flexLine.Layout.RemainingFreeSpace < 0));

      // STEP 6: MAIN-AXIS JUSTIFICATION & CROSS-AXIS SIZE DETERMINATION

      // At this point, all the children have their dimensions set in the main
      // axis. Their dimensions are also set in the cross axis with the exception
      // of items that are aligned "stretch". We need to compute these stretch
      // values and set the final positions.

      JustifyMainAxis(node, flexLine, mainAxis, crossAxis, direction, sizingModeMainDim, sizingModeCrossDim,
                      mainAxisOwnerSize, ownerWidth, availableInnerMainDim, availableInnerCrossDim,
                      availableInnerWidth, performLayout);

      float containerCrossAxis = availableInnerCrossDim;
      if (sizingModeCrossDim == SizingMode::MaxContent || sizingModeCrossDim == SizingMode::FitContent)
      {
        // Compute the cross axis from the max cross dimension of the children.
        containerCrossAxis =
          BoundAxis(node, crossAxis, direction, flexLine.Layout.CrossDim + GetPaddingAndBorderAxisCross,
                    crossAxisOwnerSize, ownerWidth)
          - GetPaddingAndBorderAxisCross;
      }

      // If there's no flex wrap, the cross dimension is defined by the container.
      if (!isNodeFlexWrap && sizingModeCrossDim == SizingMode::StretchFit)
      {
        flexLine.Layout.CrossDim = availableInnerCrossDim;
      }

      // As-per https://www.w3.org/TR/css-flexbox-1/#cross-sizing, the
      // cross-size of the line within a single-line container should be bound to
      // min/max constraints before alignment within the line. In a multi-line
      // container, affecting alignment between the lines.
      if (!isNodeFlexWrap)
      {
        flexLine.Layout.CrossDim =
          BoundAxis(node, crossAxis, direction, flexLine.Layout.CrossDim + GetPaddingAndBorderAxisCross,
                    crossAxisOwnerSize, ownerWidth)
          - GetPaddingAndBorderAxisCross;
      }

      // STEP 7: CROSS-AXIS ALIGNMENT
      // We can skip child alignment if we're just measuring the container.
      if (performLayout)
      {
        for (auto child : flexLine.ItemsInFlow)
        {
          float leadingCrossDim = leadingPaddingAndBorderCross;

          // For a relative children, we're either using alignItems (owner) or
          // alignSelf (child) in order to determine the position in the cross
          // axis
          const Align alignItem = ResolveChildAlignment(node, child);

          // If the child uses align stretch, we need to lay it out one more
          // time, this time forcing the cross-axis size to be the computed
          // cross size for the current line.
          if (alignItem == Align::Stretch && !child->GetStyle().IsFlexStartMarginAuto(crossAxis, direction)
              && !child->GetStyle().IsFlexEndMarginAuto(crossAxis, direction))
          {
            // If the child defines a definite size for its cross axis, there's
            // no need to stretch.
            if (!child->HasDefiniteLength(ToDimension(crossAxis), availableInnerCrossDim))
            {
              float childMainSize = child->GetLayout().GetMeasuredDimension(ToDimension(mainAxis));
              const auto &childStyle = child->GetStyle();
              float childCrossSize =
                childStyle.GetAspectRatio().HasValue()
                  ? child->GetStyle().ComputeMarginForAxis(crossAxis, availableInnerWidth)
                      + (isMainAxisRow ? childMainSize / childStyle.GetAspectRatio().Value()
                                       : childMainSize * childStyle.GetAspectRatio().Value())
                  : flexLine.Layout.CrossDim;

              childMainSize += child->GetStyle().ComputeMarginForAxis(mainAxis, availableInnerWidth);

              SizingMode childMainSizingMode = SizingMode::StretchFit;
              SizingMode childCrossSizingMode = SizingMode::StretchFit;
              ConstrainMaxSizeForMode(child, direction, mainAxis, availableInnerMainDim, availableInnerWidth,
                                      &childMainSizingMode, &childMainSize);
              ConstrainMaxSizeForMode(child, direction, crossAxis, availableInnerCrossDim,
                                      availableInnerWidth, &childCrossSizingMode, &childCrossSize);

              const float childWidth = isMainAxisRow ? childMainSize : childCrossSize;
              const float childHeight = !isMainAxisRow ? childMainSize : childCrossSize;

              auto alignContent = node->GetStyle().GetAlignContent();
              auto crossAxisDoesNotGrow = alignContent != Align::Stretch && isNodeFlexWrap;
              const SizingMode childWidthSizingMode =
                Krys::IsUndefined(childWidth) || (!isMainAxisRow && crossAxisDoesNotGrow)
                  ? SizingMode::MaxContent
                  : SizingMode::StretchFit;
              const SizingMode childHeightSizingMode =
                Krys::IsUndefined(childHeight) || (isMainAxisRow && crossAxisDoesNotGrow)
                  ? SizingMode::MaxContent
                  : SizingMode::StretchFit;

              CalculateLayoutInternal(child, childWidth, childHeight, direction, childWidthSizingMode,
                                      childHeightSizingMode, availableInnerWidth, availableInnerHeight, true,
                                      LayoutPassReason::kStretch, layoutMarkerData, depth, generationCount);
            }
          }
          else
          {
            const float remainingCrossDim =
              containerCrossAxis - child->DimensionWithMargin(crossAxis, availableInnerWidth);

            if (child->GetStyle().IsFlexStartMarginAuto(crossAxis, direction)
                && child->GetStyle().IsFlexEndMarginAuto(crossAxis, direction))
            {
              leadingCrossDim += Krys::MaxOrDefined(0.0f, remainingCrossDim / 2);
            }
            else if (child->GetStyle().IsFlexEndMarginAuto(crossAxis, direction))
            {
              // No-Op
            }
            else if (child->GetStyle().IsFlexStartMarginAuto(crossAxis, direction))
            {
              leadingCrossDim += Krys::MaxOrDefined(0.0f, remainingCrossDim);
            }
            else if (alignItem == Align::FlexStart)
            {
              // No-Op
            }
            else if (alignItem == Align::Center)
            {
              leadingCrossDim += remainingCrossDim / 2;
            }
            else
            {
              leadingCrossDim += remainingCrossDim;
            }
          }
          // And we apply the position
          child->SetLayoutPosition(child->GetLayout().GetPosition(FlexStartEdge(crossAxis))
                                     + totalLineCrossDim + leadingCrossDim,
                                   FlexStartEdge(crossAxis));
        }
      }

      const float appliedCrossGap = lineCount != 0 ? crossAxisGap : 0.0f;
      totalLineCrossDim += flexLine.Layout.CrossDim + appliedCrossGap;
      maxLineMainDim = Krys::MaxOrDefined(maxLineMainDim, flexLine.Layout.MainDim);
    }

    // STEP 8: MULTI-LINE CONTENT ALIGNMENT
    // currentLead stores the size of the cross dim
    if (performLayout && (isNodeFlexWrap || IsBaselineLayout(node)))
    {
      float leadPerLine = 0;
      float currentLead = leadingPaddingAndBorderCross;
      float extraSpacePerLine = 0;

      const float unclampedCrossDim =
        sizingModeCrossDim == SizingMode::StretchFit ? availableInnerCrossDim + GetPaddingAndBorderAxisCross
        : node->HasDefiniteLength(ToDimension(crossAxis), crossAxisOwnerSize)
          ? node->GetResolvedDimension(direction, ToDimension(crossAxis), crossAxisOwnerSize, ownerWidth)
              .Value()
          : totalLineCrossDim + GetPaddingAndBorderAxisCross;

      const float innerCrossDim =
        BoundAxis(node, crossAxis, direction, unclampedCrossDim, crossAxisOwnerSize, ownerWidth)
        - GetPaddingAndBorderAxisCross;

      const float remainingAlignContentDim = innerCrossDim - totalLineCrossDim;

      const auto alignContent = remainingAlignContentDim >= 0
                                  ? node->GetStyle().GetAlignContent()
                                  : FallbackAlignment(node->GetStyle().GetAlignContent());

      switch (alignContent)
      {
        case Align::FlexEnd: currentLead += remainingAlignContentDim; break;
        case Align::Center:  currentLead += remainingAlignContentDim / 2; break;
        case Align::Stretch:
          extraSpacePerLine = remainingAlignContentDim / static_cast<float>(lineCount);
          break;
        case Align::SpaceAround:
          currentLead += remainingAlignContentDim / (2 * static_cast<float>(lineCount));
          leadPerLine = remainingAlignContentDim / static_cast<float>(lineCount);
          break;
        case Align::SpaceEvenly:
          currentLead += remainingAlignContentDim / static_cast<float>(lineCount + 1);
          leadPerLine = remainingAlignContentDim / static_cast<float>(lineCount + 1);
          break;
        case Align::SpaceBetween:
          if (lineCount > 1)
          {
            leadPerLine = remainingAlignContentDim / static_cast<float>(lineCount - 1);
          }
          break;
        case Align::Auto:
        case Align::FlexStart:
        case Align::Baseline:  break;
      }
      Node::LayoutableChildren::Iterator endIterator = node->GetLayoutChildren().begin();
      for (size_t i = 0; i < static_cast<size_t>(lineCount); i++)
      {
        const Node::LayoutableChildren::Iterator startIterator = endIterator;
        auto iterator = startIterator;

        // compute the line's height and find the endIndex
        float lineHeight = 0;
        float maxAscentForCurrentLine = 0;
        float maxDescentForCurrentLine = 0;
        for (; iterator != node->GetLayoutChildren().end(); iterator++)
        {
          const auto child = *iterator;
          if (child->GetStyle().GetDisplay() == Display::None)
          {
            continue;
          }
          if (child->GetStyle().GetPositionType() != PositionType::Absolute)
          {
            if (child->GetLineIndex() != i)
            {
              break;
            }
            if (child->IsLayoutDimensionDefined(crossAxis))
            {
              lineHeight = Krys::MaxOrDefined(
                lineHeight, child->GetLayout().GetMeasuredDimension(ToDimension(crossAxis))
                              + child->GetStyle().ComputeMarginForAxis(crossAxis, availableInnerWidth));
            }
            if (ResolveChildAlignment(node, child) == Align::Baseline)
            {
              const float ascent = CalculateBaseline(child)
                                   + child->GetStyle().ComputeFlexStartMargin(FlexDirection::Column,
                                                                              direction, availableInnerWidth);
              const float descent =
                child->GetLayout().GetMeasuredDimension(Dimension::Height)
                + child->GetStyle().ComputeMarginForAxis(FlexDirection::Column, availableInnerWidth) - ascent;
              maxAscentForCurrentLine = Krys::MaxOrDefined(maxAscentForCurrentLine, ascent);
              maxDescentForCurrentLine = Krys::MaxOrDefined(maxDescentForCurrentLine, descent);
              lineHeight = Krys::MaxOrDefined(lineHeight, maxAscentForCurrentLine + maxDescentForCurrentLine);
            }
          }
        }
        endIterator = iterator;
        currentLead += i != 0 ? crossAxisGap : 0;
        lineHeight += extraSpacePerLine;

        for (iterator = startIterator; iterator != endIterator; iterator++)
        {
          const auto child = *iterator;
          if (child->GetStyle().GetDisplay() == Display::None)
          {
            continue;
          }
          if (child->GetStyle().GetPositionType() != PositionType::Absolute)
          {
            switch (ResolveChildAlignment(node, child))
            {
              case Align::FlexStart:
              {
                child->SetLayoutPosition(
                  currentLead
                    + child->GetStyle().ComputeFlexStartPosition(crossAxis, direction, availableInnerWidth),
                  FlexStartEdge(crossAxis));
                break;
              }
              case Align::FlexEnd:
              {
                child->SetLayoutPosition(
                  currentLead + lineHeight
                    - child->GetStyle().ComputeFlexEndMargin(crossAxis, direction, availableInnerWidth)
                    - child->GetLayout().GetMeasuredDimension(ToDimension(crossAxis)),
                  FlexStartEdge(crossAxis));
                break;
              }
              case Align::Center:
              {
                float childHeight = child->GetLayout().GetMeasuredDimension(ToDimension(crossAxis));

                child->SetLayoutPosition(currentLead + (lineHeight - childHeight) / 2,
                                         FlexStartEdge(crossAxis));
                break;
              }
              case Align::Stretch:
              {
                child->SetLayoutPosition(
                  currentLead
                    + child->GetStyle().ComputeFlexStartMargin(crossAxis, direction, availableInnerWidth),
                  FlexStartEdge(crossAxis));

                // Remeasure child with the line height as it as been only
                // measured with the owners height yet.
                if (!child->HasDefiniteLength(ToDimension(crossAxis), availableInnerCrossDim))
                {
                  const float childWidth =
                    isMainAxisRow ? (child->GetLayout().GetMeasuredDimension(Dimension::Width)
                                     + child->GetStyle().ComputeMarginForAxis(mainAxis, availableInnerWidth))
                                  : leadPerLine + lineHeight;

                  const float childHeight =
                    !isMainAxisRow
                      ? (child->GetLayout().GetMeasuredDimension(Dimension::Height)
                         + child->GetStyle().ComputeMarginForAxis(crossAxis, availableInnerWidth))
                      : leadPerLine + lineHeight;

                  if (!(Krys::InexactEquals(childWidth,
                                            child->GetLayout().GetMeasuredDimension(Dimension::Width))
                        && Krys::InexactEquals(childHeight,
                                               child->GetLayout().GetMeasuredDimension(Dimension::Height))))
                  {
                    CalculateLayoutInternal(child, childWidth, childHeight, direction, SizingMode::StretchFit,
                                            SizingMode::StretchFit, availableInnerWidth, availableInnerHeight,
                                            true, LayoutPassReason::kMultilineStretch, layoutMarkerData,
                                            depth, generationCount);
                  }
                }
                break;
              }
              case Align::Baseline:
              {
                child->SetLayoutPosition(currentLead + maxAscentForCurrentLine - CalculateBaseline(child)
                                           + child->GetStyle().ComputeFlexStartPosition(
                                             FlexDirection::Column, direction, availableInnerCrossDim),
                                         PhysicalEdge::Top);

                break;
              }
              case Align::Auto:
              case Align::SpaceBetween:
              case Align::SpaceAround:
              case Align::SpaceEvenly:  break;
            }
          }
        }

        currentLead = currentLead + leadPerLine + lineHeight;
      }
    }

    // STEP 9: COMPUTING FINAL DIMENSIONS

    node->SetLayoutMeasuredDimension(
      BoundAxis(node, FlexDirection::Row, direction, availableWidth - marginAxisRow, ownerWidth, ownerWidth),
      Dimension::Width);

    node->SetLayoutMeasuredDimension(BoundAxis(node, FlexDirection::Column, direction,
                                               availableHeight - marginAxisColumn, ownerHeight, ownerWidth),
                                     Dimension::Height);

    // If the user didn't specify a width or height for the node, set the
    // dimensions based on the children.
    if (sizingModeMainDim == SizingMode::MaxContent
        || (node->GetStyle().GetOverflow() != Overflow::Scroll
            && sizingModeMainDim == SizingMode::FitContent))
    {
      // Clamp the size to the min/max size, if specified, and make sure it
      // doesn't go below the GetPadding and GetBorder amount.
      node->SetLayoutMeasuredDimension(
        BoundAxis(node, mainAxis, direction, maxLineMainDim, mainAxisOwnerSize, ownerWidth),
        ToDimension(mainAxis));
    }
    else if (sizingModeMainDim == SizingMode::FitContent
             && node->GetStyle().GetOverflow() == Overflow::Scroll)
    {
      node->SetLayoutMeasuredDimension(
        Krys::MaxOrDefined(Krys::MinOrDefined(availableInnerMainDim + GetPaddingAndBorderAxisMain,
                                              BoundAxisWithinMinAndMax(node, direction, mainAxis,
                                                                       NullableFloat {maxLineMainDim},
                                                                       mainAxisOwnerSize, ownerWidth)
                                                .Value()),
                           GetPaddingAndBorderAxisMain),
        ToDimension(mainAxis));
    }

    if (sizingModeCrossDim == SizingMode::MaxContent
        || (node->GetStyle().GetOverflow() != Overflow::Scroll
            && sizingModeCrossDim == SizingMode::FitContent))
    {
      // Clamp the size to the min/max size, if specified, and make sure it
      // doesn't go below the GetPadding and GetBorder amount.
      node->SetLayoutMeasuredDimension(BoundAxis(node, crossAxis, direction,
                                                 totalLineCrossDim + GetPaddingAndBorderAxisCross,
                                                 crossAxisOwnerSize, ownerWidth),
                                       ToDimension(crossAxis));
    }
    else if (sizingModeCrossDim == SizingMode::FitContent
             && node->GetStyle().GetOverflow() == Overflow::Scroll)
    {
      node->SetLayoutMeasuredDimension(
        Krys::MaxOrDefined(
          Krys::MinOrDefined(
            availableInnerCrossDim + GetPaddingAndBorderAxisCross,
            BoundAxisWithinMinAndMax(node, direction, crossAxis,
                                     NullableFloat {totalLineCrossDim + GetPaddingAndBorderAxisCross},
                                     crossAxisOwnerSize, ownerWidth)
              .Value()),
          GetPaddingAndBorderAxisCross),
        ToDimension(crossAxis));
    }

    // As we only wrapped in normal direction yet, we need to reverse the
    // positions on wrap-reverse.
    if (performLayout && node->GetStyle().GetFlexWrap() == Wrap::WrapReverse)
    {
      for (auto child : node->GetLayoutChildren())
      {
        if (child->GetStyle().GetPositionType() != PositionType::Absolute)
        {
          child->SetLayoutPosition(node->GetLayout().GetMeasuredDimension(ToDimension(crossAxis))
                                     - child->GetLayout().GetPosition(FlexStartEdge(crossAxis))
                                     - child->GetLayout().GetMeasuredDimension(ToDimension(crossAxis)),
                                   FlexStartEdge(crossAxis));
        }
      }
    }

    if (performLayout)
    {
      // STEP 10: SETTING TRAILING POSITIONS FOR CHILDREN
      const bool needsMainTrailingPos = NeedsTrailingPosition(mainAxis);
      const bool needsCrossTrailingPos = NeedsTrailingPosition(crossAxis);

      if (needsMainTrailingPos || needsCrossTrailingPos)
      {
        for (auto child : node->GetLayoutChildren())
        {
          // Absolute children will be handled by their containing block since we
          // cannot guarantee that their positions are set when their parents are
          // done with layout.
          if (child->GetStyle().GetDisplay() == Display::None
              || child->GetStyle().GetPositionType() == PositionType::Absolute)
          {
            continue;
          }
          if (needsMainTrailingPos)
          {
            SetChildTrailingPosition(node, child, mainAxis);
          }

          if (needsCrossTrailingPos)
          {
            SetChildTrailingPosition(node, child, crossAxis);
          }
        }
      }

      // STEP 11: SIZING AND POSITIONING ABSOLUTE CHILDREN
      // Let the containing block layout its absolute descendants.
      if (node->GetStyle().GetPositionType() != PositionType::Static || node->AlwaysFormsContainingBlock()
          || depth == 1)
      {
        LayoutAbsoluteDescendants(node, node, isMainAxisRow ? sizingModeMainDim : sizingModeCrossDim,
                                  direction, layoutMarkerData, depth, generationCount, 0.0f, 0.0f,
                                  availableInnerWidth, availableInnerHeight);
      }
    }
  }

  //
  // This is a wrapper around the calculateLayoutImpl function. It determines
  // whether the layout request is redundant and can be skipped.
  //
  // Parameters:
  //  Input parameters are the same as calculateLayoutImpl (see above)
  //  Return parameter is true if layout was performed, false if skipped
  //
  bool CalculateLayoutInternal(Node *node, float availableWidth, float availableHeight,
                               Direction ownerDirection, SizingMode widthSizingMode,
                               SizingMode heightSizingMode, float ownerWidth, float ownerHeight,
                               bool performLayout, LayoutPassReason reason, LayoutData &layoutMarkerData,
                               uint32 depth, uint16 generationCount)
  {
    LayoutResults *layout = &node->GetLayout();

    depth++;

    const bool needToVisitNode = (node->IsDirty() && layout->GenerationCount != generationCount)
                                 || layout->ConfigVersion != node->GetConfig()->GetVersion()
                                 || layout->LastOwnerDirection != ownerDirection;

    if (needToVisitNode)
    {
      // Invalidate the cached results.
      layout->NextCachedMeasurementsIndex = 0;
      layout->CachedLayout.AvailableWidth = -1;
      layout->CachedLayout.AvailableHeight = -1;
      layout->CachedLayout.WidthSizingMode = SizingMode::MaxContent;
      layout->CachedLayout.HeightSizingMode = SizingMode::MaxContent;
      layout->CachedLayout.ComputedWidth = -1;
      layout->CachedLayout.ComputedHeight = -1;
    }

    CachedMeasurement *cachedResults = nullptr;

    // Determine whether the results are already cached. We maintain a separate
    // cache for layouts and measurements. A layout operation modifies the
    // positions and dimensions for nodes in the subtree. The algorithm assumes
    // that each node gets laid out a maximum of one time per tree layout, but
    // multiple measurements may be required to resolve all of the flex
    // dimensions. We handle nodes with measure functions specially here because
    // they are the most expensive to measure, so it's worth avoiding redundant
    // measurements if at all possible.
    if (node->HasMeasureFunc())
    {
      const float marginAxisRow = node->GetStyle().ComputeMarginForAxis(FlexDirection::Row, ownerWidth);
      const float marginAxisColumn = node->GetStyle().ComputeMarginForAxis(FlexDirection::Column, ownerWidth);

      // First, try to use the layout cache.
      if (CanUseCachedMeasurement(widthSizingMode, availableWidth, heightSizingMode, availableHeight,
                                  layout->CachedLayout.WidthSizingMode, layout->CachedLayout.AvailableWidth,
                                  layout->CachedLayout.HeightSizingMode, layout->CachedLayout.AvailableHeight,
                                  layout->CachedLayout.ComputedWidth, layout->CachedLayout.ComputedHeight,
                                  marginAxisRow, marginAxisColumn, node->GetConfig()))
      {
        cachedResults = &layout->CachedLayout;
      }
      else
      {
        // Try to use the measurement cache.
        for (size_t i = 0; i < layout->NextCachedMeasurementsIndex; i++)
        {
          if (CanUseCachedMeasurement(
                widthSizingMode, availableWidth, heightSizingMode, availableHeight,
                layout->CachedMeasurements[i].WidthSizingMode, layout->CachedMeasurements[i].AvailableWidth,
                layout->CachedMeasurements[i].HeightSizingMode, layout->CachedMeasurements[i].AvailableHeight,
                layout->CachedMeasurements[i].ComputedWidth, layout->CachedMeasurements[i].ComputedHeight,
                marginAxisRow, marginAxisColumn, node->GetConfig()))
          {
            cachedResults = &layout->CachedMeasurements[i];
            break;
          }
        }
      }
    }
    else if (performLayout)
    {
      if (Krys::InexactEquals(layout->CachedLayout.AvailableWidth, availableWidth)
          && Krys::InexactEquals(layout->CachedLayout.AvailableHeight, availableHeight)
          && layout->CachedLayout.WidthSizingMode == widthSizingMode
          && layout->CachedLayout.HeightSizingMode == heightSizingMode)
      {
        cachedResults = &layout->CachedLayout;
      }
    }
    else
    {
      for (uint32_t i = 0; i < layout->NextCachedMeasurementsIndex; i++)
      {
        if (Krys::InexactEquals(layout->CachedMeasurements[i].AvailableWidth, availableWidth)
            && Krys::InexactEquals(layout->CachedMeasurements[i].AvailableHeight, availableHeight)
            && layout->CachedMeasurements[i].WidthSizingMode == widthSizingMode
            && layout->CachedMeasurements[i].HeightSizingMode == heightSizingMode)
        {
          cachedResults = &layout->CachedMeasurements[i];
          break;
        }
      }
    }

    if (!needToVisitNode && cachedResults != nullptr)
    {
      layout->SetMeasuredDimension(Dimension::Width, cachedResults->ComputedWidth);
      layout->SetMeasuredDimension(Dimension::Height, cachedResults->ComputedHeight);

      (performLayout ? layoutMarkerData.CachedLayouts : layoutMarkerData.CachedMeasures) += 1;
    }
    else
    {
      CalculateLayoutImpl(node, availableWidth, availableHeight, ownerDirection, widthSizingMode,
                          heightSizingMode, ownerWidth, ownerHeight, performLayout, reason, layoutMarkerData,
                          depth, generationCount);

      layout->LastOwnerDirection = ownerDirection;
      layout->ConfigVersion = node->GetConfig()->GetVersion();

      if (cachedResults == nullptr)
      {
        layoutMarkerData.MaxMeasureCache =
          std::max(layoutMarkerData.MaxMeasureCache, layout->NextCachedMeasurementsIndex + 1u);

        if (layout->NextCachedMeasurementsIndex == LayoutResults::MaxCachedMeasurements)
        {
          layout->NextCachedMeasurementsIndex = 0;
        }

        CachedMeasurement *newCacheEntry = nullptr;
        if (performLayout)
        {
          // Use the single layout cache entry.
          newCacheEntry = &layout->CachedLayout;
        }
        else
        {
          // Allocate a new measurement cache entry.
          newCacheEntry = &layout->CachedMeasurements[layout->NextCachedMeasurementsIndex];
          layout->NextCachedMeasurementsIndex++;
        }

        newCacheEntry->AvailableWidth = availableWidth;
        newCacheEntry->AvailableHeight = availableHeight;
        newCacheEntry->WidthSizingMode = widthSizingMode;
        newCacheEntry->HeightSizingMode = heightSizingMode;
        newCacheEntry->ComputedWidth = layout->GetMeasuredDimension(Dimension::Width);
        newCacheEntry->ComputedHeight = layout->GetMeasuredDimension(Dimension::Height);
      }
    }

    if (performLayout)
    {
      node->SetLayoutDimension(node->GetLayout().GetMeasuredDimension(Dimension::Width), Dimension::Width);
      node->SetLayoutDimension(node->GetLayout().GetMeasuredDimension(Dimension::Height), Dimension::Height);

      node->SetHasNewLayout(true);
      node->SetDirty(false);
    }

    layout->GenerationCount = generationCount;

    LayoutType layoutType;
    if (performLayout)
    {
      layoutType = !needToVisitNode && cachedResults == &layout->CachedLayout ? LayoutType::kCachedLayout
                                                                              : LayoutType::kLayout;
    }
    else
    {
      layoutType = cachedResults != nullptr ? LayoutType::kCachedMeasure : LayoutType::kMeasure;
    }
    Event::Publish<Event::NodeLayout>(node, {layoutType});

    return (needToVisitNode || cachedResults == nullptr);
  }

  void CalculateLayout(Node *const node, const float ownerWidth, const float ownerHeight,
                       const Direction ownerDirection)
  {
    Event::Publish<Event::LayoutPassStart>(node);
    LayoutData markerData = {};

    // Increment the generation count. This will force the recursive routine to
    // visit all dirty nodes at least once. Subsequent visits will be skipped if
    // the input parameters don't change.
    gCurrentGenerationCount.fetch_add(1, std::memory_order_relaxed);
    node->ProcessDimensions();
    const Direction direction = node->ResolveDirection(ownerDirection);
    float width = std::numeric_limits<float>::quiet_NaN();
    SizingMode widthSizingMode = SizingMode::MaxContent;
    const auto &GetStyle = node->GetStyle();
    if (node->HasDefiniteLength(Dimension::Width, ownerWidth))
    {
      width = (node->GetResolvedDimension(direction, ToDimension(FlexDirection::Row), ownerWidth, ownerWidth)
                 .Value()
               + node->GetStyle().ComputeMarginForAxis(FlexDirection::Row, ownerWidth));
      widthSizingMode = SizingMode::StretchFit;
    }
    else if (GetStyle.ResolvedMaxDimension(direction, Dimension::Width, ownerWidth, ownerWidth).HasValue())
    {
      width = GetStyle.ResolvedMaxDimension(direction, Dimension::Width, ownerWidth, ownerWidth).Value();
      widthSizingMode = SizingMode::FitContent;
    }
    else
    {
      width = ownerWidth;
      widthSizingMode = Krys::IsUndefined(width) ? SizingMode::MaxContent : SizingMode::StretchFit;
    }

    float height = std::numeric_limits<float>::quiet_NaN();
    SizingMode heightSizingMode = SizingMode::MaxContent;
    if (node->HasDefiniteLength(Dimension::Height, ownerHeight))
    {
      height =
        (node->GetResolvedDimension(direction, ToDimension(FlexDirection::Column), ownerHeight, ownerWidth)
           .Value()
         + node->GetStyle().ComputeMarginForAxis(FlexDirection::Column, ownerWidth));
      heightSizingMode = SizingMode::StretchFit;
    }
    else if (GetStyle.ResolvedMaxDimension(direction, Dimension::Height, ownerHeight, ownerWidth).HasValue())
    {
      height = GetStyle.ResolvedMaxDimension(direction, Dimension::Height, ownerHeight, ownerWidth).Value();
      heightSizingMode = SizingMode::FitContent;
    }
    else
    {
      height = ownerHeight;
      heightSizingMode = Krys::IsUndefined(height) ? SizingMode::MaxContent : SizingMode::StretchFit;
    }

    if (CalculateLayoutInternal(node, width, height, ownerDirection, widthSizingMode, heightSizingMode,
                                ownerWidth, ownerHeight, true, LayoutPassReason::kInitial, markerData,
                                0, // tree root
                                gCurrentGenerationCount.load(std::memory_order_relaxed)))
    {
      node->SetPosition(node->GetLayout().GetDirection(), ownerWidth, ownerHeight);
      RoundLayoutResultsToPixelGrid(node, 0.0f, 0.0f);
    }

    Event::Publish<Event::LayoutPassEnd>(node, {&markerData});
  }
}