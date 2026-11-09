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

#include "Krystal.Booey/Style/Values/Text/TextIndent.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  TextIndent CSSValueConversion<TextIndent>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return ConvertCSSValueToStyle<TextIndentLength>(state, *primitiveValue);
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return 0_css_px;
    }

    Maybe<TextIndentLength> length;
    Maybe<css::keywords::Hanging> hanging;
    Maybe<css::keywords::EachLine> eachLine;

    for (const auto &primitiveValue : *list)
    {
      if (primitiveValue.ValueId() == css::ValueId::Hanging)
      {
        hanging = css::keywords::Hanging {};
      }
      else if (primitiveValue.ValueId() == css::ValueId::EachLine)
      {
        eachLine = css::keywords::EachLine {};
      }
      else
      {
        length = ConvertCSSValueToStyle<TextIndentLength>(state, primitiveValue);
      }
    }

    if (!length)
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return 0_css_px;
    }

    return TextIndent {krys::move(*length), hanging, eachLine};
  }

  // MARK: - Blending

  bool Blending<TextIndent>::CanBlend(const TextIndent &a, const TextIndent &b) noexcept
  {
    return a.Hanging == b.Hanging && a.EachLine == b.EachLine && style::CanBlend(a.Length, b.Length);
  }

  TextIndent Blending<TextIndent>::Blend(const TextIndent &a, const TextIndent &b,
                                         const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(a.Hanging == b.Hanging);
    krys_debug_assert(a.EachLine == b.EachLine);
    return TextIndent {style::Blend(a.Length, b.Length, context), a.Hanging, a.EachLine};
  }
}
