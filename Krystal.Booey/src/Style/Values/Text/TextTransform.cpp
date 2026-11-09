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

#include "Krystal.Booey/Style/Values/Text/TextTransform.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  TextTransform CSSValueConversion<TextTransform>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None:         return css::keywords::None {};
        case css::ValueId::Capitalize:   return {TextTransformValue::Capitalize};
        case css::ValueId::Uppercase:    return {TextTransformValue::Uppercase};
        case css::ValueId::Lowercase:    return {TextTransformValue::Lowercase};
        case css::ValueId::FullWidth:    return {TextTransformValue::FullWidth};
        case css::ValueId::FullSizeKana: return {TextTransformValue::FullSizeKana};
        case css::ValueId::MathAuto:     return {TextTransformValue::MathAuto};
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    TextTransformValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Capitalize:
          if (result.Value.ContainsAny({TextTransformValue::Uppercase, TextTransformValue::Lowercase}))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(TextTransformValue::Capitalize);
          break;
        case css::ValueId::Uppercase:
          if (result.Value.ContainsAny({TextTransformValue::Capitalize, TextTransformValue::Lowercase}))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(TextTransformValue::Uppercase);
          break;
        case css::ValueId::Lowercase:
          if (result.Value.ContainsAny({TextTransformValue::Capitalize, TextTransformValue::Uppercase}))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::None {};
          }
          result.Value.Add(TextTransformValue::Lowercase);
          break;
        case css::ValueId::FullWidth:    result.Value.Add(TextTransformValue::FullWidth); break;
        case css::ValueId::FullSizeKana: result.Value.Add(TextTransformValue::FullSizeKana); break;
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
