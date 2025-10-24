#include "Krystal.UI.Layout/Algorithm/FlexLine.hpp"
#include "Krystal.UI.Layout/Algorithm/BoundAxis.hpp"
#include "Krystal.UI.Styles/Helpers/FlexDirection.hpp"

namespace Krys::UI::Layout
{
  using namespace Styles;

  FlexLine CalculateFlexLine(Node *const node, const Direction ownerDirection, const float ownerWidth,
                             const float mainAxisOwnerSize, const float availableInnerWidth,
                             const float availableInnerMainDim, Node::LayoutableChildren::Iterator &iterator,
                             const uint16 lineCount)
  {
    std::vector<Node *> itemsInFlow;
    itemsInFlow.reserve(node->GetChildCount());

    float sizeConsumed = 0.0f;
    float totalFlexGrowFactors = 0.0f;
    float totalFlexShrinkScaledFactors = 0.0f;
    size_t numberOfAutoMargins = 0;
    Node *firstElementInLine = nullptr;

    float sizeConsumedIncludingMinConstraint = 0;
    const Direction direction = node->ResolveDirection(ownerDirection);
    const FlexDirection mainAxis = ResolveDirection(node->GetStyle().GetFlexDirection(), direction);
    const bool isNodeFlexWrap = node->GetStyle().GetFlexWrap() != Wrap::NoWrap;
    const float gap = node->GetStyle().ComputeGapForAxis(mainAxis, availableInnerMainDim);

    const auto childrenEnd = node->GetLayoutChildren().end();
    // Add items to the current line until it's full or we run out of items.
    for (; iterator != childrenEnd; iterator++)
    {
      auto child = *iterator;
      if (child->GetStyle().GetDisplay() == Display::None
          || child->GetStyle().GetPositionType() == PositionType::Absolute)
      {
        continue;
      }

      if (firstElementInLine == nullptr)
      {
        firstElementInLine = child;
      }

      if (child->GetStyle().IsFlexStartMarginAuto(mainAxis, ownerDirection))
      {
        numberOfAutoMargins++;
      }
      if (child->GetStyle().IsFlexEndMarginAuto(mainAxis, ownerDirection))
      {
        numberOfAutoMargins++;
      }

      child->SetLineIndex(lineCount);
      const float childMarginMainAxis = child->GetStyle().ComputeMarginForAxis(mainAxis, availableInnerWidth);
      const float childLeadingGapMainAxis = child == firstElementInLine ? 0.0f : gap;
      const float flexBasisWithMinAndMaxConstraints =
        BoundAxisWithinMinAndMax(child, direction, mainAxis, child->GetLayout().ComputedFlexBasis,
                                 mainAxisOwnerSize, ownerWidth)
          .Value();

      // If this is a multi-line flow and this item pushes us over the available
      // size, we've hit the end of the current line. Break out of the loop and
      // lay out the current line.
      if (sizeConsumedIncludingMinConstraint + flexBasisWithMinAndMaxConstraints + childMarginMainAxis
              + childLeadingGapMainAxis
            > availableInnerMainDim
          && isNodeFlexWrap && !itemsInFlow.empty())
      {
        break;
      }

      sizeConsumedIncludingMinConstraint +=
        flexBasisWithMinAndMaxConstraints + childMarginMainAxis + childLeadingGapMainAxis;
      sizeConsumed += flexBasisWithMinAndMaxConstraints + childMarginMainAxis + childLeadingGapMainAxis;

      if (child->IsNodeFlexible())
      {
        totalFlexGrowFactors += child->ResolveFlexGrow();

        // Unlike the grow factor, the shrink factor is scaled relative to the
        // child dimension.
        totalFlexShrinkScaledFactors +=
          -child->ResolveFlexShrink() * child->GetLayout().ComputedFlexBasis.Value();
      }

      itemsInFlow.push_back(child);
    }

    // The total flex factor needs to be floored to 1.
    if (totalFlexGrowFactors > 0 && totalFlexGrowFactors < 1)
    {
      totalFlexGrowFactors = 1;
    }

    // The total flex shrink factor needs to be floored to 1.
    if (totalFlexShrinkScaledFactors > 0 && totalFlexShrinkScaledFactors < 1)
    {
      totalFlexShrinkScaledFactors = 1;
    }

    return FlexLine {.ItemsInFlow = std::move(itemsInFlow),
                     .Layout =
                       FlexLineRunningLayout {
                         totalFlexGrowFactors,
                         totalFlexShrinkScaledFactors,
                       },
                     .SizeConsumed = sizeConsumed,
                     .NumberOfAutoMargins = numberOfAutoMargins};
  }
}