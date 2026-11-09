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

#include "Krystal.Booey/Style/Values/Grid/GridAutoFlow.hpp"

#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  GridAutoFlow CSSValueConversion<GridAutoFlow>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal: return css::keywords::Normal {};
        case css::ValueId::Row:    return css::keywords::Row {};
        case css::ValueId::Column: return css::keywords::Column {};
        case css::ValueId::Dense:  return css::keywords::Dense {};
        default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Row {};
      }
    }

    auto list = RequiredListDowncast<css::ValueList, css::PrimitiveValue, 1>(state, value);
    if (!list)
    {
      return css::keywords::Normal {};
    }

    const auto &first = list->item(0);
    switch (first.ValueId())
    {
      case css::ValueId::Row:
        if (list->size() == 2)
        {
          const auto &second = list->item(1);
          switch (second.ValueId())
          {
            case css::ValueId::Dense: return {css::keywords::Row {}, css::keywords::Dense {}};
            default:                  state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Row {};
          }
        }
        return css::keywords::Row {};
      case css::ValueId::Column:
        if (list->size() == 2)
        {
          const auto &second = list->item(1);
          switch (second.ValueId())
          {
            case css::ValueId::Dense: return {css::keywords::Column {}, css::keywords::Dense {}};
            default:                  state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Column {};
          }
        }
        return css::keywords::Column {};
      case css::ValueId::Dense:
        if (list->size() == 2)
        {
          const auto &second = list->item(1);
          switch (second.ValueId())
          {
            case css::ValueId::Row:    return {css::keywords::Row {}, css::keywords::Dense {}};
            case css::ValueId::Column: return {css::keywords::Column {}, css::keywords::Dense {}};
            default:                   state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Dense {};
          }
        }
        return css::keywords::Dense {};
      case css::ValueId::Normal:
        if (list->size() != 1)
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
        }
        return css::keywords::Normal {};
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Row {};
    }
  }
}
