/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/StylePropertyMap.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnparsedValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::css
{
  namespace
  {
    /// @brief @p values as one value of @p property: the value itself, or a list of them.
    KRYS_NODISCARD RefPtr<Value> ValueFromStyleValues(PropertyId property,
                                                      const List<Ref<CSSStyleValue>> &values) noexcept
    {
      if (values.empty())
      {
        return nullptr;
      }
      if (values.size() == 1)
      {
        return values[0]->ToCSSValueWithProperty(property);
      }
      ValueListBuilder list;
      for (const auto &value : values)
      {
        if (auto cssValue = value->ToCSSValueWithProperty(property))
        {
          list.push_back(Ref<Value>(krys::move(cssValue)));
        }
      }
      return ValueList::Create(Property::ListValuedPropertySeparator(property), krys::move(list));
    }

    /// @brief The property @p property names, if it's one that's exposed in @p document.
    KRYS_NODISCARD PropertyId ExposedPropertyId(dom::Document &document, CSSOMStringView property) noexcept
    {
      auto propertyId = PropertyIdFromName(text::ToASCIILower(property));
      if (propertyId == PropertyId::Invalid
          || !IsExposed(propertyId, ParserContext(document).PropertySettings))
      {
        return PropertyId::Invalid;
      }
      return propertyId;
    }
  }

  dom::ExceptionOr<void> StylePropertyMap::Set(CSSOMStringView property,
                                               List<CSSStyleValueOrString> values) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-set
    auto *document = Document();
    if (!document)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // A custom property's value is one unparsed value.
    if (IsCustomPropertyName(property))
    {
      auto styleValues =
        CSSStyleValueFactory::ListFromStyleValuesOrStrings(*document, property, krys::move(values));
      if (styleValues.HasException())
      {
        return styleValues.ReleaseException();
      }
      const auto &list = styleValues.Value();
      if (list.size() != 1 || !Is<CSSUnparsedValue>(*list[0]))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      auto value = list[0]->ToCSSValue();
      auto *reference = value ? DynamicDowncast<VariableReferenceValue>(*value) : nullptr;
      if (!reference)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      std::ignore = SetCustomProperty(CSSOMStringAtom(property), ShareRef(*reference));
      return {};
    }

    auto propertyId = ExposedPropertyId(*document, property);
    if (propertyId == PropertyId::Invalid)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    if (!Property::IsListValuedProperty(propertyId) && values.size() > 1)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // A shorthand is set from its one value's serialisation.
    if (IsShorthand(propertyId))
    {
      if (values.size() != 1)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      auto value = krys::SwitchOn(
        values[0], [](const RefPtr<CSSStyleValue> &styleValue) { return styleValue->ToString(); },
        [](const CSSOMString &string) { return string; });
      if (value.empty() || !SetShorthandProperty(propertyId, value))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      return {};
    }

    auto styleValues =
      CSSStyleValueFactory::ListFromStyleValuesOrStrings(*document, property, krys::move(values));
    if (styleValues.HasException())
    {
      return styleValues.ReleaseException();
    }
    const auto &list = styleValues.Value();
    if (list.size() > 1)
    {
      for (const auto &styleValue : list)
      {
        if (Is<CSSUnparsedValue>(*styleValue))
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }
      }
    }

    auto value = ValueFromStyleValues(propertyId, list);
    if (!value)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // The parser may convert numbers to lengths, which the Typed OM doesn't allow.
    if (const auto *primitiveValue = DynamicDowncast<PrimitiveValue>(*value);
        primitiveValue && (primitiveValue->IsNumber() || primitiveValue->IsInteger())
        && !Property::AllowsNumberOrIntegerInput(propertyId))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // A pair or quad of equal values serialises as fewer, so its serialisation wouldn't represent it.
    if (const auto *pair = DynamicDowncast<ValuePair>(*value); pair && pair->CanBeCoalesced())
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }
    if (const auto *quad = DynamicDowncast<QuadValue>(*value); quad && quad->CanBeCoalesced())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    if (!SetProperty(propertyId, Ref<Value>(krys::move(value))))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    return {};
  }

  dom::ExceptionOr<void> StylePropertyMap::Append(CSSOMStringView property,
                                                  List<CSSStyleValueOrString> values) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-append
    if (values.empty())
    {
      return {};
    }

    auto *document = Document();
    if (!document)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    if (IsCustomPropertyName(property))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    auto propertyId = ExposedPropertyId(*document, property);
    if (propertyId == PropertyId::Invalid || !Property::IsListValuedProperty(propertyId))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    auto currentValue = PropertyValue(propertyId);
    ValueListBuilder list;
    if (const auto *currentList = currentValue ? DynamicDowncast<ValueList>(*currentValue) : nullptr)
    {
      list = currentList->CopyValues();
    }
    else if (currentValue)
    {
      list.push_back(Ref<Value>(krys::move(currentValue)));
    }

    auto styleValues =
      CSSStyleValueFactory::ListFromStyleValuesOrStrings(*document, property, krys::move(values));
    if (styleValues.HasException())
    {
      return styleValues.ReleaseException();
    }
    for (const auto &styleValue : styleValues.Value())
    {
      if (Is<CSSUnparsedValue>(*styleValue))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      auto cssValue = styleValue->ToCSSValueWithProperty(propertyId);
      if (!cssValue)
      {
        continue;
      }
      if (Is<VariableReferenceValue>(*cssValue))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      list.push_back(Ref<Value>(krys::move(cssValue)));
    }

    if (!SetProperty(propertyId,
                     ValueList::Create(Property::ListValuedPropertySeparator(propertyId), krys::move(list))))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    return {};
  }

  dom::ExceptionOr<void> StylePropertyMap::Delete(CSSOMStringView property) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-delete
    if (IsCustomPropertyName(property))
    {
      RemoveCustomProperty(CSSOMStringAtom(property));
      return {};
    }

    auto *document = Document();
    auto propertyId =
      document ? ExposedPropertyId(*document, property) : PropertyIdFromName(text::ToASCIILower(property));
    if (propertyId == PropertyId::Invalid)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    RemoveProperty(propertyId);
    return {};
  }
}
