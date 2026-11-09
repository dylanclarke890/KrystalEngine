/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationThickness.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontMetrics.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"

namespace krys::boo::style
{
  static constexpr float textDecorationBaseFontSize = 16;

  float TextDecorationThickness::Resolve(const RenderStyle &style) const noexcept
  {
    return krys::SwitchOn(
      _value, [&](const css::keywords::Auto &)
      { return style.ComputedFontSize() / textDecorationBaseFontSize; }, [&](const css::keywords::FromFont &)
      { return style.MetricsOfPrimaryFont().UnderlineThickness().value_or(0); },
      [&](const TextDecorationThicknessLength &length)
      { return style::Evaluate<float>(length, style.ComputedFontSize(), style.UsedZoomForLength()); });
  }

  // MARK: - Conversion

  TextDecorationThickness
    CSSValueConversion<TextDecorationThickness>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->IsValueId())
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:     return css::keywords::Auto {};
        case css::ValueId::FromFont: return css::keywords::FromFont {};
        default:                     break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    return ConvertCSSValueToStyle<TextDecorationThicknessLength>(state, *primitiveValue);
  }

  // MARK: - Blending

  bool Blending<TextDecorationThickness>::CanBlend(const TextDecorationThickness &a,
                                                   const TextDecorationThickness &b,
                                                   const RenderStyle &aStyle,
                                                   const RenderStyle &bStyle) noexcept
  {
    if (a.IsAuto() || b.IsAuto())
    {
      return false;
    }
    return a.Resolve(aStyle) != b.Resolve(bStyle);
  }

  TextDecorationThickness Blending<TextDecorationThickness>::Blend(const TextDecorationThickness &a,
                                                                   const TextDecorationThickness &b,
                                                                   const RenderStyle &aStyle,
                                                                   const RenderStyle &bStyle,
                                                                   const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    return TextDecorationThicknessLength {
      TextDecorationThicknessLength::Fixed {boo::Blend(a.Resolve(aStyle), b.Resolve(bStyle), context)}};
  }
}
