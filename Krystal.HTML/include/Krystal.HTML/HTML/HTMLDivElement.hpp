#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmldivelement
  class HTMLDivElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDivElement);

  public:
    HTMLDivElement(Document &document) noexcept;

#pragma region HTMLDivElement Obsolete members - https://html.spec.whatwg.org/#HTMLDivElement-partial

    /// @see https://html.spec.whatwg.org/#dom-div-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-div-align
    void Align(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLDivElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLDivElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();