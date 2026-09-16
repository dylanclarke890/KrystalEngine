#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltablecaptionelement
  class HTMLTableCaptionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCaptionElement);

  public:
    HTMLTableCaptionElement(dom::Document &document) noexcept;

#pragma region HTMLTableCaptionElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableCaptionElement-partial

    /// @see https://html.spec.whatwg.org/#dom-caption-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-caption-align
    void Align(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableCaptionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCaptionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();