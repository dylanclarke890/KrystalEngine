#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlbrelement
  class HTMLBRElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBRElement);

  public:
    HTMLBRElement(Document &document) noexcept;

#pragma region HTMLBRElement Obsolete members - https://html.spec.whatwg.org/#HTMLBRElement-partial

    /// @see https://html.spec.whatwg.org/#dom-br-compact
    KRYS_NODISCARD DOMString Clear() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-br-compact
    void Clear(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLBRElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLBRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();