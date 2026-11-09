/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Stefan Schimanski (1Stein@gmx.de)
 * Copyright (C) 2004-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
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

#include "Krystal.Booey/HTML/Elements/HTMLObjectElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLObjectElement::HTMLObjectElement(dom::Document &document) noexcept
      : HTMLPlugInElement(document, HTMLElementInterface::Object)
  {
  }

  HTMLObjectElement::~HTMLObjectElement() noexcept
  {
    ClearForm();
  }

  bool HTMLObjectElement::AppendFormData(DOMFormData &) noexcept
  {
    return false;
  }

  void HTMLObjectElement::OnInsert() noexcept
  {
    HTMLPlugInElement::OnInsert();
    FormListedElement::ElementInsertedIntoAncestor(*this, IsConnected());
    if (!IsConnected())
    {
      ResetFormOwner();
    }
  }

  void HTMLObjectElement::OnPostConnection() noexcept
  {
    HTMLPlugInElement::OnPostConnection();
    ResetFormOwner();
  }

  void HTMLObjectElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLPlugInElement::OnRemove(isSubtreeRoot, oldAncestor);
    FormListedElement::ElementRemovedFromAncestor(*this, oldAncestor.IsConnected());
  }

  void HTMLObjectElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLPlugInElement::OnMove(isSubtreeRoot, oldAncestor);
    FormListedElement::ElementRemovedFromAncestor(*this, false);
    FormListedElement::ElementInsertedIntoAncestor(*this, IsConnected());
    ResetFormOwner();
  }

  void HTMLObjectElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLPlugInElement::OnAdopt(oldDocument);
    FormListedElement::DidMoveToNewDocument();
  }

  void HTMLObjectElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLPlugInElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      FormListedElement::ParseAttribute(localName, value);
    }
  }
#pragma region HTMLObjectElement

  dom::USVString HTMLObjectElement::Data() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"data");
  }

  void HTMLObjectElement::Data(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"data", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Type() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLObjectElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLObjectElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Width() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLObjectElement::Width(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Height() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLObjectElement::Height(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

#pragma endregion

#pragma region HTMLObjectElement Obsolete members

  dom::DOMString HTMLObjectElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLObjectElement::Align(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Archive() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"archive").value_or(u8"");
  }

  void HTMLObjectElement::Archive(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"archive", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Code() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"code").value_or(u8"");
  }

  void HTMLObjectElement::Code(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"code", krys::move(value));
  }

  bool HTMLObjectElement::Declare() const noexcept
  {
    return Reflect<bool>(*this, u8"declare");
  }

  void HTMLObjectElement::Declare(bool value) noexcept
  {
    Reflect<bool>(*this, u8"declare", krys::move(value));
  }

  uint32 HTMLObjectElement::HSpace() const noexcept
  {
    return Reflect<uint32>(*this, u8"hspace");
  }

  void HTMLObjectElement::HSpace(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"hspace", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Standby() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"standby").value_or(u8"");
  }

  void HTMLObjectElement::Standby(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"standby", krys::move(value));
  }

  uint32 HTMLObjectElement::VSpace() const noexcept
  {
    return Reflect<uint32>(*this, u8"vspace");
  }

  void HTMLObjectElement::VSpace(uint32 value) noexcept
  {
    Reflect<uint32>(*this, u8"vspace", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::CodeBase() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"codebase").value_or(u8"");
  }

  void HTMLObjectElement::CodeBase(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"codebase", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::CodeType() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"codetype").value_or(u8"");
  }

  void HTMLObjectElement::CodeType(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"codetype", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::UseMap() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLObjectElement::UseMap(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

  dom::DOMString HTMLObjectElement::Border() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLObjectElement::Border(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLObjectElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                             dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"border"))
    {
      return true;
    }
    return HTMLPlugInElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLObjectElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                 dom::DOMStringAtom namespaceURI,
                                                                 dom::DOMStringView value,
                                                                 css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLPlugInElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"border")
    {
      ApplyBorderAttributeToStyle(value, style);
      return;
    }
    HTMLPlugInElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
