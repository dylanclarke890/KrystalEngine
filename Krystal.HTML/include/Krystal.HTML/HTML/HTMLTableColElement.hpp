#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltablecolelement
  class HTMLTableColElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableColElement);

  public:
    HTMLTableColElement(Document &document) noexcept;

#pragma region HTMLTableColElement - https://html.spec.whatwg.org/#htmltablecolelement

    /// @see https://html.spec.whatwg.org/#dom-colgroup-span
    KRYS_NODISCARD uint32 Span() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-span
    void Span(uint32 value) noexcept;

#pragma endregion

#pragma region HTMLTableColElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableColElement-partial

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    void Char(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    void CharOff(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    void VAlign(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    void Width(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTableColElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableColElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();