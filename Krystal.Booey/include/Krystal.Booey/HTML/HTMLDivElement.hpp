#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmldivelement
  class HTMLDivElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDivElement);

  public:
    HTMLDivElement(Document &document) noexcept;

#pragma region HTMLDivElement Obsolete members - https://html.spec.whatwg.org/#HTMLDivElement-partial

    /// @see https://html.spec.whatwg.org/#dom-div-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-div-align
    void Align(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDivElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDivElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();