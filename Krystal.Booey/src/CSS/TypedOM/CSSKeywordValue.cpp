/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"

namespace krys::boo::css
{
  Ref<CSSKeywordValue> CSSKeywordValue::RectifyKeywordish(CSSKeywordish &&keywordish) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-keywordish-value
    return krys::SwitchOn(
      keywordish, [](const CSSOMString &string) { return AdoptRef(*new CSSKeywordValue(string)); },
      [](RefPtr<CSSKeywordValue> &value) -> Ref<CSSKeywordValue>
      {
        krys_assert(value);
        return krys::move(value);
      });
  }

  dom::ExceptionOr<Ref<CSSKeywordValue>> CSSKeywordValue::Create(const CSSOMString &value) noexcept
  {
    if (value.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    return AdoptRef(*new CSSKeywordValue(value));
  }

  dom::ExceptionOr<void> CSSKeywordValue::SetValue(const CSSOMString &value) noexcept
  {
    if (value.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    _value = value;
    return {};
  }

  void CSSKeywordValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#keywordvalue-serialization
    SerialiseIdentifier(_value, builder);
  }

  RefPtr<css::Value> CSSKeywordValue::ToCSSValue() const noexcept
  {
    auto keyword = ValueIdFromName(_value);
    if (keyword == ValueId::Invalid)
    {
      return PrimitiveValue::CreateCustomIdent(_value);
    }
    return PrimitiveValue::Create(keyword);
  }
}
