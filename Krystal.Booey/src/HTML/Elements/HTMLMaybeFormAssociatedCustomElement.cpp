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

#include "Krystal.Booey/HTML/Elements/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/HTML/CustomElement/FormAssociatedCustomElement.hpp"

namespace krys::boo::html
{
  HTMLMaybeFormAssociatedCustomElement::HTMLMaybeFormAssociatedCustomElement(
    dom::Document &document, dom::DOMStringAtom localName) noexcept
      : HTMLElement(document, HTMLElementInterface::MaybeFormAssociatedCustom, localName,
                    HTMLElementFlags::None, dom::NodeFlags::None)
  {
    krys_debug_assert(dom::NameValidation::IsValidCustomElementName(localName.View()));
  }

  HTMLMaybeFormAssociatedCustomElement::~HTMLMaybeFormAssociatedCustomElement() noexcept = default;

  bool HTMLMaybeFormAssociatedCustomElement::IsFormListedElement() const noexcept
  {
    return IsFormAssociatedCustomElement();
  }

  bool HTMLMaybeFormAssociatedCustomElement::IsValidatedFormListedElement() const noexcept
  {
    return IsFormAssociatedCustomElement();
  }

  bool HTMLMaybeFormAssociatedCustomElement::IsFormAssociatedCustomElement() const noexcept
  {
    return HasFormAssociatedInterface() && IsDefinedCustomElement();
  }

  FormAssociatedElement *HTMLMaybeFormAssociatedCustomElement::AsFormAssociatedElement() noexcept
  {
    return IsFormAssociatedCustomElement() ? _formAssociatedCustomElement.get() : nullptr;
  }

  FormListedElement *HTMLMaybeFormAssociatedCustomElement::AsFormListedElement() noexcept
  {
    return IsFormAssociatedCustomElement() ? _formAssociatedCustomElement.get() : nullptr;
  }

  ValidatedFormListedElement *HTMLMaybeFormAssociatedCustomElement::AsValidatedFormListedElement() noexcept
  {
    return IsFormAssociatedCustomElement() ? _formAssociatedCustomElement.get() : nullptr;
  }

  FormAssociatedCustomElement *
    HTMLMaybeFormAssociatedCustomElement::FormAssociatedCustomElementForElementInternals() const noexcept
  {
    return HasFormAssociatedInterface() && IsPrecustomizedOrDefinedCustomElement()
             ? _formAssociatedCustomElement.get()
             : nullptr;
  }

  bool HTMLMaybeFormAssociatedCustomElement::MatchesValidPseudoClass() const noexcept
  {
    return IsFormAssociatedCustomElement() && _formAssociatedCustomElement->MatchesValidPseudoClass();
  }

  bool HTMLMaybeFormAssociatedCustomElement::MatchesInvalidPseudoClass() const noexcept
  {
    return IsFormAssociatedCustomElement() && _formAssociatedCustomElement->MatchesInvalidPseudoClass();
  }

  bool HTMLMaybeFormAssociatedCustomElement::MatchesUserValidPseudoClass() const noexcept
  {
    return IsFormAssociatedCustomElement() && _formAssociatedCustomElement->MatchesUserValidPseudoClass();
  }

  bool HTMLMaybeFormAssociatedCustomElement::MatchesUserInvalidPseudoClass() const noexcept
  {
    return IsFormAssociatedCustomElement() && _formAssociatedCustomElement->MatchesUserInvalidPseudoClass();
  }

  bool HTMLMaybeFormAssociatedCustomElement::IsLabelable() const noexcept
  {
    return IsFormAssociatedCustomElement();
  }

  bool HTMLMaybeFormAssociatedCustomElement::IsDisabledFormControl() const noexcept
  {
    return IsFormAssociatedCustomElement() && _formAssociatedCustomElement->IsDisabled();
  }

  void HTMLMaybeFormAssociatedCustomElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (IsFormAssociatedCustomElement())
    {
      if (auto *parent = ParentNode())
      {
        _formAssociatedCustomElement->InsertedIntoAncestor(IsConnected(), *parent);
      }
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();
    if (IsFormAssociatedCustomElement())
    {
      _formAssociatedCustomElement->DidFinishInsertingNode();
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::OnRemove(bool isSubtreeRoot,
                                                      dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (IsFormAssociatedCustomElement())
    {
      _formAssociatedCustomElement->RemovedFromAncestor(oldAncestor.IsConnected(), oldAncestor);
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::OnMove(bool isSubtreeRoot,
                                                    dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);
    if (!IsFormAssociatedCustomElement())
    {
      return;
    }

    // A move is a removal and an insertion that keep the element connected.
    _formAssociatedCustomElement->RemovedFromAncestor(false, oldAncestor);
    if (auto *parent = ParentNode())
    {
      _formAssociatedCustomElement->InsertedIntoAncestor(IsConnected(), *parent);
    }
    if (IsConnected())
    {
      _formAssociatedCustomElement->DidFinishInsertingNode();
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);
    if (IsFormAssociatedCustomElement())
    {
      _formAssociatedCustomElement->DidMoveToNewDocument();
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                                                Maybe<dom::DOMStringView> oldValue,
                                                                Maybe<dom::DOMStringView> value,
                                                                dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (IsFormAssociatedCustomElement() && namespaceURI == dom::DOMStringAtom::Null())
    {
      _formAssociatedCustomElement->ParseAttribute(localName, value);
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::SetInterfaceIsFormAssociated() noexcept
  {
    if (!_formAssociatedCustomElement)
    {
      _formAssociatedCustomElement = CreateUnique<FormAssociatedCustomElement>(*this);
      // It's now a listed element if it's defined.
      InvalidateLiveCollectionCachesInAncestors();
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::WillUpgradeFormAssociated() noexcept
  {
    krys_debug_assert(GetCustomElementState() == dom::CustomElementState::Precustomized);
    SetInterfaceIsFormAssociated();
    _formAssociatedCustomElement->WillUpgrade();
  }

  void HTMLMaybeFormAssociatedCustomElement::DidUpgradeFormAssociated() noexcept
  {
    krys_debug_assert(IsFormAssociatedCustomElement());
    _formAssociatedCustomElement->DidUpgrade();
  }
}
