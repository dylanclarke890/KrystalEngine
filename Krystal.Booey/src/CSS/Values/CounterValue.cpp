/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/CounterValue.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  CounterValue::CounterValue(CSSOMStringAtom &&identifier, CSSOMStringAtom &&separator,
                             Ref<Value> &&counterStyle) noexcept
      : Value(ValueType::Counter), _identifier(krys::move(identifier)), _separator(krys::move(separator)),
        _counterStyle(krys::move(counterStyle))
  {
  }

  Ref<CounterValue> CounterValue::Create(CSSOMStringAtom identifier, CSSOMStringAtom separator,
                                         Ref<Value> counterStyle) noexcept
  {
    return AdoptRef(
      *new CounterValue(krys::move(identifier), krys::move(separator), krys::move(counterStyle)));
  }

  bool CounterValue::Equals(const CounterValue &other) const noexcept
  {
    return _identifier == other._identifier && _separator == other._separator
           && CompareValue(_counterStyle, other._counterStyle);
  }

  CSSOMString CounterValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    bool isDecimal = _counterStyle->ValueId() == css::ValueId::Decimal
                     || (_counterStyle->IsCustomIdent() && _counterStyle->CustomIdent() == u8"decimal");
    CSSOMStringView styleSeparator = isDecimal ? u8"" : u8", ";
    CSSOMString styleLiteral = isDecimal ? CSSOMString() : CounterStyleCSSText();

    StringBuilder result;
    if (_separator == CSSOMStringAtom::Null() || _separator.View().empty())
    {
      result.Append(u8"counter(", _identifier, styleSeparator, styleLiteral, u8')');
      return krys::move(result).ToString();
    }
    result.Append(u8"counters(", _identifier, u8", ");
    SerialiseString(CSSOMString(_separator.View()), result);
    result.Append(styleSeparator, styleLiteral, u8')');
    return krys::move(result).ToString();
  }

  CSSOMString CounterValue::CounterStyleCSSText() const noexcept
  {
    if (_counterStyle->IsValueId())
    {
      return CSSOMString(ToString(_counterStyle->ValueId()));
    }
    if (_counterStyle->IsCustomIdent())
    {
      return _counterStyle->CustomIdent();
    }

    krys_debug_assert_msg(false, "A counter style is a keyword or a custom ident.");
    return CSSOMString();
  }
}
