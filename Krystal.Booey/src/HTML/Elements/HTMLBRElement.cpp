/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2003, 2006, 2009, 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  HTMLBRElement::HTMLBRElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::BR)
  {
  }

  RenderPtr<RenderElement> HTMLBRElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    if (RenderElement::IsContentDataSupported(style.GetContent()))
    {
      return RenderElement::CreateFor(*this, krys::move(style));
    }

    return CreateRenderer<RenderLineBreak>(*this, krys::move(style));
  }

#pragma region HTMLBRElement Obsolete members

  DOMString HTMLBRElement::Clear() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"clear").value_or(u8"");
  }

  void HTMLBRElement::Clear(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"clear", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLBRElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                         const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"clear"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLBRElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI,
                                                             DOMStringView value,
                                                             css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"clear")
    {
      // If the string is empty, then don't add the clear property.
      // <br clear> and <br clear=""> are just treated like <br> by Gecko, Mac IE, etc. -dwh
      if (!value.empty())
      {
        if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"all"))
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::Clear, css::ValueId::Both);
        }
        else
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::Clear, ToCSSOMString(value));
        }
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
