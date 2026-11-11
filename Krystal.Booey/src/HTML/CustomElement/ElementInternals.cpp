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
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

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

  Maybe<dom::DOMString> ElementInternals::InternalContentAttribute(dom::DOMStringAtom name) const noexcept
  {
    if (auto it = _internalContentAttributeMap.find(name); it != _internalContentAttributeMap.end())
    {
      return it->second;
    }
    return null;
  }

  void ElementInternals::SetInternalContentAttribute(dom::DOMStringAtom name, dom::DOMString value) noexcept
  {
    _internalContentAttributeMap[name] = krys::move(value);
    // FIXME: Telling the accessibility cache the default changed, with accessibility.
  }

  void ElementInternals::RemoveInternalContentAttribute(dom::DOMStringAtom name) noexcept
  {
    _internalContentAttributeMap.erase(name);
    // FIXME: Telling the accessibility cache the default changed, with accessibility.
  }

  RefPtr<dom::Element> ElementInternals::ElementForAttributeInternal(dom::DOMStringAtom name) const noexcept
  {
    // 1. Let element be the result of running reflectedTarget's get the element.
    auto *element = _target.get();
    if (!element)
    {
      return nullptr;
    }

    // 2. Let contentAttributeValue be the result of running reflectedTarget's get the content attribute.
    auto contentAttributeValue = InternalContentAttribute(name);

    // 3. If reflectedTarget's explicitly set attr-element is not null:
    if (auto it = _explicitlySetAttrElements.find(name); it != _explicitlySetAttrElements.end())
    {
      // 3a. If reflectedTarget's explicitly set attr-element is a descendant of any of element's
      //     shadow-including ancestors, then return reflectedTarget's explicitly set attr-element.
      auto *attrElement = it->second.get();
      if (attrElement
          && dom::TreeQueries::IsShadowIncludingDescendant(*element, dom::TreeQueries::Root(*attrElement)))
      {
        return ShareRefPtr(attrElement);
      }

      // 3b. Return null.
      return nullptr;
    }

    // 4. Otherwise, if contentAttributeValue is not null, return the first element candidate, in tree order,
    //    that meets the following criteria: candidate's root is the same as element's root; candidate's ID is
    //    contentAttributeValue; and candidate implements T. If no such element exists, then return null.
    if (contentAttributeValue)
    {
      return element->ElementInTreeWithId(*contentAttributeValue);
    }

    // 5. Return null.
    return nullptr;
  }

  void ElementInternals::SetElementAttribute(dom::DOMStringAtom name, dom::Element *element) noexcept
  {
    // 1. If the given value is null:
    if (!element)
    {
      // 1a. Set this's explicitly set attr-element to null.
      _explicitlySetAttrElements.erase(name);
      // 1b. Run this's delete the content attribute.
      RemoveInternalContentAttribute(name);
      // 1c. Return.
      return;
    }

    // 2. Run this's set the content attribute with the empty string.
    SetInternalContentAttribute(name, dom::DOMString());

    // 3. Set this's explicitly set attr-element to a weak reference to the given value.
    _explicitlySetAttrElements[name] = CreateWeakPtr(element);
  }

  Maybe<List<Ref<dom::Element>>>
    ElementInternals::ElementsForAttributeInternal(dom::DOMStringAtom name) const noexcept
  {
    // 1. Let elements be an empty list.
    List<Ref<dom::Element>> elements;

    // 2. Let element be the result of running reflectedTarget's get the element.
    auto *element = _target.get();
    if (!element)
    {
      return null;
    }

    // 3. If reflectedTarget's explicitly set attr-elements is not null:
    if (auto it = _explicitlySetAttrElementLists.find(name); it != _explicitlySetAttrElementLists.end())
    {
      // 3a. For each attrElement in reflectedTarget's explicitly set attr-elements:
      for (auto &weakElement : it->second)
      {
        // 3a.i. If attrElement is not a descendant of any of element's shadow-including ancestors, then
        //       continue.
        auto *attrElement = weakElement.get();
        if (!attrElement
            || !dom::TreeQueries::IsShadowIncludingDescendant(*element, dom::TreeQueries::Root(*attrElement)))
        {
          continue;
        }

        // 3a.ii. Append attrElement to elements.
        elements.push_back(ShareRef(*attrElement));
      }

      // 5. Return elements.
      return elements;
    }

    // 4. Otherwise:
    // 4a. Let contentAttributeValue be the result of running reflectedTarget's get the content attribute.
    auto contentAttributeValue = InternalContentAttribute(name);

    // 4b. If contentAttributeValue is null, then return null.
    if (!contentAttributeValue)
    {
      return null;
    }

    // 4c. Let tokens be contentAttributeValue, split on ASCII whitespace.
    // 4d. For each id of tokens:
    for (const auto &id : infra::StringAlgorithms::SplitOnWhitespace(*contentAttributeValue))
    {
      // 4d.i. Let candidate be the first element, in tree order, that meets the following criteria:
      //       candidate's root is the same as element's root; candidate's ID is id; and candidate implements
      //       T. If no such element exists, then continue.
      // 4d.ii. Append candidate to elements.
      if (auto candidate = element->ElementInTreeWithId(id))
      {
        elements.push_back(ShareRef(*candidate));
      }
    }

    // 5. Return elements.
    return elements;
  }

  void ElementInternals::SetElementsArrayAttribute(dom::DOMStringAtom name,
                                                   const Maybe<List<Ref<dom::Element>>> &elements) noexcept
  {
    // 1. If the given value is null:
    if (!elements)
    {
      // 1a. Set this's explicitly set attr-elements to null.
      _explicitlySetAttrElementLists.erase(name);
      // 1b. Run this's delete the content attribute.
      RemoveInternalContentAttribute(name);
      // 1c. Return.
      return;
    }

    // 2. Run this's set the content attribute with the empty string.
    SetInternalContentAttribute(name, dom::DOMString());

    // 3. Let elements be an empty list.
    // 4. For each element in the given value:
    // 4a. Append a weak reference to element to elements.
    List<WeakPtr<dom::Element>> weakElements;
    weakElements.reserve(elements->size());
    for (auto &element : *elements)
    {
      weakElements.push_back(CreateWeakPtr(element.get()));
    }

    // 5. Set this's explicitly set attr-elements to elements.
    _explicitlySetAttrElementLists[name] = krys::move(weakElements);
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
