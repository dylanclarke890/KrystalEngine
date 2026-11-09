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

#include "Krystal.Booey/Style/Values/Text/HangingPunctuation.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  HangingPunctuation CSSValueConversion<HangingPunctuation>::operator()(BuilderState &state,
                                                                        const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:     return css::keywords::None {};
        case css::ValueId::First:    return {HangingPunctuationValue::First};
        case css::ValueId::ForceEnd: return {HangingPunctuationValue::ForceEnd};
        case css::ValueId::AllowEnd: return {HangingPunctuationValue::AllowEnd};
        case css::ValueId::Last:     return {HangingPunctuationValue::Last};
        default:                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    HangingPunctuationValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::First: result.Value.Add(HangingPunctuationValue::First); break;
        case css::ValueId::ForceEnd:
          if (result.Value.Contains(HangingPunctuationValue::AllowEnd))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(HangingPunctuationValue::ForceEnd);
          break;
        case css::ValueId::AllowEnd:
          if (result.Value.Contains(HangingPunctuationValue::ForceEnd))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(HangingPunctuationValue::AllowEnd);
          break;
        case css::ValueId::Last: result.Value.Add(HangingPunctuationValue::Last); break;
        default:                 state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
