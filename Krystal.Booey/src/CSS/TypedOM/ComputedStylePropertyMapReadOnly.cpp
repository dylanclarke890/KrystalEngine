/*
 * Copyright (C) 2022 Igalia S.L.
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

#include "Krystal.Booey/CSS/TypedOM/ComputedStylePropertyMapReadOnly.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Booey/Style/Extractor/Extractor.hpp"

#include <algorithm>

namespace krys::boo::css
{
  Ref<ComputedStylePropertyMapReadOnly>
    ComputedStylePropertyMapReadOnly::Create(dom::Element &element) noexcept
  {
    return AdoptRef(*new ComputedStylePropertyMapReadOnly(element));
  }

  ComputedStylePropertyMapReadOnly::ComputedStylePropertyMapReadOnly(dom::Element &element) noexcept
      : _element(CreateWeakPtr(&element))
  {
  }

  ComputedStylePropertyMapReadOnly::~ComputedStylePropertyMapReadOnly() noexcept = default;

  dom::Document *ComputedStylePropertyMapReadOnly::Document() const noexcept
  {
    return _element ? &_element->NodeDocument() : nullptr;
  }

  RefPtr<Value> ComputedStylePropertyMapReadOnly::PropertyValue(PropertyId property) const noexcept
  {
    return style::Extractor(_element.get())
      .PropertyValue(property, style::Extractor::UpdateLayout::Yes,
                     style::ExtractorState::PropertyValueType::Computed);
  }

  CSSOMString
    ComputedStylePropertyMapReadOnly::ShorthandPropertySerialisation(PropertyId property) const noexcept
  {
    return style::Extractor(_element.get())
      .PropertyValueSerialisation(property, DefaultSerialisationContext(),
                                  style::Extractor::UpdateLayout::Yes,
                                  style::ExtractorState::PropertyValueType::Computed);
  }

  RefPtr<Value>
    ComputedStylePropertyMapReadOnly::CustomPropertyValue(const CSSOMStringAtom &property) const noexcept
  {
    return style::Extractor(_element.get()).CustomPropertyValue(property);
  }

  size_t ComputedStylePropertyMapReadOnly::Size() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-stylepropertymapreadonly-size
    auto element = ShareRefPtr(_element.get());
    if (!element)
    {
      return 0;
    }
    std::ignore = style::Extractor::UpdateStyleIfNeededForProperty(*element, PropertyId::Custom);
    const auto *style = element->ComputedStyle();
    if (!style)
    {
      return 0;
    }
    return element->NodeDocument().ExposedComputedCSSPropertyIds().size()
           + style->InheritedCustomProperties().Size() + style->NonInheritedCustomProperties().Size();
  }

  auto ComputedStylePropertyMapReadOnly::Entries() const noexcept -> List<Entry>
  {
    // https://drafts.css-houdini.org/css-typed-om/#the-stylepropertymap
    List<Entry> entries;
    auto element = ShareRefPtr(_element.get());
    if (!element)
    {
      return entries;
    }

    // Brings the custom properties up to date.
    std::ignore = style::Extractor::UpdateStyleIfNeededForProperty(*element, PropertyId::Custom);
    const auto *style = element->ComputedStyle();
    if (!style)
    {
      return entries;
    }

    auto &document = element->NodeDocument();
    const auto &exposedProperties = document.ExposedComputedCSSPropertyIds();
    const auto &inheritedCustomProperties = style->InheritedCustomProperties();
    const auto &nonInheritedCustomProperties = style->NonInheritedCustomProperties();
    entries.reserve(exposedProperties.size() + inheritedCustomProperties.Size()
                    + nonInheritedCustomProperties.Size());

    style::Extractor extractor(element.get());
    for (auto property : exposedProperties)
    {
      auto value = extractor.PropertyValue(property, style::Extractor::UpdateLayout::No,
                                           style::ExtractorState::PropertyValueType::Computed);
      entries.push_back(
        Entry {CSSOMString(ToString(property)), ReifyValueToList(document, krys::move(value), property)});
    }
    for (const auto *customProperties : {&nonInheritedCustomProperties, &inheritedCustomProperties})
    {
      customProperties->ForEach(
        [&](const auto &entry)
        {
          entries.push_back(
            Entry {CSSOMString(entry.first.View()),
                   ReifyValueToList(document, extractor.CustomPropertyValue(entry.first), {})});
          return IterationStatus::Continue;
        });
    }

    // Custom properties sort after the others, and vendor-prefixed properties (starting with `-`) after the
    // standard ones, each in code point order.
    std::ranges::sort(entries,
                      [](const Entry &a, const Entry &b)
                      {
                        const auto &nameA = a.Property;
                        const auto &nameB = b.Property;
                        if (nameA.starts_with(u8"--"))
                        {
                          return nameB.starts_with(u8"--") && nameA < nameB;
                        }
                        if (nameA.starts_with(u8'-'))
                        {
                          return nameB.starts_with(u8"--") || (nameB.starts_with(u8'-') && nameA < nameB);
                        }
                        return nameB.starts_with(u8'-') || nameA < nameB;
                      });
    return entries;
  }
}
