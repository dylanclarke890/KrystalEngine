/*
 * Copyright (C) 2008, 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  FunctionValue::FunctionValue(css::ValueId name, ValueListBuilder arguments) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(arguments)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator), _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument1),
                            krys::move(argument2)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                               Ref<Value> argument3) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument1),
                            krys::move(argument2), krys::move(argument3)),
        _name(name)
  {
  }

  FunctionValue::FunctionValue(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                               Ref<Value> argument3, Ref<Value> argument4) noexcept
      : ValueContainingList(ValueType::Function, ValueSeparator::CommaSeparator, krys::move(argument1),
                            krys::move(argument2), krys::move(argument3), krys::move(argument4)),
        _name(name)
  {
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, ValueListBuilder arguments) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(arguments)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name) noexcept
  {
    return AdoptRef(*new FunctionValue(name));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(argument)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument1,
                                           Ref<Value> argument2) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(argument1), krys::move(argument2)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                                           Ref<Value> argument3) noexcept
  {
    return AdoptRef(
      *new FunctionValue(name, krys::move(argument1), krys::move(argument2), krys::move(argument3)));
  }

  Ref<FunctionValue> FunctionValue::Create(css::ValueId name, Ref<Value> argument1, Ref<Value> argument2,
                                           Ref<Value> argument3, Ref<Value> argument4) noexcept
  {
    return AdoptRef(*new FunctionValue(name, krys::move(argument1), krys::move(argument2),
                                       krys::move(argument3), krys::move(argument4)));
  }

  CSSOMString FunctionValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    StringBuilder result;
    result.Append(ToString(_name));
    result.Append(u8'(');
    SerialiseItems(result, context);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  bool FunctionValue::AddDerivedHash(size_t &seed) const noexcept
  {
    seed = Hash::Combine(seed, _name);
    return ValueContainingList::AddDerivedHash(seed);
  }
}
