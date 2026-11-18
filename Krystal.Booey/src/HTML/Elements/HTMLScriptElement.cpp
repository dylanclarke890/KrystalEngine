/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/HTML/Elements/HTMLScriptElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo
{
  HTMLScriptElement::HTMLScriptElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Script)
  {
  }

#pragma region HTMLScriptElement

  DOMString HTMLScriptElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLScriptElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  dom::USVString HTMLScriptElement::Src() const noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLScriptElement::Src(dom::USVString value) noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  bool HTMLScriptElement::NoModule() const noexcept
  {
    return html::Reflect<bool>(*this, u8"nomodule");
  }

  void HTMLScriptElement::NoModule(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"nomodule", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-script-async
  bool HTMLScriptElement::Async() const noexcept
  {
    // 1. If this's force async is true, then return true.
    // 2. If this's async content attribute is present, then return true.
    // 3. Return false.
    return _forceAsync || html::Reflect<bool>(*this, u8"async");
  }

  // https://html.spec.whatwg.org/#dom-script-async
  void HTMLScriptElement::Async(bool value) noexcept
  {
    // 1. Set this's force async to false.
    _forceAsync = false;

    // 2. If the given value is true, then set this's async content attribute to the empty string.
    // 3. Otherwise, remove this's async content attribute.
    html::Reflect<bool>(*this, u8"async", krys::move(value));
  }

  bool HTMLScriptElement::Defer() const noexcept
  {
    return html::Reflect<bool>(*this, u8"defer");
  }

  void HTMLScriptElement::Defer(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"defer", krys::move(value));
  }

  dom::DOMTokenList &HTMLScriptElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking =
        dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking", [](Document &, DOMStringView token)
                                                   { return text::IsASCIICaselessEqual(token, u8"render"); });
    }

    return *_blocking;
  }

  void HTMLScriptElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                             Maybe<DOMStringView> value,
                                             const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }
    if (localName == u8"blocking" && _blocking)
    {
      _blocking->AssociatedAttributeValueChanged();
    }
    // When an async attribute is added to a script element el, the user agent must set el's force async to
    // false.
    else if (localName == u8"async" && value && !oldValue)
    {
      _forceAsync = false;
    }
  }

  // https://html.spec.whatwg.org/#the-script-element:concept-node-clone-ext
  void HTMLScriptElement::OnClone(Node &copy, bool subtree) noexcept
  {
    HTMLElement::OnClone(copy, subtree);

    // The cloning steps for script elements given node, copy, and subtree are to set copy's already started
    // to node's already started.
    Downcast<HTMLScriptElement>(copy)._alreadyStarted = _alreadyStarted;
  }

  Maybe<DOMString> HTMLScriptElement::CrossOrigin() const noexcept
  {
    return html::ReflectEnumerated<"crossorigin", HTMLElement, Maybe<DOMString>>(*this);
  }

  void HTMLScriptElement::CrossOrigin(Maybe<DOMString> value) noexcept
  {
    html::Reflect<Maybe<DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  DOMString HTMLScriptElement::ReferrerPolicy() const noexcept
  {
    return html::ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLScriptElement::ReferrerPolicy(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  DOMString HTMLScriptElement::Integrity() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLScriptElement::Integrity(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"integrity", krys::move(value));
  }

  DOMString HTMLScriptElement::FetchPriority() const noexcept
  {
    return html::ReflectEnumerated<"fetchpriority">(*this);
  }

  void HTMLScriptElement::FetchPriority(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-script-text
  DOMString HTMLScriptElement::Text() const noexcept
  {
    // The text getter steps are to return this's child text content.
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  // https://html.spec.whatwg.org/#dom-script-text
  void HTMLScriptElement::Text(DOMString value) noexcept
  {
    // 1. (Skipped: there are no Trusted Types.)
    // 2. (Skipped: there are no Trusted Types.)
    // 3. String replace all with compliantScript within this.
    (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  // https://html.spec.whatwg.org/#dom-script-supports
  bool HTMLScriptElement::Supports(DOMStringView type) noexcept
  {
    // 1. If type is "classic", then return true.
    // 2. If type is "module", then return true.
    // 3. If type is "importmap", then return true.
    // 4. If type is "speculationrules", then return true.
    // 5. Return false.
    return type == u8"classic" || type == u8"module" || type == u8"importmap" || type == u8"speculationrules";
  }

#pragma endregion

#pragma region HTMLScriptElement Obsolete members

  DOMString HTMLScriptElement::Charset() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLScriptElement::Charset(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charset", krys::move(value));
  }

  DOMString HTMLScriptElement::Event() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"event").value_or(u8"");
  }

  void HTMLScriptElement::Event(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"event", krys::move(value));
  }

  DOMString HTMLScriptElement::HtmlFor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLScriptElement::HtmlFor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"for", krys::move(value));
  }

#pragma endregion
}
