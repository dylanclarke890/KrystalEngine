#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltablesectionelement
  class HTMLTableSectionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableSectionElement);

  public:
    HTMLTableSectionElement(Document &document) noexcept;

#pragma region HTMLTableSectionElement - https://html.spec.whatwg.org/#htmltablesectionelement

    // TODO(HTMLTableSectionElement, HTML): Implement api.

#pragma endregion

#pragma region HTMLTableSectionElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableSectionElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    void Char(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    void CharOff(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    void VAlign(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTableSectionElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableSectionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();