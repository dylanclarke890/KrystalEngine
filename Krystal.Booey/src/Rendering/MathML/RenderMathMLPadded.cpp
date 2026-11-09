/*
 * Copyright (C) 2016 Igalia S.L. All rights reserved.
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/MathML/RenderMathMLPadded.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLPaddedElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  RenderMathMLPadded::RenderMathMLPadded(mathml::MathMLPaddedElement &element, RenderStyle &&style) noexcept
      : RenderMathMLRow(Type::MathMLPadded, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLPadded());
  }

  RenderMathMLPadded::~RenderMathMLPadded() noexcept = default;

  mathml::MathMLPaddedElement &RenderMathMLPadded::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLPaddedElement &>(NodeForNonAnonymous());
  }

  LayoutUnit RenderMathMLPadded::Voffset() const noexcept
  {
    return ToUserUnits(GetElement().Voffset(), Style(), 0);
  }

  LayoutUnit RenderMathMLPadded::Lspace() const noexcept
  {
    LayoutUnit lspace = ToUserUnits(GetElement().Lspace(), Style(), 0);
    // FIXME: Negative lspace values are not supported yet (https://bugs.webkit.org/show_bug.cgi?id=85730).
    return std::max<LayoutUnit>(0, lspace);
  }

  LayoutUnit RenderMathMLPadded::MpaddedWidth(LayoutUnit contentWidth) const noexcept
  {
    return std::max<LayoutUnit>(0, ToUserUnits(GetElement().Width(), Style(), contentWidth));
  }

  LayoutUnit RenderMathMLPadded::MpaddedHeight(LayoutUnit contentHeight) const noexcept
  {
    return std::max<LayoutUnit>(0, ToUserUnits(GetElement().Height(), Style(), contentHeight));
  }

  LayoutUnit RenderMathMLPadded::MpaddedDepth(LayoutUnit contentDepth) const noexcept
  {
    return std::max<LayoutUnit>(0, ToUserUnits(GetElement().Depth(), Style(), contentDepth));
  }

  void RenderMathMLPadded::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    // Only the width attribute should modify the width.
    // We parse it using the preferred width of the content as its default value.
    LayoutUnit preferredWidth = PreferredLogicalWidthOfRowItems();
    preferredWidth = MpaddedWidth(preferredWidth);
    _maxPreferredLogicalWidth = _minPreferredLogicalWidth = preferredWidth;

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderMathMLPadded::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
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

    // We first layout our children as a normal <mrow> element.
    LayoutUnit contentWidth, contentAscent, contentDescent;
    StretchVerticalOperatorsAndLayoutChildren();
    GetContentBoundingBox(contentWidth, contentAscent, contentDescent);
    LayoutRowItems(contentWidth, contentAscent);

    // We parse the mpadded attributes using the content metrics as the default value.
    LayoutUnit width = MpaddedWidth(contentWidth);
    LayoutUnit ascent = MpaddedHeight(contentAscent);
    LayoutUnit descent = MpaddedDepth(contentDescent);

    auto inlineShift = Style().GetWritingMode().InlineDirection() == FlowDirection::RightToLeft
                         ? (width - contentWidth - Lspace())
                         : Lspace();

    // Align children on the new baseline and shift them by (lspace, -voffset)
    ShiftInFlowChildren(inlineShift, ascent - contentAscent - Voffset());

    // Set the final metrics.
    SetLogicalWidth(width);
    SetLogicalHeight(ascent + descent);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();

    LayoutOutOfFlowBoxes(relayoutChildren);
  }

  Maybe<LayoutUnit> RenderMathMLPadded::FirstLineBaseline() const noexcept
  {
    // We try and calculate the baseline from the position of the first child.
    LayoutUnit ascent;
    if (auto *baselineChild = FirstInFlowChildBox())
    {
      ascent = AscentForChild(*baselineChild) + baselineChild->LogicalTop() + Voffset();
    }
    else
    {
      ascent = MpaddedHeight(0);
    }
    return ascent;
  }
}
