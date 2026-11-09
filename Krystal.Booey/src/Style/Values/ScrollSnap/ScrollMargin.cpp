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

#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollMargin.hpp"

#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollMarginEdge CSSValueConversion<ScrollMarginEdge>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    return ScrollMarginEdge {ConvertCSSValueToStyle<Length<>>(state, value)};
  }

  // MARK: - Evaluation

  LayoutUnit Evaluation<ScrollMarginEdge, LayoutUnit>::operator()(const ScrollMarginEdge &edge, LayoutUnit,
                                                                  ZoomNeeded token) noexcept
  {
    return Evaluate<LayoutUnit>(edge._value, token);
  }

  LayoutUnit Evaluation<ScrollMarginEdge, LayoutUnit>::operator()(const ScrollMarginEdge &edge,
                                                                  ZoomNeeded token) noexcept
  {
    return Evaluate<LayoutUnit>(edge._value, token);
  }

  float Evaluation<ScrollMarginEdge, float>::operator()(const ScrollMarginEdge &edge, float,
                                                        ZoomNeeded token) noexcept
  {
    return Evaluate<float>(edge._value, token);
  }

  float Evaluation<ScrollMarginEdge, float>::operator()(const ScrollMarginEdge &edge,
                                                        ZoomNeeded token) noexcept
  {
    return Evaluate<float>(edge._value, token);
  }

  // MARK: - Extent

  KRYS_NODISCARD LayoutBoxExtent ExtentForRect(const ScrollMarginBox &margin, const LayoutRect &rect) noexcept
  {
    return LayoutBoxExtent {
      Evaluate<LayoutUnit>(margin.Top(), rect.Height(), ZoomNeeded {}),
      Evaluate<LayoutUnit>(margin.Right(), rect.Width(), ZoomNeeded {}),
      Evaluate<LayoutUnit>(margin.Bottom(), rect.Height(), ZoomNeeded {}),
      Evaluate<LayoutUnit>(margin.Left(), rect.Width(), ZoomNeeded {}),
    };
  }
}
