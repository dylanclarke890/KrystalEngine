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

#include "Krystal.Booey/Style/Values/Transforms/Perspective.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Perspective CSSValueConversion<Perspective>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::None {};
    }

    if (primitiveValue->ValueId() == css::ValueId::None)
    {
      return css::keywords::None {};
    }

    auto &conversionData = state.CssToLengthConversionData();

    // NOTE: The isNumber() case below is only possible due to the `-webkit-perspective` legacy shorthand
    // which extends the grammar to `<'perspective'> | <number [0,inf]>`.

    float perspective = -1;
    if (primitiveValue->IsLength())
    {
      perspective = primitiveValue->ResolveAsLength<float>(conversionData);
    }
    else if (primitiveValue->IsNumber())
    {
      perspective = primitiveValue->ResolveAsNumber<float>(conversionData) * conversionData.Zoom();
    }
    else
    {
      krys_debug_assert_msg(false, "Unreachable");
    }

    // FIXME: This should probably clamp to 0, like other numeric values would, rather than return
    // css::keywords::None.
    if (perspective < 0)
    {
      return css::keywords::None {};
    }

    return style::Perspective::Length {perspective};
  }

  // MARK: - Blending

  bool Blending<Perspective>::CanBlend(const Perspective &from, const Perspective &to) noexcept
  {
    return !from.IsNone() && !to.IsNone();
  }

  Perspective Blending<Perspective>::Blend(const Perspective &from, const Perspective &to,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? to : from;
    }

    krys_debug_assert(!from.IsNone());
    krys_debug_assert(!to.IsNone());
    return {krys::boo::style::Blend(*from.TryValue(), *to.TryValue(), context)};
  }
}
