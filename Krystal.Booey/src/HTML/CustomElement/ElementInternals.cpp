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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/CustomElement/ElementInternals.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomStateSet.hpp"
#include "Krystal.Booey/HTML/CustomElement/FormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMaybeFormAssociatedCustomElement.hpp"

namespace krys::boo::html
{
  ElementInternals::ElementInternals(HTMLElement &element) noexcept : _target(CreateWeakPtr(&element))
  {
  }

  ElementInternals::~ElementInternals() noexcept = default;

  // https://html.spec.whatwg.org/#dom-elementinternals-shadowroot
  RefPtr<dom::ShadowRoot> ElementInternals::ShadowRoot() const noexcept
  {
    // 1. Let target be this's target element.
    auto *element = _target.get();
    if (!element)
    {
      return nullptr;
    }

    // 2. If target is not a shadow host, then return null.
    // 3. Let shadow be target's shadow root.
    // 4. If shadow's available to element internals is false, then return null.
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*element);
    if (!shadowRoot || !shadowRoot->IsAvailableToElementInternals())
    {
      return nullptr;
    }

    // 5. Return shadow.
    return ShareRefPtr(shadowRoot);
  }

  dom::ExceptionOr<RefPtr<HTMLFormElement>> ElementInternals::Form() const noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->FormForBindings();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
  // 1. Let element be this's target element.
  // 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  // (And so for the other form members.)
  dom::ExceptionOr<void> ElementInternals::SetFormValue(CustomElementFormValue &&value,
                                                        Maybe<CustomElementFormValue> &&state) noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      element->SetFormValue(krys::move(value), krys::move(state));
      return {};
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<void> ElementInternals::SetValidity(ValidityStateFlags flags, dom::DOMString message,
                                                       HTMLElement *validationAnchor) noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->SetValidity(flags, krys::move(message), validationAnchor);
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<bool> ElementInternals::WillValidate() const noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->WillValidate();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<ValidityState> ElementInternals::Validity() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->Validity();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<dom::DOMString> ElementInternals::ValidationMessage() const noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->ValidationMessage();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<bool> ElementInternals::ReportValidity() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->ReportValidity();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<bool> ElementInternals::CheckValidity() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->CheckValidity();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<RefPtr<dom::NodeList>> ElementInternals::Labels() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->AsHTMLElement().Labels();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-states
  // (The states getter steps are to return this's target element's states set.)
  CustomStateSet &ElementInternals::States() noexcept
  {
    return _target->EnsureCustomStateSet();
  }

  FormAssociatedCustomElement *ElementInternals::ElementAsFormAssociatedCustom() const noexcept
  {
    if (auto *element = DynamicDowncast<HTMLMaybeFormAssociatedCustomElement>(_target.get()))
    {
      return element->FormAssociatedCustomElementForElementInternals();
    }
    return nullptr;
  }
}
