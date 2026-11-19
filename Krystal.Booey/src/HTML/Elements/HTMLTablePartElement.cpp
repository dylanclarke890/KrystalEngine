/**
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2020 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/HTML/Elements/HTMLTablePartElement.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  HTMLTablePartElement::HTMLTablePartElement(Document &document,
                                             html::HTMLElementInterface interface) noexcept
      : HTMLElement(document, interface)
  {
  }

  const HTMLTableElement *HTMLTablePartElement::FindParentTable() const noexcept
  {
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      auto *htmlElement = DynamicDowncast<HTMLElement>(*ancestor);
      if (auto *table = htmlElement ? DynamicDowncast<HTMLTableElement>(*htmlElement) : nullptr)
      {
        return table;
      }
    }
    return nullptr;
  }

  bool
    HTMLTablePartElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"bgcolor" || localName == u8"background" || localName == u8"valign"
            || localName == u8"height"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTablePartElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"bgcolor")
    {
      AddHTMLColorToStyle(style, css::PropertyId::BackgroundColor, value);
      return;
    }
    if (localName == u8"background")
    {
      if (!infra::StringAlgorithms::StripLeadingAndTrailingWhitespace(value).empty())
      {
        style.SetProperty(css::PropertyId::BackgroundImage,
                          css::ImageValue::Create(NodeDocument().CompleteUrl(value)));
      }
      return;
    }
    if (localName == u8"valign")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"top"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Top);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Middle);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"bottom"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Bottom);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"baseline"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, css::ValueId::Baseline);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::VerticalAlign, ToCSSOMString(value));
      }
      return;
    }
    if (localName == u8"align")
    {
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"middle")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"center"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::InternalCenter);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"absmiddle"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::Center);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"left"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::InternalLeft);
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"right"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::InternalRight);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, ToCSSOMString(value));
      }
      return;
    }
    if (localName == u8"height")
    {
      if (!value.empty())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}

namespace krys::boo::html::detail
{
  Ref<HTMLElement> CreateTableElement(Document &document, DOMStringView localName) noexcept
  {
    auto element = document.CreateElementNS(infra::namespaces::HTML, DOMStringAtom(localName));
    krys_debug_assert(!element.HasException());
    return ShareRef(Downcast<HTMLElement>(*element.ReleaseValue()));
  }
}
