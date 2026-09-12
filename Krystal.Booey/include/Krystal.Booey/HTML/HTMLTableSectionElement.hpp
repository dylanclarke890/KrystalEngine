#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
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
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    void Char(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    void CharOff(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    void VAlign(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableSectionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableSectionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();