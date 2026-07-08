#pragma once

#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltemplateelement
  class HTMLTemplateElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTemplateElement);

  private:
    Ref<DocumentFragment> _content;

  public:
    HTMLTemplateElement(Document &document) noexcept;

#pragma region HTMLTemplateElement - https://html.spec.whatwg.org/#htmltemplateelement

    /// @see https://html.spec.whatwg.org/#dom-template-content
    RefPtr<DocumentFragment> Content() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootmode
    KRYS_NODISCARD DOMString ShadowRootMode() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootmode
    void ShadowRootMode(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootdelegatesfocus
    KRYS_NODISCARD bool ShadowRootDelegatesFocus() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootdelegatesfocus
    void ShadowRootDelegatesFocus(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment
    KRYS_NODISCARD DOMString ShadowRootSlotAssignment() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootslotassignment
    void ShadowRootSlotAssignment(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootclonable
    KRYS_NODISCARD bool ShadowRootClonable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootclonable
    void ShadowRootClonable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootserializable
    KRYS_NODISCARD bool ShadowRootSerializable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootserializable
    void ShadowRootSerializable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootcustomelementregistry
    KRYS_NODISCARD DOMString ShadowRootCustomElementRegistry() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-template-shadowrootcustomelementregistry
    void ShadowRootCustomElementRegistry(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTemplateElement)

  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTemplateElement();
  }

  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return Is<Krys::HTML::HTMLElement>(target)
           && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLTemplateElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();