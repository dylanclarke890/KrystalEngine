/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web::html
{
  class FormAssociatedCustomElement;
}

namespace krys::web
{
  /// @brief An HTML element whose name is a valid custom element name: a custom element, defined or not yet,
  /// which is form-associated if its definition says so.
  class HTMLMaybeFormAssociatedCustomElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMaybeFormAssociatedCustomElement);

  private:
    UniquePtr<html::FormAssociatedCustomElement> _formAssociatedCustomElement;

  public:
    HTMLMaybeFormAssociatedCustomElement(Document &document, const DOMStringAtom &localName) noexcept;

    ~HTMLMaybeFormAssociatedCustomElement() noexcept override;

    KRYS_NODISCARD bool IsFormListedElement() const noexcept final;

    KRYS_NODISCARD bool IsValidatedFormListedElement() const noexcept final;

    /// @brief Whether the element is a defined custom element whose definition is form-associated.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#form-associated-custom-element
    KRYS_NODISCARD bool IsFormAssociatedCustomElement() const noexcept;

    KRYS_NODISCARD html::FormAssociatedElement *AsFormAssociatedElement() noexcept final;

    KRYS_NODISCARD html::FormListedElement *AsFormListedElement() noexcept final;

    KRYS_NODISCARD html::ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final;

    /// @brief The form-associated half that the element's internals act on: present once the element is
    /// being upgraded (or created) by a form-associated definition.
    KRYS_NODISCARD html::FormAssociatedCustomElement *
      FormAssociatedCustomElementForElementInternals() const noexcept;

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept final;

    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept final;

    KRYS_NODISCARD bool MatchesUserValidPseudoClass() const noexcept final;

    KRYS_NODISCARD bool MatchesUserInvalidPseudoClass() const noexcept final;

    KRYS_NODISCARD bool IsLabelable() const noexcept final;

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final;

    /// @brief Makes the element form-associated, for a form-associated definition.
    void SetInterfaceIsFormAssociated() noexcept;

    KRYS_NODISCARD bool HasFormAssociatedInterface() const noexcept
    {
      return _formAssociatedCustomElement != nullptr;
    }

    void WillUpgradeFormAssociated() noexcept;

    void DidUpgradeFormAssociated() noexcept;

  private:
    void OnInsert() noexcept final;

    void OnPostConnection() noexcept final;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept final;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept final;

    void OnAdopt(Document &oldDocument) noexcept final;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLMaybeFormAssociatedCustomElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement()
           && Downcast<krys::web::HTMLElement>(target).IsHTMLMaybeFormAssociatedCustomElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLMaybeFormAssociatedCustomElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
