/*
 * Copyright (C) 2010 Alex Milowski (alex@milowski.com). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/MathML/RenderMathMLMath.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLRowElement.hpp"
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
