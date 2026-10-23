#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::html
{
  class FormAssociatedCustomElement;

  /// @brief An HTML element whose name is a valid custom element name: a custom element, defined or not yet,
  /// which is form-associated if its definition says so.
  class HTMLMaybeFormAssociatedCustomElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMaybeFormAssociatedCustomElement);

  private:
    UniquePtr<FormAssociatedCustomElement> _formAssociatedCustomElement;

  public:
    HTMLMaybeFormAssociatedCustomElement(dom::Document &document, dom::DOMStringAtom localName) noexcept;
    ~HTMLMaybeFormAssociatedCustomElement() noexcept override;

    KRYS_NODISCARD bool IsFormListedElement() const noexcept final;
    KRYS_NODISCARD bool IsValidatedFormListedElement() const noexcept final;

    /// @brief Whether the element is a defined custom element whose definition is form-associated.
    /// @see https://html.spec.whatwg.org/#form-associated-custom-element
    KRYS_NODISCARD bool IsFormAssociatedCustomElement() const noexcept;

    KRYS_NODISCARD FormAssociatedElement *AsFormAssociatedElement() noexcept final;
    KRYS_NODISCARD FormListedElement *AsFormListedElement() noexcept final;
    KRYS_NODISCARD ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final;

    /// @brief The form-associated half that the element's internals act on: present once the element is
    /// being upgraded (or created) by a form-associated definition.
    KRYS_NODISCARD FormAssociatedCustomElement *
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
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;
    void OnAdopt(dom::Document &oldDocument) noexcept final;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMaybeFormAssociatedCustomElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement()
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLMaybeFormAssociatedCustomElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMaybeFormAssociatedCustomElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
