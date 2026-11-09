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

#include "Krystal.Booey/Style/Values/Box/MarginTrim.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  MarginTrim CSSValueConversion<MarginTrim>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    // See if value is "block" or "inline" before trying to parse a list
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Block: return {style::MarginTrimSide::BlockStart, style::MarginTrimSide::BlockEnd};
        case css::ValueId::Inline:
          return {style::MarginTrimSide::InlineStart, style::MarginTrimSide::InlineEnd};
        case css::ValueId::BlockStart:  return {style::MarginTrimSide::BlockStart};
        case css::ValueId::BlockEnd:    return {style::MarginTrimSide::BlockEnd};
        case css::ValueId::InlineStart: return {style::MarginTrimSide::InlineStart};
        case css::ValueId::InlineEnd:   return {style::MarginTrimSide::InlineEnd};
        default:                        state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return css::keywords::None {};
    }

    MarginTrimSideEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Block:
          result.Value.Add({style::MarginTrimSide::BlockStart, style::MarginTrimSide::BlockEnd});
          break;
        case css::ValueId::Inline:
          result.Value.Add({style::MarginTrimSide::InlineStart, style::MarginTrimSide::InlineEnd});
          break;
        case css::ValueId::BlockStart:  result.Value.Add(style::MarginTrimSide::BlockStart); break;
        case css::ValueId::BlockEnd:    result.Value.Add(style::MarginTrimSide::BlockEnd); break;
        case css::ValueId::InlineStart: result.Value.Add(style::MarginTrimSide::InlineStart); break;
        case css::ValueId::InlineEnd:   result.Value.Add(style::MarginTrimSide::InlineEnd); break;
        default:                        state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::None {};
      }
    }
    return result;
  }
}
