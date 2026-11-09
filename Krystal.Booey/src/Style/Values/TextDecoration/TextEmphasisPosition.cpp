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

#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisPosition.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  TextEmphasisPosition CSSValueConversion<TextEmphasisPosition>::operator()(BuilderState &state,
                                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Over:  return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
        case css::ValueId::Under: return {TextEmphasisPositionValue::Under, TextEmphasisPositionValue::Right};
        default:
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue>(state, value);
    if (!list)
    {
      return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
    }

    TextEmphasisPositionValueEnumSet result;
    for (const auto &item : *list)
    {
      switch (item.ValueId())
      {
        case css::ValueId::Over:
          if (result.Contains(TextEmphasisPositionValue::Under))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Over);
          break;
        case css::ValueId::Under:
          if (result.Contains(TextEmphasisPositionValue::Over))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Under);
          break;
        case css::ValueId::Left:
          if (result.Contains(TextEmphasisPositionValue::Right))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Left);
          break;
        case css::ValueId::Right:
          if (result.Contains(TextEmphasisPositionValue::Left))
          {
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
          }
          result.Value.Add(TextEmphasisPositionValue::Right);
          break;
        default:
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
      }
    }

    // Result must contain either `over` or `under`.
    if (!result.ContainsAny({TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Under}))
    {
      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return {TextEmphasisPositionValue::Over, TextEmphasisPositionValue::Right};
    }

    // If neither `left` nor `right` has been specified, `right` is added as the default.
    if (!result.ContainsAny({TextEmphasisPositionValue::Left, TextEmphasisPositionValue::Right}))
    {
      result.Value.Add(TextEmphasisPositionValue::Right);
    }

    return result;
  }
}
