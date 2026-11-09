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

#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlineOffset.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  float TextUnderlineOffset::Resolve(const RenderStyle &style, float autoValue) const noexcept
  {
    return krys::SwitchOn(
      *this, [&](const css::keywords::Auto &) -> float { return autoValue; }, [&](const Fixed &fixed) -> float
      { return style::Evaluate<float>(fixed, style.UsedZoomForLength()); }, [&](const Calc &calc) -> float
      { return style::Evaluate<float>(calc, style.ComputedFontSize(), style.UsedZoomForLength()); },
      [&](const Percentage &percentage) -> float
      { return style::Evaluate<float>(percentage, style.ComputedFontSize()); });
  }

  // MARK: - Blending

  bool Blending<TextUnderlineOffset>::CanBlend(const TextUnderlineOffset &a, const TextUnderlineOffset &b,
                                               const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
  {
    if (a.IsAuto() || b.IsAuto())
    {
      return false;
    }

    return a.Resolve(aStyle) != b.Resolve(bStyle);
  }

  TextUnderlineOffset Blending<TextUnderlineOffset>::Blend(const TextUnderlineOffset &a,
                                                           const TextUnderlineOffset &b,
                                                           const RenderStyle &aStyle,
                                                           const RenderStyle &bStyle,
                                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    return TextUnderlineOffset {
      TextUnderlineOffset::Fixed {boo::Blend(a.Resolve(aStyle), b.Resolve(bStyle), context)}};
  }
}
