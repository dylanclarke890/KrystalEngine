#pragma once

#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlbodyelement
  class HTMLBodyElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBodyElement);

  public:
    HTMLBodyElement(dom::Document &document) noexcept;

#pragma region WindowEventHandlers - https://html.spec.whatwg.org/#windoweventhandlers

    /// @note The body and frameset elements' window event handlers are their document's window's.
    KRYS_WINDOW_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region HTMLBodyElement Obsolete members - https://html.spec.whatwg.org/#HTMLBodyElement-partial

    /// @see https://html.spec.whatwg.org/#dom-body-text
    KRYS_NODISCARD dom::DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-text
    void Text(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-link
    KRYS_NODISCARD dom::DOMString Link() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-link
    void Link(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-vlink
    KRYS_NODISCARD dom::DOMString VLink() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-vlink
    void VLink(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-alink
    KRYS_NODISCARD dom::DOMString ALink() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-alink
    void ALink(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-bgcolor
    void BgColor(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-background
    KRYS_NODISCARD dom::DOMString Background() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-background
    void Background(dom::DOMString &&value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLBodyElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLBodyElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLBodyElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();