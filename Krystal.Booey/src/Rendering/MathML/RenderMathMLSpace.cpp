/*
 * Copyright (C) 2013 The MathJax Consortium. All rights reserved.
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

#include "Krystal.Booey/Rendering/MathML/RenderMathMLSpace.hpp"
#include "Krystal.Booey/MathML/Elements/MathMLSpaceElement.hpp"
#include <algorithm>

namespace krys::boo
{
  RenderMathMLSpace::RenderMathMLSpace(mathml::MathMLSpaceElement &element, RenderStyle &&style) noexcept
      : RenderMathMLBlock(Type::MathMLSpace, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLSpace());
  }

  RenderMathMLSpace::~RenderMathMLSpace() noexcept = default;

  mathml::MathMLSpaceElement &RenderMathMLSpace::GetElement() const noexcept
  {
    return static_cast<mathml::MathMLSpaceElement &>(NodeForNonAnonymous());
  }

  void RenderMathMLSpace::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    _minPreferredLogicalWidth = _maxPreferredLogicalWidth = SpaceWidth();

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  LayoutUnit RenderMathMLSpace::SpaceWidth() const noexcept
  {
    // FIXME: Negative width values are not supported yet.
    return std::max<LayoutUnit>(0, ToUserUnits(GetElement().Width(), Style(), 0));
  }

  void RenderMathMLSpace::GetSpaceHeightAndDepth(LayoutUnit &height, LayoutUnit &depth) const noexcept
  {
    auto &spaceElement = GetElement();
    height = ToUserUnits(spaceElement.Height(), Style(), 0);
    depth = ToUserUnits(spaceElement.Depth(), Style(), 0);

    // If the total height is negative, set vertical dimensions to 0.
    if (height + depth < 0)
    {
      height = 0;
      depth = 0;
    }
  }

  void RenderMathMLSpace::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    LayoutFloatingChildren();

    RecomputeLogicalWidth();

    SetLogicalWidth(SpaceWidth());
    LayoutUnit height, depth;
    GetSpaceHeightAndDepth(height, depth);
    SetLogicalHeight(height + depth);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    ApplySizeToMathContent(LayoutPhase::Layout, sizes);

    AdjustLayoutForBorderAndPadding();
  }

  Maybe<LayoutUnit> RenderMathMLSpace::FirstLineBaseline() const noexcept
  {
    LayoutUnit height, depth;
    GetSpaceHeightAndDepth(height, depth);
    return height + BorderAndPaddingBefore();
  }
}
