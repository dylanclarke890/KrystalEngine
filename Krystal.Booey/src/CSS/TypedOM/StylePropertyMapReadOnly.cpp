/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/StylePropertyMapReadOnly.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSStyleValueFactory.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css
{
  StylePropertyMapReadOnly::~StylePropertyMapReadOnly() noexcept = default;

  RefPtr<CSSStyleValue> StylePropertyMapReadOnly::ReifyValue(dom::Document &document, RefPtr<Value> value,
                                                             Maybe<PropertyId> property) noexcept
  {
    if (!value)
    {
      return nullptr;
    }
    auto result = CSSStyleValueFactory::ReifyValue(document, *value, property);
    if (result.HasException())
    {
      return nullptr;
    }
    return result.ReleaseValue();
  }

  List<Ref<CSSStyleValue>> StylePropertyMapReadOnly::ReifyValueToList(dom::Document &document,
                                                                      RefPtr<Value> value,
                                                                      Maybe<PropertyId> property) noexcept
  {
    List<Ref<CSSStyleValue>> values;
    if (!value)
    {
      return values;
    }
    const auto *list = DynamicDowncast<ValueList>(*value);
    if (list && property && Property::IsListValuedProperty(*property))
    {
      for (const auto &item : *list)
      {
        if (auto reified = ReifyValue(document, ShareRefPtr(const_cast<Value *>(&item)), property))
        {
          values.push_back(Ref<CSSStyleValue>(krys::move(reified)));
        }
      }
      return values;
    }
    if (auto reified = ReifyValue(document, krys::move(value), property))
    {
      values.push_back(Ref<CSSStyleValue>(krys::move(reified)));
    }
    return values;
  }

  dom::ExceptionOr<RefPtr<CSSStyleValue>>
    StylePropertyMapReadOnly::Get(CSSOMStringView property) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-get
    auto *document = Document();
    if (!document)
    {
      return RefPtr<CSSStyleValue>();
    }

    if (IsCustomPropertyName(property))
    {
      return ReifyValue(*document, CustomPropertyValue(CSSOMStringAtom(property)), PropertyId::Custom);
    }

    auto propertyId = PropertyIdFromName(text::ToASCIILower(property));
    if (propertyId == PropertyId::Invalid
        || !IsExposed(propertyId, ParserContext(*document).PropertySettings))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // A shorthand's value is its serialisation.
    if (IsShorthand(propertyId))
    {
      return CSSStyleValueFactory::ConstructStyleValueForShorthandSerialisation(
        *document, ShorthandPropertySerialisation(propertyId));
    }

    return ReifyValue(*document, PropertyValue(propertyId), propertyId);
  }

  dom::ExceptionOr<List<Ref<CSSStyleValue>>>
    StylePropertyMapReadOnly::GetAll(CSSOMStringView property) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-getall
    auto *document = Document();
    if (!document)
    {
      return List<Ref<CSSStyleValue>> {};
    }

    if (IsCustomPropertyName(property))
    {
      return ReifyValueToList(*document, CustomPropertyValue(CSSOMStringAtom(property)), PropertyId::Custom);
    }

    auto propertyId = PropertyIdFromName(text::ToASCIILower(property));
    if (propertyId == PropertyId::Invalid
        || !IsExposed(propertyId, ParserContext(*document).PropertySettings))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    if (IsShorthand(propertyId))
    {
      List<Ref<CSSStyleValue>> values;
      if (auto value = CSSStyleValueFactory::ConstructStyleValueForShorthandSerialisation(
            *document, ShorthandPropertySerialisation(propertyId)))
      {
        values.push_back(Ref<CSSStyleValue>(krys::move(value)));
      }
      return values;
    }

    return ReifyValueToList(*document, PropertyValue(propertyId), propertyId);
  }

  dom::ExceptionOr<bool> StylePropertyMapReadOnly::Has(CSSOMStringView property) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-has
    auto result = Get(property);
    if (result.HasException())
    {
      return result.ReleaseException();
    }
    return static_cast<bool>(result.Value());
  }
}
