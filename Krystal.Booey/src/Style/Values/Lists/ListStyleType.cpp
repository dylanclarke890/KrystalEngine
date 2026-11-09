/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  bool ListStyleType::IsCircle() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Circle)};
  }

  bool ListStyleType::IsDecimal() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Decimal)};
  }

  bool ListStyleType::IsDisc() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Disc)};
  }

  bool ListStyleType::IsSquare() const noexcept
  {
    return _type == Type::CounterStyle
           && _identifier == css::CSSOMStringAtom {css::ToString(css::ValueId::Square)};
  }

  // MARK: - Conversion

  ListStyleType CSSValueConversion<ListStyleType>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::None {};
    }

    if (primitiveValue->IsValueId())
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }
      return CounterStyle {{css::CSSOMStringAtom {primitiveValue->StringValue()}}};
    }
    if (primitiveValue->IsCustomIdent())
    {
      return CounterStyle {{css::CSSOMStringAtom {primitiveValue->StringValue()}}};
    }

    return css::CSSOMStringAtom {primitiveValue->StringValue()};
  }

  Ref<css::Value> CSSValueCreation<ListStyleType>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                              const ListStyleType &value) noexcept
  {
    return krys::SwitchOn(
      value,
      [&](const css::keywords::None &none) { return style::CreateCSSValue(pool, style, none); }, // none
      [&](const CounterStyle &counterStyle)
      { return style::CreateCSSValue(pool, style, counterStyle.Identifier); }, // custom-ident
      [&](const css::CSSOMStringAtom &identifier)
      { return style::CreateCSSValue(pool, style, identifier); }); // string
  }
}
