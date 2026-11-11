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

#pragma once

#include "Krystal.Booey/Accessibility/ARIAMixin.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementFormValue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ValidityStateFlags.hpp"
#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Element;
  class NodeList;
  class ShadowRoot;
}

namespace krys::boo::html
{
  class CustomStateSet;
  class FormAssociatedCustomElement;
  class HTMLElement;
  class HTMLFormElement;
  class ElementInternals;

  namespace detail
  {
    KRYS_NODISCARD inline HTMLElement *GetElement(ElementInternals &internals) noexcept;
  }

  /// @brief What a custom element's behaviour reaches its element through: its closed shadow root, its
  /// custom states, its default ARIA semantics, and, for a form-associated custom element, its form value,
  /// validity and labels.
  /// @see https://html.spec.whatwg.org/#elementinternals
  class KRYS_EMPTY_BASE_CLASS ElementInternals : public RefCounted<ElementInternals>,
                                                 public aria::mixins::ARIAMixin<ElementInternals>
  {
    friend HTMLElement *detail::GetElement(ElementInternals &internals) noexcept;

  private:
    WeakPtr<HTMLElement> _target;
    // The specification gives the element its internal content attribute map, but only its internals, of
    // which it has one, read and write it.
    HashMap<dom::DOMStringAtom, dom::DOMString> _internalContentAttributeMap;
    HashMap<dom::DOMStringAtom, WeakPtr<dom::Element>> _explicitlySetAttrElements;
    HashMap<dom::DOMStringAtom, List<WeakPtr<dom::Element>>> _explicitlySetAttrElementLists;

  public:
    explicit ElementInternals(HTMLElement &element) noexcept;
    ~ElementInternals() noexcept;

    KRYS_NODISCARD static Ref<ElementInternals> Create(HTMLElement &element) noexcept
    {
      return CreateRef<ElementInternals>(element);
    }

    /// @brief The element whose internals these are; null once it's gone.
    KRYS_NODISCARD HTMLElement *Element() const noexcept
    {
      return _target.get();
    }

    /// @brief The element's shadow root, open or closed, if it was attached while the element was a custom
    /// element.
    /// @see https://html.spec.whatwg.org/#dom-elementinternals-shadowroot
    KRYS_NODISCARD RefPtr<dom::ShadowRoot> ShadowRoot() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-form
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<HTMLFormElement>> Form() const noexcept;

    /// @brief Sets the value the element submits with its form, and the state it restores (by default its
    /// value).
    /// @see https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
    dom::ExceptionOr<void> SetFormValue(CustomElementFormValue &&value,
                                        Maybe<CustomElementFormValue> &&state = null) noexcept;

    /// @brief Sets the element's validity: invalid with @p message if any of @p flags is set.
    /// @param validationAnchor Where the validation message points (a shadow-including descendant of the
    /// element); the element if null.
    /// @see https://html.spec.whatwg.org/#dom-elementinternals-setvalidity
    dom::ExceptionOr<void> SetValidity(ValidityStateFlags flags = {}, dom::DOMString message = {},
                                       HTMLElement *validationAnchor = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-willvalidate
    KRYS_NODISCARD dom::ExceptionOr<bool> WillValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-validity
    KRYS_NODISCARD dom::ExceptionOr<ValidityState> Validity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-validationmessage
    KRYS_NODISCARD dom::ExceptionOr<dom::DOMString> ValidationMessage() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-reportvalidity
    dom::ExceptionOr<bool> ReportValidity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-checkvalidity
    dom::ExceptionOr<bool> CheckValidity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-labels
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<dom::NodeList>> Labels() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-states
    KRYS_NODISCARD CustomStateSet &States() noexcept;

    /// @brief The value the target element's ARIA defaults give @p name (an ARIA attribute's content
    /// attribute name), if the internals set one.
    /// @see https://html.spec.whatwg.org/#internal-content-attribute-map
    KRYS_NODISCARD Maybe<dom::DOMString> InternalContentAttribute(dom::DOMStringAtom name) const noexcept;

    /// @see https://html.spec.whatwg.org/#internal-content-attribute-map
    void SetInternalContentAttribute(dom::DOMStringAtom name, dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#internal-content-attribute-map
    void RemoveInternalContentAttribute(dom::DOMStringAtom name) noexcept;

    /// @brief The element an `Element?` ARIA attribute of the internals refers to.
    /// @see https://html.spec.whatwg.org/#attr-associated-element
    KRYS_NODISCARD RefPtr<dom::Element> ElementForAttributeInternal(dom::DOMStringAtom name) const noexcept;

    /// @see https://html.spec.whatwg.org/#attr-associated-element
    void SetElementAttribute(dom::DOMStringAtom name, dom::Element *element) noexcept;

    /// @brief The elements a `FrozenArray<Element>?` ARIA attribute of the internals refers to.
    /// @see https://html.spec.whatwg.org/#attr-associated-elements
    KRYS_NODISCARD Maybe<List<Ref<dom::Element>>>
      ElementsForAttributeInternal(dom::DOMStringAtom name) const noexcept;

    /// @see https://html.spec.whatwg.org/#attr-associated-elements
    void SetElementsArrayAttribute(dom::DOMStringAtom name,
                                   const Maybe<List<Ref<dom::Element>>> &elements) noexcept;

  private:
    KRYS_NODISCARD HTMLElement *TargetElement() noexcept
    {
      return _target.get();
    }

    KRYS_NODISCARD FormAssociatedCustomElement *ElementAsFormAssociatedCustom() const noexcept;
    /// @brief Tells accessibility the default for @p name changed from @p oldValue, unless the element's own
    /// attribute overrides it.
    void InternalContentAttributeChanged(dom::DOMStringAtom name,
                                         const Maybe<dom::DOMString> &oldValue) noexcept;
  };
}
