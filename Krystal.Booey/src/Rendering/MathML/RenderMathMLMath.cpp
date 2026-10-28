#include "Krystal.Booey/Rendering/MathML/RenderMathMLMath.hpp"
#include "Krystal.Booey/MathML/MathMLRowElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderMathMLMath::RenderMathMLMath(mathml::MathMLRowElement &element, RenderStyle &&style) noexcept
      : RenderMathMLRow(Type::MathMLMath, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLMath());
  }

  RenderMathMLMath::~RenderMathMLMath() noexcept = default;

  void RenderMathMLMath::CenterChildren(LayoutUnit contentWidth) noexcept
  {
    auto centerBlockOffset = (LogicalWidth() - contentWidth) / 2;
    if (!centerBlockOffset)
    {
      return;
    }

    if (!Style().IsLeftToRightDirection())
    {
      centerBlockOffset = -centerBlockOffset;
    }
    for (auto *child = FirstInFlowChildBox(); child; child = child->NextInFlowSiblingBox())
    {
      auto repaintRect = child->CheckForRepaintDuringLayout() ? Maybe<LayoutRect> {child->FrameRect()} : null;
      child->Move(centerBlockOffset, {});
      if (repaintRect)
      {
        repaintRect->UniteEvenIfEmpty(child->FrameRect());
        RepaintRectangle(*repaintRect);
      }
    }
  }

  void RenderMathMLMath::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit pageLogicalHeight) noexcept
  {
    krys_debug_assert(NeedsLayout());

    if (Style().Display() != DisplayType::Block)
    {
      RenderMathMLRow::LayoutBlock(relayoutChildren, pageLogicalHeight);
      return;
    }

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    LayoutFloatingChildren();

    RecomputeLogicalWidth();
    ComputeAndSetBlockDirectionMarginsOfChildren();

    SetLogicalHeight(BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight()));

    LayoutUnit width, ascent, descent;
    StretchVerticalOperatorsAndLayoutChildren();
    GetContentBoundingBox(width, ascent, descent);
    LayoutRowItems(LogicalWidth(), ascent);

    // Display formulas must be centered horizontally if there is extra space left.
    // Otherwise, logical width must be updated to the content width to avoid truncation.
    if (width < LogicalWidth())
    {
      CenterChildren(width);
    }
    else
    {
      SetLogicalWidth(width);
    }
    ShiftInFlowChildren(0_lu, BorderAndPaddingBefore());

    SetLogicalHeight(ascent + descent + BorderAndPaddingLogicalHeight()
                     + LayoutUnit(HorizontalScrollbarHeight()));
    UpdateLogicalHeight();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }
}
