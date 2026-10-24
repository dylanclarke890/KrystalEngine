#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/HTML/FormAssociatedCustomElement.hpp"

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
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::WillUpgradeFormAssociated() noexcept
  {
    krys_debug_assert(GetCustomElementState() == dom::CustomElementState::Precustomized);
    SetInterfaceIsFormAssociated();
    _formAssociatedCustomElement->WillUpgrade();
  }

  void HTMLMaybeFormAssociatedCustomElement::FinishParsingChildren() noexcept
  {
    HTMLElement::FinishParsingChildren();
    if (IsFormAssociatedCustomElement())
    {
      _formAssociatedCustomElement->FinishParsingChildren();
    }
  }

  void HTMLMaybeFormAssociatedCustomElement::DidUpgradeFormAssociated() noexcept
  {
    krys_debug_assert(IsFormAssociatedCustomElement());
    _formAssociatedCustomElement->DidUpgrade();
  }
}
