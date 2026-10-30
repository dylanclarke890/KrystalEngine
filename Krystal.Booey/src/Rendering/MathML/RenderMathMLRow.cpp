#include "Krystal.Booey/Rendering/MathML/RenderMathMLRow.hpp"
#include "Krystal.Booey/MathML/MathMLRowElement.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  RenderMathMLRow::RenderMathMLRow(Type type, mathml::MathMLRowElement &element, RenderStyle &&style) noexcept
      : RenderMathMLBlock(type, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLRow());
  }

  RenderMathMLRow::~RenderMathMLRow() noexcept = default;

  mathml::MathMLRowElement &RenderMathMLRow::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLRowElement &>(NodeForNonAnonymous());
  }

  Maybe<LayoutUnit> RenderMathMLRow::FirstLineBaseline() const noexcept
  {
    auto *baselineChild = FirstInFlowChildBox();
    if (!baselineChild)
    {
      return null;
    }

    // Subpixel inline layout is off, so the baseline is rounded.
    return LayoutUnit(std::round(
      (baselineChild->MarginBefore() + baselineChild->LogicalTop() + AscentForChild(*baselineChild))
        .ToFloat()));
  }

  namespace
  {
    KRYS_NODISCARD RenderMathMLOperator *ToVerticalStretchyOperator(RenderBox *box) noexcept
    {
      if (auto *mathMLBlock = DynamicDowncast<RenderMathMLBlock>(box))
      {
        auto *renderOperator = mathMLBlock->UnembellishedOperator();
        if (renderOperator && renderOperator->IsStretchy() && renderOperator->IsVertical())
        {
          return renderOperator;
        }
      }
      return nullptr;
    }
  }

  void RenderMathMLRow::StretchVerticalOperatorsAndLayoutChildren() noexcept
  {
    // First calculate stretch ascent and descent.
    LayoutUnit stretchAscent, stretchDescent;
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      if (ToVerticalStretchyOperator(child))
      {
        continue;
      }
      child->LayoutIfNeeded();
      LayoutUnit childAscent = AscentForChild(*child) + child->MarginBefore();
      LayoutUnit childDescent = child->LogicalHeight() + child->MarginLogicalHeight() - childAscent;
      stretchAscent = std::max(stretchAscent, childAscent);
      stretchDescent = std::max(stretchDescent, childDescent);
    }
    if (stretchAscent + stretchDescent <= 0)
    {
      // We ensure a minimal stretch size.
      stretchAscent = LayoutUnit(Style().ComputedFontSize());
      stretchDescent = 0;
    }

    // Next, we stretch the vertical operators.
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      if (auto *renderOperator = ToVerticalStretchyOperator(child))
      {
        renderOperator->StretchTo(stretchAscent, stretchDescent);
        renderOperator->LayoutIfNeeded();
        child->LayoutIfNeeded();
      }
    }
  }

  void RenderMathMLRow::GetContentBoundingBox(LayoutUnit &width, LayoutUnit &ascent,
                                              LayoutUnit &descent) const noexcept
  {
    ascent = 0;
    descent = 0;
    width = 0;
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      width += child->MarginStart() + child->LogicalWidth() + child->MarginEnd();
      LayoutUnit childAscent = AscentForChild(*child) + child->MarginBefore();
      LayoutUnit childDescent = child->LogicalHeight() + child->MarginLogicalHeight() - childAscent;
      ascent = std::max(ascent, childAscent);
      descent = std::max(descent, childDescent);
    }
  }

  LayoutUnit RenderMathMLRow::PreferredLogicalWidthOfRowItems() noexcept
  {
    LayoutUnit preferredWidth = 0;
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      preferredWidth += child->MaxPreferredLogicalWidth() + MarginIntrinsicLogicalWidthForChild(*child);
    }
    return preferredWidth;
  }

  void RenderMathMLRow::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    _minPreferredLogicalWidth = _maxPreferredLogicalWidth = PreferredLogicalWidthOfRowItems();

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderMathMLRow::LayoutRowItems(LayoutUnit width, LayoutUnit ascent) noexcept
  {
    LayoutUnit horizontalOffset = 0;
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      horizontalOffset += child->MarginStart();
      LayoutUnit childVerticalOffset = ascent - AscentForChild(*child);
      LayoutUnit childWidth = child->LogicalWidth();
      LayoutUnit childHorizontalOffset =
        GetWritingMode().IsBidiLTR() ? horizontalOffset : width - horizontalOffset - childWidth;
      auto repaintRect = child->CheckForRepaintDuringLayout() ? Maybe<LayoutRect> {child->FrameRect()} : null;
      child->SetLocation(LayoutPoint(childHorizontalOffset, childVerticalOffset));
      if (repaintRect)
      {
        repaintRect->UniteEvenIfEmpty(child->FrameRect());
        RepaintRectangle(*repaintRect);
      }
      horizontalOffset += childWidth + child->MarginEnd();
    }
  }

  void RenderMathMLRow::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    LayoutFloatingChildren();

    RecomputeLogicalWidth();
    ComputeAndSetBlockDirectionMarginsOfChildren();

    LayoutUnit width, ascent, descent;
    StretchVerticalOperatorsAndLayoutChildren();
    GetContentBoundingBox(width, ascent, descent);
    LayoutRowItems(width, ascent);
    SetLogicalWidth(width);
    SetLogicalHeight(ascent + descent);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }
}
