#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlhtmlelement
  class HTMLHtmlElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHtmlElement);

  public:
    HTMLHtmlElement(Document &document) noexcept;

#pragma region HTMLHtmlElement Obsolete members - https://html.spec.whatwg.org/#HTMLHtmlElement-partial

    /// @see https://html.spec.whatwg.org/#dom-html-version
    KRYS_NODISCARD DOMString Version() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-html-version
    void Version(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLHtmlElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLHtmlElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();