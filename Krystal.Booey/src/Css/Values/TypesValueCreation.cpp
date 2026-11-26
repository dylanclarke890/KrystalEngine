/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Css/Values/FunctionValue.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Css/Values/QuadValue.hpp"
#include "Krystal.Booey/Css/Values/Types.hpp"
#include "Krystal.Booey/Css/Values/ValueList.hpp"
#include "Krystal.Booey/Css/Values/ValuePair.hpp"
#include "Krystal.Booey/Css/Values/ValuePool.hpp"

namespace krys::boo::css
{
  Ref<Value> CreatePrimitiveValue(ValueId keyword) noexcept
  {
    return PrimitiveValue::Create(keyword);
  }

  Ref<Value> CreatePrimitiveValue(const CustomIdentifier &identifier) noexcept
  {
    return PrimitiveValue::CreateCustomIdent(identifier.Value);
  }

  Ref<Value> CreatePrimitiveValue(const PropertyIdentifier &identifier) noexcept
  {
    return PrimitiveValue::Create(identifier.Value);
  }

  Ref<Value> CreatePrimitiveValue(const CSSOMStringAtom &string) noexcept
  {
    return PrimitiveValue::Create(string);
  }

  Ref<Value> CreatePrimitiveValue(const CSSOMString &string) noexcept
  {
    return PrimitiveValue::Create(string);
  }

  Ref<Value> CreateFunctionValue(ValueId name, Ref<Value> &&arguments) noexcept
  {
    return FunctionValue::Create(name, krys::move(arguments));
  }

  template <>
  Ref<Value> CreateCoalescingPairValue<SerialisationSeparatorType::Space>(Ref<Value> &&first,
                                                                          Ref<Value> &&second) noexcept
  {
    return ValuePair::Create(krys::move(first), krys::move(second));
  }

  template <>
  Ref<Value> CreateCoalescingQuadValue<SerialisationSeparatorType::Space>(Ref<Value> &&first,
                                                                          Ref<Value> &&second,
                                                                          Ref<Value> &&third,
                                                                          Ref<Value> &&fourth) noexcept
  {
    return QuadValue::Create(krys::move(first), krys::move(second), krys::move(third), krys::move(fourth));
  }

  template <>
  Ref<Value> CreateListValue<SerialisationSeparatorType::Space>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSpaceSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListValue<SerialisationSeparatorType::Comma>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateCommaSeparated(krys::move(builder));
  }

  template <>
  Ref<Value> CreateListValue<SerialisationSeparatorType::Slash>(ValueListBuilder &&builder) noexcept
  {
    return ValueList::CreateSlashSeparated(krys::move(builder));
  }
}
