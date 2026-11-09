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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Backgrounds/RepeatStyle.hpp"

#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  RepeatStyle CSSValueConversion<RepeatStyle>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *repeatValue = DynamicDowncast<css::BackgroundRepeatValue>(value))
    {
      return RepeatStyle {.Values {
        FromValueId<FillRepeat>(repeatValue->XValue()),
        FromValueId<FillRepeat>(repeatValue->YValue()),
      }};
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BackgroundRepeatValue.
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return RepeatStyle {.Values {
        FillRepeat::Repeat,
        FillRepeat::Repeat,
      }};
    }

    if (primitiveValue->ValueId() == css::ValueId::RepeatX)
    {
      return RepeatStyle {.Values {
        FillRepeat::Repeat,
        FillRepeat::NoRepeat,
      }};
    }
    if (primitiveValue->ValueId() == css::ValueId::RepeatY)
    {
      return RepeatStyle {.Values {
        FillRepeat::NoRepeat,
        FillRepeat::Repeat,
      }};
    }

    auto repeat = ConvertCSSValueToStyle<FillRepeat>(state, *primitiveValue);
    return RepeatStyle {.Values {
      repeat,
      repeat,
    }};
  }
}
