#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlparagraphelement
  class HTMLParagraphElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLParagraphElement);

  public:
    HTMLParagraphElement(dom::Document &document) noexcept;

#pragma region HTMLParagraphElement Obsolete members - https://html.spec.whatwg.org/#HTMLParagraphElement-partial

    /// @see https://html.spec.whatwg.org/#dom-p-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-p-align
    void Align(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLParagraphElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLParagraphElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();