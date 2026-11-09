/*
 * Copyright (c) 2010 Google Inc. All rights reserved.
 * Copyright (c) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/Elements/HTMLOutputElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::boo::html
{
  HTMLOutputElement::HTMLOutputElement(dom::Document &document) noexcept
      : HTMLFormControlElement(document, HTMLElementInterface::Output)
  {
  }

  HTMLOutputElement::~HTMLOutputElement() noexcept = default;

  // https://html.spec.whatwg.org/#the-output-element:concept-form-reset-control
  void HTMLOutputElement::Reset() noexcept
  {
    SetInteractedWithSinceLastFormSubmitEvent(false);

    // 1. String replace all with this element's default value within this element.
    (void)dom::NodeAlgorithms::StringReplaceAll(DefaultValue(), *this);

    // 2. Set this element's default value override to null.
    _defaultValueOverride = std::nullopt;
  }

#pragma region HTMLOutputElement

  dom::DOMTokenList &HTMLOutputElement::HtmlFor() noexcept
  {
    if (!_htmlFor)
    {
      _htmlFor = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"for");
    }

    return *_htmlFor;
  }

  void HTMLOutputElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"for" && _htmlFor)
    {
      _htmlFor->AssociatedAttributeValueChanged();
    }
  }

  dom::DOMString HTMLOutputElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLOutputElement::Name(dom::DOMString value) noexcept
  {
    Reflect(*this, u8"name", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-output-value
  dom::DOMString HTMLOutputElement::Value() const noexcept
  {
    // Return this's descendant text content.
    return TextContent().value_or(dom::DOMString());
  }

  // https://html.spec.whatwg.org/#dom-output-value
  void HTMLOutputElement::Value(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;

    // 1. Set this's default value override to its default value.
    _defaultValueOverride = DefaultValue();

    // 2. String replace all with the given value within this.
    (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  // https://html.spec.whatwg.org/#concept-output-default-value
  dom::DOMString HTMLOutputElement::DefaultValue() const noexcept
  {
    // 1. If this element's default value override is non-null, then return it.
    // 2. Return this element's descendant text content.
    return _defaultValueOverride ? *_defaultValueOverride : TextContent().value_or(dom::DOMString());
  }

  // https://html.spec.whatwg.org/#dom-output-defaultvalue
  void HTMLOutputElement::DefaultValue(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;

    // 1. If this's default value override is null, then string replace all with the given value within this
    // and
    //    return.
    if (!_defaultValueOverride)
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
    }
    // 2. Set this's default value override to the given value.
    else
    {
      _defaultValueOverride = krys::move(value);
    }
  }

#pragma endregion
}
