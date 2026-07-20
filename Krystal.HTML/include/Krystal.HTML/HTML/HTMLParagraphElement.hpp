#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlparagraphelement
  class HTMLParagraphElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLParagraphElement);

  public:
    HTMLParagraphElement(Document &document) noexcept;

#pragma region HTMLParagraphElement Obsolete members - https://html.spec.whatwg.org/#HTMLParagraphElement-partial

    /// @see https://html.spec.whatwg.org/#dom-p-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-p-align
    void Align(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLParagraphElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLParagraphElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();