#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltablecolelement
  class HTMLTableColElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableColElement);

  public:
    HTMLTableColElement(dom::Document &document) noexcept;

#pragma region HTMLTableColElement - https://html.spec.whatwg.org/#htmltablecolelement

    /// @see https://html.spec.whatwg.org/#dom-colgroup-span
    KRYS_NODISCARD uint32 Span() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-span
    void Span(uint32 value) noexcept;

#pragma endregion

#pragma region HTMLTableColElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableColElement-partial

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    void Char(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    void CharOff(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    void VAlign(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    void Width(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableColElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableColElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();