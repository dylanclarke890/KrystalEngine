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

#include "Krystal.Booey/Css/TypedOM/StylePropertyMap.hpp"
#include "Krystal.Booey/Css/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Properties/Property.hpp"
#include "Krystal.Booey/Css/Properties/PropertyQueries.hpp"
#include "Krystal.Booey/Css/TypedOM/CSSUnparsedValue.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Css/Values/QuadValue.hpp"
#include "Krystal.Booey/Css/Values/ValueList.hpp"
#include "Krystal.Booey/Css/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/Css/Values/ValuePair.hpp"
#include "Krystal.Booey/Css/Values/VariableReferenceValue.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief @p values as one value of @p property: the value itself, or a list of them.
    KRYS_NODISCARD RefPtr<css::Value> ValueFromStyleValues(css::PropertyId property,
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
      auto separator = css::ListValuedPropertySeparator(property);
      if (!separator)
      {
        return nullptr;
      }

      css::ValueListBuilder list;
      for (const auto &value : values)
      {
        if (auto cssValue = value->ToCSSValueWithProperty(property))
        {
          list.push_back(Ref<css::Value>(krys::move(cssValue)));
        }
      }
      return css::ValueList::Create(*separator, krys::move(list));
    }

    /// @brief The property @p property names, if it's one that's exposed in @p document.
    KRYS_NODISCARD css::PropertyId ExposedPropertyId(boo::Document &document,
                                                     CSSOMStringView property) noexcept
    {
      auto propertyId = css::PropertyIdFromName(text::ToASCIILower(property));
      if (propertyId == css::PropertyId::Invalid
          || !IsExposed(propertyId, css::ParserContext(document).PropertySettings))
      {
        return css::PropertyId::Invalid;
      }
      return propertyId;
    }
  }

  ExceptionOr<void> StylePropertyMap::Set(CSSOMStringView property,
                                          List<CSSStyleValueOrString> values) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-set
    auto *document = Document();
    if (!document)
    {
      return Exception {ExceptionCode::TypeError};
    }

    // A custom property's value is one unparsed value.
    if (css::IsCustomPropertyName(property))
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
        return Exception {ExceptionCode::TypeError};
      }
      auto value = list[0]->ToCSSValue();
      auto *reference = value ? DynamicDowncast<css::VariableReferenceValue>(*value) : nullptr;
      if (!reference)
      {
        return Exception {ExceptionCode::TypeError};
      }
      std::ignore = SetCustomProperty(CSSOMStringAtom(property), ShareRef(*reference));
      return {};
    }

    auto propertyId = ExposedPropertyId(*document, property);
    if (propertyId == css::PropertyId::Invalid)
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (!css::IsListValuedProperty(propertyId) && values.size() > 1)
    {
      return Exception {ExceptionCode::TypeError};
    }

    // A shorthand is set from its one value's serialisation.
    if (IsShorthand(propertyId))
    {
      if (values.size() != 1)
      {
        return Exception {ExceptionCode::TypeError};
      }
      auto value = krys::SwitchOn(
        values[0], [](const RefPtr<CSSStyleValue> &styleValue) { return styleValue->ToString(); },
        [](const CSSOMString &string) { return string; });
      if (value.empty() || !SetShorthandProperty(propertyId, value))
      {
        return Exception {ExceptionCode::TypeError};
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
          return Exception {ExceptionCode::TypeError};
        }
      }
    }

    auto value = ValueFromStyleValues(propertyId, list);
    if (!value)
    {
      return Exception {ExceptionCode::TypeError};
    }

    // The parser may convert numbers to lengths, which the Typed OM doesn't allow.
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(*value);
        primitiveValue && (primitiveValue->IsNumber() || primitiveValue->IsInteger())
        && !css::AllowsNumberOrIntegerInput(propertyId))
    {
      return Exception {ExceptionCode::TypeError};
    }

    // A pair or quad of equal values serialises as fewer, so its serialisation wouldn't represent it.
    if (const auto *pair = DynamicDowncast<css::ValuePair>(*value); pair && pair->CanBeCoalesced())
    {
      return Exception {ExceptionCode::NotSupportedError};
    }
    if (const auto *quad = DynamicDowncast<css::QuadValue>(*value); quad && quad->CanBeCoalesced())
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (!SetProperty(propertyId, Ref<css::Value>(krys::move(value))))
    {
      return Exception {ExceptionCode::TypeError};
    }
    return {};
  }

  ExceptionOr<void> StylePropertyMap::Append(CSSOMStringView property,
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
      return Exception {ExceptionCode::TypeError};
    }

    if (css::IsCustomPropertyName(property))
    {
      return Exception {ExceptionCode::TypeError};
    }
    auto propertyId = ExposedPropertyId(*document, property);
    if (propertyId == css::PropertyId::Invalid || !css::IsListValuedProperty(propertyId))
    {
      return Exception {ExceptionCode::TypeError};
    }

    auto currentValue = PropertyValue(propertyId);
    css::ValueListBuilder list;
    if (const auto *currentList = currentValue ? DynamicDowncast<css::ValueList>(*currentValue) : nullptr)
    {
      list = currentList->CopyValues();
    }
    else if (currentValue)
    {
      list.push_back(Ref<css::Value>(krys::move(currentValue)));
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
        return Exception {ExceptionCode::TypeError};
      }
      auto cssValue = styleValue->ToCSSValueWithProperty(propertyId);
      if (!cssValue)
      {
        continue;
      }
      if (Is<css::VariableReferenceValue>(*cssValue))
      {
        return Exception {ExceptionCode::TypeError};
      }
      list.push_back(Ref<css::Value>(krys::move(cssValue)));
    }

    // IsListValuedProperty was checked above, and custom properties were turned away.
    auto separator = *css::ListValuedPropertySeparator(propertyId);
    if (!SetProperty(propertyId, css::ValueList::Create(separator, krys::move(list))))
    {
      return Exception {ExceptionCode::TypeError};
    }
    return {};
  }

  ExceptionOr<void> StylePropertyMap::Delete(CSSOMStringView property) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymap-delete
    if (css::IsCustomPropertyName(property))
    {
      RemoveCustomProperty(CSSOMStringAtom(property));
      return {};
    }

    auto *document = Document();
    auto propertyId = document ? ExposedPropertyId(*document, property)
                               : css::PropertyIdFromName(text::ToASCIILower(property));
    if (propertyId == css::PropertyId::Invalid)
    {
      return Exception {ExceptionCode::TypeError};
    }
    RemoveProperty(propertyId);
    return {};
  }
}
