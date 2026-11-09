/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  // MARK: - Evaluation

  LayoutUnit Evaluation<ScrollPaddingEdge, LayoutUnit>::operator()(const ScrollPaddingEdge &edge,
                                                                   LayoutUnit referenceLength,
                                                                   ZoomNeeded token) noexcept
  {
    return krys::SwitchOn(
      edge, [&](const ScrollPaddingEdge::Fixed &fixed) { return Evaluate<LayoutUnit>(fixed, token); },
      [&](const ScrollPaddingEdge::Percentage &percentage)
      { return Evaluate<LayoutUnit>(percentage, referenceLength); },
      [&](const ScrollPaddingEdge::Calc &calculated)
      { return Evaluate<LayoutUnit>(calculated, referenceLength, token); },
      [&](const css::keywords::Auto &) { return 0_lu; });
  }

  float Evaluation<ScrollPaddingEdge, float>::operator()(const ScrollPaddingEdge &edge, float referenceLength,
                                                         ZoomNeeded token) noexcept
  {
    return krys::SwitchOn(
      edge, [&](const ScrollPaddingEdge::Fixed &fixed) { return Evaluate<float>(fixed, token); },
      [&](const ScrollPaddingEdge::Percentage &percentage)
      { return Evaluate<float>(percentage, referenceLength); }, [&](const ScrollPaddingEdge::Calc &calculated)
      { return Evaluate<float>(calculated, referenceLength, token); },
      [&](const css::keywords::Auto &) { return 0.0f; });
  }

  KRYS_NODISCARD LayoutBoxExtent ExtentForRect(const ScrollPaddingBox &padding, const LayoutRect &rect,
                                               ZoomNeeded token) noexcept
  {
    return LayoutBoxExtent {
      Evaluate<LayoutUnit>(padding.Top(), rect.Height(), token),
      Evaluate<LayoutUnit>(padding.Right(), rect.Width(), token),
      Evaluate<LayoutUnit>(padding.Bottom(), rect.Height(), token),
      Evaluate<LayoutUnit>(padding.Left(), rect.Width(), token),
    };
  }
}
