#pragma once

#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltemplateelement
  class HTMLTemplateElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTemplateElement);

  private:
    Ref<dom::DocumentFragment> _content;

  public:
    HTMLTemplateElement(dom::Document &document) noexcept;

#pragma region HTMLTemplateElement - https://html.spec.whatwg.org/#htmltemplateelement

    /// @see https://html.spec.whatwg.org/#dom-template-content
    RefPtr<dom::DocumentFragment> Content() const noexcept;

    /// @brief The template contents: a document fragment, or the shadow root of the host a declarative shadow
    /// root was attached to (the template element is then never inserted).
    /// @see https://html.spec.whatwg.org/#template-contents
    KRYS_NODISCARD dom::DocumentFragment &TemplateContents() const noexcept
    {
      return *_content;
    }

    /// @brief Sets the template contents to the declarative shadow root @p shadow.
    void SetTemplateContents(dom::ShadowRoot &shadow) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootmode
    KRYS_NODISCARD dom::DOMString ShadowRootMode() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootmode
    void ShadowRootMode(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootdelegatesfocus
    KRYS_NODISCARD bool ShadowRootDelegatesFocus() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootdelegatesfocus
    void ShadowRootDelegatesFocus(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment
    KRYS_NODISCARD dom::DOMString ShadowRootSlotAssignment() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment
    void ShadowRootSlotAssignment(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootclonable
    KRYS_NODISCARD bool ShadowRootClonable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootclonable
    void ShadowRootClonable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootserializable
    KRYS_NODISCARD bool ShadowRootSerialisable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootserializable
    void ShadowRootSerialisable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootcustomelementregistry
    KRYS_NODISCARD dom::DOMString ShadowRootCustomElementRegistry() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootcustomelementregistry
    void ShadowRootCustomElementRegistry(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTemplateElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTemplateElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLTemplateElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();