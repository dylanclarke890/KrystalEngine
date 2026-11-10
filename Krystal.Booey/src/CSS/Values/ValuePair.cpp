/*
 * Copyright (C) 2021 Tyler Wilcock <twilco.o@protonmail.com>.
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

#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Core/Hash.hpp"

namespace krys::boo::css
{
  ValuePair::ValuePair(ValueSeparator separator, Ref<Value> first, Ref<Value> second,
                       IdenticalValueSerialisation serialisation) noexcept
      : Value(ValueType::ValuePair), _first(krys::move(first)), _second(krys::move(second))
  {
    _valueSeparator = separator;
    _coalesceIdenticalValues = serialisation != IdenticalValueSerialisation::DoNotCoalesce;
  }

  // A pair is a value and its two references: its coalescing bit is in the value's bit fields.
  static_assert(sizeof(ValuePair)
                == ((sizeof(Value) + alignof(Ref<Value>) - 1) / alignof(Ref<Value>) * alignof(Ref<Value>))
                     + (2 * sizeof(Ref<Value>)));

  Ref<ValuePair> ValuePair::Create(Ref<Value> first, Ref<Value> second) noexcept
  {
    return AdoptRef(*new ValuePair(ValueSeparator::SpaceSeparator, krys::move(first), krys::move(second),
                                   IdenticalValueSerialisation::Coalesce));
  }

  Ref<ValuePair> ValuePair::CreateSlashSeparated(Ref<Value> first, Ref<Value> second) noexcept
  {
    return AdoptRef(*new ValuePair(ValueSeparator::SlashSeparator, krys::move(first), krys::move(second),
                                   IdenticalValueSerialisation::DoNotCoalesce));
  }

  Ref<ValuePair> ValuePair::CreateNonCoalescing(Ref<Value> first, Ref<Value> second) noexcept
  {
    return AdoptRef(*new ValuePair(ValueSeparator::SpaceSeparator, krys::move(first), krys::move(second),
                                   IdenticalValueSerialisation::DoNotCoalesce));
  }

  bool ValuePair::CanBeCoalesced() const noexcept
  {
    return _coalesceIdenticalValues && _first->Equals(*_second);
  }

  CSSOMString ValuePair::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    auto first = _first->CssText(context);
    auto second = _second->CssText(context);
    if (_coalesceIdenticalValues && first == second)
    {
      return first;
    }
    return first.append(SeparatorCSSText()).append(second);
  }

  bool ValuePair::Equals(const ValuePair &other) const noexcept
  {
    return _valueSeparator == other._valueSeparator
           && _coalesceIdenticalValues == other._coalesceIdenticalValues && _first->Equals(*other._first)
           && _second->Equals(*other._second);
  }

  bool ValuePair::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, _valueSeparator, _coalesceIdenticalValues);
    return _first->AddHash(seed) && _second->AddHash(seed);
  }
}
