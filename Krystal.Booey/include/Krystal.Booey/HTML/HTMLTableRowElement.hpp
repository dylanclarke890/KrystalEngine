#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltablerowelement
  class HTMLTableRowElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableRowElement);

  public:
    HTMLTableRowElement(Document &document) noexcept;

#pragma region HTMLTableRowElement - https://html.spec.whatwg.org/#htmltablerowelement

    // TODO(HTMLTableRowElement, HTML): Implement api.

#pragma endregion

#pragma region HTMLTableRowElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableRowElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tr-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    void Char(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    void CharOff(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-valign
    void VAlign(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-bgcolor
    void BgColor(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableRowElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableRowElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();