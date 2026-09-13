#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlbrelement
  class HTMLBRElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBRElement);

  public:
    HTMLBRElement(dom::Document &document) noexcept;

#pragma region HTMLBRElement Obsolete members - https://html.spec.whatwg.org/#HTMLBRElement-partial

    /// @see https://html.spec.whatwg.org/#dom-br-compact
    KRYS_NODISCARD dom::DOMString Clear() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-br-compact
    void Clear(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLBRElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLBRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();