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

#include "Krystal.Booey/CSS/TypedOM/DeclaredStylePropertyMap.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"

namespace krys::boo::css
{
  Ref<DeclaredStylePropertyMap> DeclaredStylePropertyMap::Create(CSSStyleRule &ownerRule) noexcept
  {
    return AdoptRef(*new DeclaredStylePropertyMap(ownerRule));
  }

  DeclaredStylePropertyMap::DeclaredStylePropertyMap(CSSStyleRule &ownerRule) noexcept
      : _ownerRule(CreateWeakPtr(&ownerRule))
  {
  }

  DeclaredStylePropertyMap::~DeclaredStylePropertyMap() noexcept = default;

  StyleRule *DeclaredStylePropertyMap::GetStyleRule() const noexcept
  {
    return _ownerRule ? &_ownerRule->GetStyleRule() : nullptr;
  }

  dom::Document *DeclaredStylePropertyMap::Document() const noexcept
  {
    auto *sheet = _ownerRule ? _ownerRule->ParentStyleSheet() : nullptr;
    return sheet ? sheet->OwnerDocument() : nullptr;
  }

  size_t DeclaredStylePropertyMap::Size() const noexcept
  {
    auto *styleRule = GetStyleRule();
    return styleRule ? styleRule->Properties().PropertyCount() : 0;
  }

  auto DeclaredStylePropertyMap::Entries() const noexcept -> List<Entry>
  {
    List<Entry> entries;
    auto *styleRule = GetStyleRule();
    auto *document = Document();
    if (!styleRule || !document)
    {
      return entries;
    }
    for (auto property : styleRule->Properties())
    {
      entries.push_back(Entry {
        property.CssName(),
        ReifyValueToList(*document, ShareRefPtr(const_cast<Value *>(property.Value())), property.Id())});
    }
    return entries;
  }

  RefPtr<Value> DeclaredStylePropertyMap::PropertyValue(PropertyId property) const noexcept
  {
    auto *styleRule = GetStyleRule();
    return styleRule ? styleRule->Properties().GetPropertyCSSValue(property) : nullptr;
  }

  CSSOMString DeclaredStylePropertyMap::ShorthandPropertySerialisation(PropertyId property) const noexcept
  {
    auto *styleRule = GetStyleRule();
    return styleRule ? styleRule->Properties().GetPropertyValue(property) : CSSOMString();
  }

  RefPtr<Value> DeclaredStylePropertyMap::CustomPropertyValue(const CSSOMStringAtom &property) const noexcept
  {
    auto *styleRule = GetStyleRule();
    return styleRule ? styleRule->Properties().GetCustomPropertyCSSValue(property.View()) : nullptr;
  }

  bool DeclaredStylePropertyMap::SetShorthandProperty(PropertyId property, const CSSOMString &value) noexcept
  {
    if (!_ownerRule)
    {
      return false;
    }
    // The scope may copy the sheet's contents before they change, so the rule is found after it.
    CSSStyleSheet::RuleMutationScope mutationScope(_ownerRule.get());
    auto *styleRule = GetStyleRule();
    bool didFailParsing = false;
    std::ignore =
      styleRule->MutableProperties().SetProperty(property, value, IsImportant(false), &didFailParsing);
    return !didFailParsing;
  }

  bool DeclaredStylePropertyMap::SetProperty(PropertyId property, Ref<Value> value) noexcept
  {
    if (!_ownerRule)
    {
      return false;
    }
    // The scope may copy the sheet's contents before they change, so the rule is found after it.
    CSSStyleSheet::RuleMutationScope mutationScope(_ownerRule.get());
    auto *styleRule = GetStyleRule();
    bool didFailParsing = false;
    std::ignore = styleRule->MutableProperties().SetProperty(
      property, value->CssText(DefaultSerialisationContext()), IsImportant(false), &didFailParsing);
    return !didFailParsing;
  }

  bool DeclaredStylePropertyMap::SetCustomProperty(const CSSOMStringAtom &property,
                                                   Ref<VariableReferenceValue> value) noexcept
  {
    if (!_ownerRule)
    {
      return false;
    }
    // The scope may copy the sheet's contents before they change, so the rule is found after it.
    CSSStyleSheet::RuleMutationScope mutationScope(_ownerRule.get());
    auto *styleRule = GetStyleRule();
    std::ignore = styleRule->MutableProperties().AddParsedProperty(
      Property(PropertyId::Custom, CustomPropertyValue::CreateUnresolved(property, krys::move(value))));
    return true;
  }

  void DeclaredStylePropertyMap::RemoveProperty(PropertyId property) noexcept
  {
    if (!_ownerRule)
    {
      return;
    }
    // The scope may copy the sheet's contents before they change, so the rule is found after it.
    CSSStyleSheet::RuleMutationScope mutationScope(_ownerRule.get());
    auto *styleRule = GetStyleRule();
    std::ignore = styleRule->MutableProperties().RemoveProperty(property);
  }

  void DeclaredStylePropertyMap::RemoveCustomProperty(const CSSOMStringAtom &property) noexcept
  {
    if (!_ownerRule)
    {
      return;
    }
    // The scope may copy the sheet's contents before they change, so the rule is found after it.
    CSSStyleSheet::RuleMutationScope mutationScope(_ownerRule.get());
    auto *styleRule = GetStyleRule();
    std::ignore = styleRule->MutableProperties().RemoveCustomProperty(property.View());
  }

  void DeclaredStylePropertyMap::Clear() noexcept
  {
    if (!_ownerRule)
    {
      return;
    }
    // The scope may copy the sheet's contents before they change, so the rule is found after it.
    CSSStyleSheet::RuleMutationScope mutationScope(_ownerRule.get());
    auto *styleRule = GetStyleRule();
    styleRule->MutableProperties().Clear();
  }
}
