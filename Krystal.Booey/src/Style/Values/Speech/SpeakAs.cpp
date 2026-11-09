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

#include "Krystal.Booey/Style/Values/Speech/SpeakAs.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  SpeakAs CSSValueConversion<SpeakAs>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:
        case css::ValueId::Normal:             return css::keywords::Normal {};
        case css::ValueId::SpellOut:           return {SpeakAsValue::SpellOut};
        case css::ValueId::Digits:             return {SpeakAsValue::Digits};
        case css::ValueId::LiteralPunctuation: return {SpeakAsValue::LiteralPunctuation};
        case css::ValueId::NoPunctuation:      return {SpeakAsValue::NoPunctuation};
        default:                               state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    SpeakAsValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::SpellOut: result.Value.Add(SpeakAsValue::SpellOut); break;
        case css::ValueId::Digits:   result.Value.Add(SpeakAsValue::Digits); break;
        case css::ValueId::LiteralPunctuation:
          if (result.Contains(SpeakAsValue::NoPunctuation))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Normal {};
          }
          result.Value.Add(SpeakAsValue::LiteralPunctuation);
          break;
        case css::ValueId::NoPunctuation:
          if (result.Contains(SpeakAsValue::LiteralPunctuation))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Normal {};
          }
          result.Value.Add(SpeakAsValue::NoPunctuation);
          break;
        default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }
    return result;
  }
}
