#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmlbodyelement
  class HTMLBodyElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBodyElement);

  public:
    HTMLBodyElement(Document &document) noexcept;

#pragma region WindowEventHandlers - https://html.spec.whatwg.org/#windoweventhandlers

    /// @note The body and frameset elements' window event handlers are their document's window's.
    KRYS_WINDOW_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region HTMLBodyElement Obsolete members - https://html.spec.whatwg.org/#HTMLBodyElement-partial

    /// @see https://html.spec.whatwg.org/#dom-body-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-text
    void Text(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-link
    KRYS_NODISCARD DOMString Link() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-link
    void Link(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-vlink
    KRYS_NODISCARD DOMString VLink() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-vlink
    void VLink(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-alink
    KRYS_NODISCARD DOMString ALink() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-alink
    void ALink(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-bgcolor
    void BgColor(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-background
    KRYS_NODISCARD DOMString Background() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-background
    void Background(DOMString value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLBodyElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLBodyElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target) && Downcast<krys::boo::HTMLElement>(target).IsHTMLBodyElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
