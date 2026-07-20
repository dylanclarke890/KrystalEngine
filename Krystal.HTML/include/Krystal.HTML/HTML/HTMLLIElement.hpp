#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmllielement
  class HTMLLIElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLIElement);

  public:
    HTMLLIElement(Document &document) noexcept;

#pragma region HTMLLIElement - https://html.spec.whatwg.org/#htmllielement

    /// @see https://html.spec.whatwg.org/#dom-li-value
    KRYS_NODISCARD int32 Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-li-value
    void Value(int32 value) noexcept;

#pragma endregion

#pragma region HTMLLIElement Obsolete members - https://html.spec.whatwg.org/#HTMLLIElement-partial

    /// @see https://html.spec.whatwg.org/#dom-li-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-li-type
    void Type(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLLIElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLLIElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();