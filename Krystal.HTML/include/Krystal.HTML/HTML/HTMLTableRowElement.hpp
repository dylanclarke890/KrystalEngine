#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
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
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    void Char(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    void CharOff(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-valign
    void VAlign(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-bgcolor
    void BgColor(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTableRowElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableRowElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();