/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLMetaElement.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  KRYS_NODISCARD static bool IsNameColorScheme(dom::DOMStringView nameValue) noexcept
  {
    return text::IsASCIICaselessEqual(nameValue, u8"color-scheme");
  }

#pragma region HTMLMetaElement

  HTMLMetaElement::HTMLMetaElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Meta)
  {
  }

  dom::DOMString HTMLMetaElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMetaElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLMetaElement::HttpEquiv() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"http-equiv").value_or(u8"");
  }

  void HTMLMetaElement::HttpEquiv(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"http-equiv", krys::move(value));
  }

  dom::DOMString HTMLMetaElement::Content() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"content").value_or(u8"");
  }

  void HTMLMetaElement::Content(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"content", krys::move(value));
  }

  dom::DOMString HTMLMetaElement::Media() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLMetaElement::Media(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

#pragma endregion

#pragma region HTMLMetaElement Obsolete members

  dom::DOMString HTMLMetaElement::Scheme() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"scheme").value_or(u8"");
  }

  void HTMLMetaElement::Scheme(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"scheme", krys::move(value));
  }

#pragma endregion

  void HTMLMetaElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    Process();
  }

  void HTMLMetaElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }
    if (localName == u8"name")
    {
      Process(oldValue);
    }
    else if (localName == u8"http-equiv" || localName == u8"content")
    {
      Process();
    }
  }

  void HTMLMetaElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    auto name = GetAttribute(u8"name");
    if (oldAncestor.IsConnected() && name && IsNameColorScheme(*name))
    {
      oldAncestor.NodeDocument().MetaElementColorSchemeChanged();
    }
  }

  void HTMLMetaElement::Process(Maybe<dom::DOMStringView> oldName) noexcept
  {
    // Changing a meta tag while it's not in the document tree shouldn't have any effect on the document.
    if (IsInDocumentTree())
    {
      auto name = GetAttribute(u8"name");
      if ((name && IsNameColorScheme(*name)) || (oldName && IsNameColorScheme(*oldName)))
      {
        NodeDocument().MetaElementColorSchemeChanged();
      }
    }

    // When a meta element is inserted into the document, if its http-equiv attribute is present and
    // represents one of the pragma directives, run its algorithm.
    if (!IsConnected())
    {
      return;
    }
    auto httpEquiv = GetAttribute(u8"http-equiv");
    auto content = GetAttribute(u8"content");
    if (!httpEquiv || !content)
    {
      return;
    }
    NodeDocument().ProcessMetaHttpEquiv(*httpEquiv, *content);
  }
}
