/*
 * Copyright (C) 2021-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Css/TypedOM/InlineStylePropertyMap.hpp"
#include "Krystal.Booey/Css/CSSOM/StyleAttributeMutationScope.hpp"
#include "Krystal.Booey/Css/Properties/StyleProperties.hpp"
#include "Krystal.Booey/Css/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/Css/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/Css/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Css/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/StyledElement.hpp"

namespace krys::boo
{
  Ref<InlineStylePropertyMap> InlineStylePropertyMap::Create(StyledElement &element) noexcept
  {
    return AdoptRef(*new InlineStylePropertyMap(element));
  }

  InlineStylePropertyMap::InlineStylePropertyMap(StyledElement &element) noexcept
      : _element(CreateWeakPtr(&element))
  {
  }

  InlineStylePropertyMap::~InlineStylePropertyMap() noexcept = default;

  boo::Document *InlineStylePropertyMap::Document() const noexcept
  {
    return _element ? &_element->NodeDocument() : nullptr;
  }

  RefPtr<css::Value> InlineStylePropertyMap::PropertyValue(css::PropertyId property) const noexcept
  {
    const auto *inlineStyle = _element ? _element->InlineStyle() : nullptr;
    return inlineStyle ? inlineStyle->GetPropertyCSSValue(property) : nullptr;
  }

  CSSOMString InlineStylePropertyMap::ShorthandPropertySerialisation(css::PropertyId property) const noexcept
  {
    const auto *inlineStyle = _element ? _element->InlineStyle() : nullptr;
    return inlineStyle ? inlineStyle->GetPropertyValue(property) : CSSOMString();
  }

  RefPtr<css::Value>
    InlineStylePropertyMap::CustomPropertyValue(const CSSOMStringAtom &property) const noexcept
  {
    const auto *inlineStyle = _element ? _element->InlineStyle() : nullptr;
    return inlineStyle ? inlineStyle->GetCustomPropertyCSSValue(property.View()) : nullptr;
  }

  size_t InlineStylePropertyMap::Size() const noexcept
  {
    const auto *inlineStyle = _element ? _element->InlineStyle() : nullptr;
    return inlineStyle ? inlineStyle->size() : 0;
  }

  auto InlineStylePropertyMap::Entries() const noexcept -> List<Entry>
  {
    List<Entry> entries;
    const auto *inlineStyle = _element ? _element->InlineStyle() : nullptr;
    if (!inlineStyle)
    {
      return entries;
    }
    auto &document = _element->NodeDocument();
    for (auto property : *inlineStyle)
    {
      entries.push_back(Entry {
        property.CssName(),
        ReifyValueToList(document, ShareRefPtr(const_cast<css::Value *>(property.Value())), property.Id())});
    }
    return entries;
  }

  void InlineStylePropertyMap::RemoveProperty(css::PropertyId property) noexcept
  {
    if (!_element)
    {
      return;
    }
    StyleAttributeMutationScope mutationScope(_element.get());
    if (_element->RemoveInlineStyleProperty(property))
    {
      mutationScope.EnqueueMutationRecord();
    }
  }

  bool InlineStylePropertyMap::SetShorthandProperty(css::PropertyId property,
                                                    const CSSOMString &value) noexcept
  {
    if (!_element)
    {
      return false;
    }
    StyleAttributeMutationScope mutationScope(_element.get());
    bool didFailParsing = false;
    std::ignore = _element->SetInlineStyleProperty(property, value, css::IsImportant::No, &didFailParsing);
    if (!didFailParsing)
    {
      mutationScope.EnqueueMutationRecord();
    }
    return !didFailParsing;
  }

  bool InlineStylePropertyMap::SetProperty(css::PropertyId property, Ref<css::Value> value) noexcept
  {
    if (!_element)
    {
      return false;
    }
    StyleAttributeMutationScope mutationScope(_element.get());
    bool didFailParsing = false;
    // FIXME: Validating the value without serialising it and parsing the text.
    std::ignore = _element->SetInlineStyleProperty(
      property, value->CssText(css::DefaultSerialisationContext()), css::IsImportant::No, &didFailParsing);
    if (!didFailParsing)
    {
      std::ignore = _element->SetInlineStyleProperty(property, krys::move(value));
      mutationScope.EnqueueMutationRecord();
    }
    return !didFailParsing;
  }

  bool InlineStylePropertyMap::SetCustomProperty(const CSSOMStringAtom &property,
                                                 Ref<css::VariableReferenceValue> value) noexcept
  {
    if (!_element)
    {
      return false;
    }
    StyleAttributeMutationScope mutationScope(_element.get());
    if (_element->SetInlineStyleCustomProperty(
          css::CustomPropertyValue::CreateUnresolved(property, krys::move(value))))
    {
      mutationScope.EnqueueMutationRecord();
    }
    return true;
  }

  void InlineStylePropertyMap::RemoveCustomProperty(const CSSOMStringAtom &property) noexcept
  {
    if (!_element)
    {
      return;
    }
    StyleAttributeMutationScope mutationScope(_element.get());
    if (_element->RemoveInlineStyleCustomProperty(property))
    {
      mutationScope.EnqueueMutationRecord();
    }
  }

  void InlineStylePropertyMap::Clear() noexcept
  {
    if (!_element)
    {
      return;
    }
    StyleAttributeMutationScope mutationScope(_element.get());
    _element->RemoveAllInlineStyleProperties();
    mutationScope.EnqueueMutationRecord();
  }
}
