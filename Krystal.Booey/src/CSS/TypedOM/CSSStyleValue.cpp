/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/CSSStyleValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValueFactory.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSStyleValue>> CSSStyleValue::Parse(dom::Document &document, CSSOMStringView property,
                                                            CSSOMStringView cssText) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssstylevalue-parse
    auto result = CSSStyleValueFactory::ParseStyleValue(document, property, cssText, false);
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    auto values = result.ReleaseValue();
    // A shorthand that serialises to nothing gives no values.
    if (values.empty())
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError};
    }
    return krys::move(values.front());
  }

  dom::ExceptionOr<List<Ref<CSSStyleValue>>> CSSStyleValue::ParseAll(dom::Document &document,
                                                                     CSSOMStringView property,
                                                                     CSSOMStringView cssText) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssstylevalue-parseall
    return CSSStyleValueFactory::ParseStyleValue(document, property, cssText, true);
  }

  Ref<CSSStyleValue> CSSStyleValue::Create(RefPtr<Value> &&cssValue, CSSOMString &&property) noexcept
  {
    return AdoptRef(*new CSSStyleValue(krys::move(cssValue), krys::move(property)));
  }

  Ref<CSSStyleValue> CSSStyleValue::Create() noexcept
  {
    return AdoptRef(*new CSSStyleValue());
  }

  CSSStyleValue::CSSStyleValue(RefPtr<Value> &&cssValue, CSSOMString &&property) noexcept
      : _customPropertyName(krys::move(property)), _propertyValue(krys::move(cssValue))
  {
  }

  CSSStyleValue::CSSStyleValue() noexcept = default;

  CSSStyleValue::~CSSStyleValue() noexcept = default;

  CSSOMString CSSStyleValue::ToString() const noexcept
  {
    StringBuilder builder;
    Serialise(builder);
    return krys::move(builder).ToString();
  }

  void CSSStyleValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    if (_propertyValue)
    {
      builder.Append(_propertyValue->CssText(DefaultSerialisationContext()));
    }
  }

  RefPtr<Value> CSSStyleValue::ToCSSValue() const noexcept
  {
    return _propertyValue;
  }
}
