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

#include "Krystal.Booey/DOM/Mixins/ARIAMixin.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementFormValue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ValidityStateFlags.hpp"
#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Element;
  class HTMLElement;
  class HTMLFormElement;
  class NodeList;
  class ShadowRoot;
}

namespace krys::boo::html
{
  class CustomStateSet;
  class ElementInternals;
  class FormAssociatedCustomElement;

  namespace detail
  {
    KRYS_NODISCARD inline HTMLElement *GetElement(ElementInternals &internals) noexcept;
  }

  /// @brief What a custom element's behaviour reaches its element through: its closed shadow root, its
  /// custom states, its default ARIA semantics, and, for a form-associated custom element, its form value,
  /// validity and labels.
  /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#elementinternals
  class KRYS_EMPTY_BASE_CLASS ElementInternals : public RefCounted<ElementInternals>,
                                                 public dom::mixins::ARIAMixin<ElementInternals>
  {
    friend HTMLElement *detail::GetElement(ElementInternals &internals) noexcept;

  private:
    WeakPtr<HTMLElement> _target;
    // The specification gives the element its internal content attribute map, but only its internals, of
    // which it has one, read and write it.
    HashMap<DOMStringAtom, DOMString> _internalContentAttributeMap;
    HashMap<DOMStringAtom, WeakPtr<boo::Element>> _explicitlySetAttrElements;
    HashMap<DOMStringAtom, List<WeakPtr<boo::Element>>> _explicitlySetAttrElementLists;

  public:
    explicit ElementInternals(HTMLElement &element) noexcept;

    ~ElementInternals() noexcept;

    KRYS_NODISCARD static Ref<ElementInternals> Create(HTMLElement &element) noexcept
    {
      return CreateRef<ElementInternals>(element);
    }

    /// @brief The element whose internals these are; null once it's gone.
    KRYS_NODISCARD HTMLElement *Element() const noexcept;

    /// @brief The element's shadow root, open or closed, if it was attached while the element was a custom
    /// element.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-elementinternals-shadowroot
    KRYS_NODISCARD RefPtr<boo::ShadowRoot> ShadowRoot() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-elementinternals-form
    KRYS_NODISCARD ExceptionOr<RefPtr<boo::Element>> Form() const noexcept;

    /// @brief Sets the value the element submits with its form, and the state it restores (by default its
    /// value).
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-elementinternals-setformvalue
    ExceptionOr<void> SetFormValue(CustomElementFormValue &&value,
                                   Maybe<CustomElementFormValue> &&state = null) noexcept;

    /// @brief Sets the element's validity: invalid with @p message if any of @p flags is set.
    /// @param validationAnchor Where the validation message points (a shadow-including descendant of the
    /// element); the element if null.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-elementinternals-setvalidity
    ExceptionOr<void> SetValidity(ValidityStateFlags flags = {}, DOMString message = {},
                                  HTMLElement *validationAnchor = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-elementinternals-willvalidate
    KRYS_NODISCARD ExceptionOr<bool> WillValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-elementinternals-validity
    KRYS_NODISCARD ExceptionOr<ValidityState> Validity() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-elementinternals-validationmessage
    KRYS_NODISCARD ExceptionOr<DOMString> ValidationMessage() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-elementinternals-reportvalidity
    ExceptionOr<bool> ReportValidity() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-elementinternals-checkvalidity
    ExceptionOr<bool> CheckValidity() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-elementinternals-labels
    KRYS_NODISCARD ExceptionOr<RefPtr<NodeList>> Labels() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-elementinternals-states
    KRYS_NODISCARD CustomStateSet &States() noexcept;

    /// @brief The value the target element's ARIA defaults give @p name (an ARIA attribute's content
    /// attribute name), if the internals set one.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#internal-content-attribute-map
    KRYS_NODISCARD Maybe<DOMString> InternalContentAttribute(const DOMStringAtom &name) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#internal-content-attribute-map
    void SetInternalContentAttribute(const DOMStringAtom &name, DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#internal-content-attribute-map
    void RemoveInternalContentAttribute(const DOMStringAtom &name) noexcept;

    /// @brief The element an `Element?` ARIA attribute of the internals refers to.
    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#attr-associated-element
    KRYS_NODISCARD RefPtr<boo::Element> ElementForAttributeInternal(const DOMStringAtom &name) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#attr-associated-element
    void SetElementAttribute(const DOMStringAtom &name, boo::Element *element) noexcept;

    /// @brief The elements a `FrozenArray<Element>?` ARIA attribute of the internals refers to.
    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#attr-associated-elements
    KRYS_NODISCARD Maybe<List<Ref<boo::Element>>>
      ElementsForAttributeInternal(const DOMStringAtom &name) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#attr-associated-elements
    void SetElementsArrayAttribute(const DOMStringAtom &name,
                                   const Maybe<List<Ref<boo::Element>>> &elements) noexcept;

  private:
    KRYS_NODISCARD HTMLElement *TargetElement() noexcept;

    KRYS_NODISCARD FormAssociatedCustomElement *ElementAsFormAssociatedCustom() const noexcept;

    /// @brief Tells accessibility the default for @p name changed from @p oldValue, unless the element's own
    /// attribute overrides it.
    void InternalContentAttributeChanged(const DOMStringAtom &name,
                                         const Maybe<DOMString> &oldValue) noexcept;
  };
}
