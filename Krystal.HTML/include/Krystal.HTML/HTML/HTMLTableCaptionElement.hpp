#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltablecaptionelement
  class HTMLTableCaptionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCaptionElement);

  public:
    HTMLTableCaptionElement(Document &document) noexcept;

#pragma region HTMLTableCaptionElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableCaptionElement-partial

    /// @see https://html.spec.whatwg.org/#dom-caption-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-caption-align
    void Align(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTableCaptionElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCaptionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();