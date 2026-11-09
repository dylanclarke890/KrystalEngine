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

#include "Krystal.Booey/Style/Values/TextDecoration/TextUnderlinePosition.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  TextUnderlinePosition
    CSSValueConversion<TextUnderlinePosition>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:     return css::keywords::Auto {};
        case css::ValueId::FromFont: return {TextUnderlinePositionValue::FromFont};
        case css::ValueId::Under:    return {TextUnderlinePositionValue::Under};
        case css::ValueId::Left:     return {TextUnderlinePositionValue::Left};
        case css::ValueId::Right:    return {TextUnderlinePositionValue::Right};
        default:                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    TextUnderlinePositionValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::FromFont:
          if (result.Contains(TextUnderlinePositionValue::Under))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::FromFont);
          break;
        case css::ValueId::Under:
          if (result.Contains(TextUnderlinePositionValue::FromFont))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::Under);
          break;
        case css::ValueId::Left:
          if (result.Contains(TextUnderlinePositionValue::Right))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::Left);
          break;
        case css::ValueId::Right:
          if (result.Contains(TextUnderlinePositionValue::Left))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return css::keywords::Auto {};
          }
          result.Value.Add(TextUnderlinePositionValue::Right);
          break;
        default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Auto {};
      }
    }
    return result;
  }
}
