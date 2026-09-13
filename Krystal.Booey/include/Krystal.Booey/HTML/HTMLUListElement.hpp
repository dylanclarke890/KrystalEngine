#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlulistelement
  class HTMLUListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLUListElement);

  public:
    HTMLUListElement(dom::Document &document) noexcept;

#pragma region HTMLUListElement Obsolete members - https://html.spec.whatwg.org/#HTMLUListElement-partial

    /// @see https://html.spec.whatwg.org/#dom-ul-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ul-compact
    void Compact(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ul-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ul-type
    void Type(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLUListElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLUListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();