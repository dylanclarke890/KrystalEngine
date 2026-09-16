#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmllielement
  class HTMLLIElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLIElement);

  public:
    HTMLLIElement(dom::Document &document) noexcept;

#pragma region HTMLLIElement - https://html.spec.whatwg.org/#htmllielement

    /// @see https://html.spec.whatwg.org/#dom-li-value
    KRYS_NODISCARD int32 Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-li-value
    void Value(int32 value) noexcept;

#pragma endregion

#pragma region HTMLLIElement Obsolete members - https://html.spec.whatwg.org/#HTMLLIElement-partial

    /// @see https://html.spec.whatwg.org/#dom-li-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-li-type
    void Type(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLIElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLIElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();