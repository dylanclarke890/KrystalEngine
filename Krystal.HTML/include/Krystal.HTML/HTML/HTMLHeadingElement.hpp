#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlheadingelement
  class HTMLHeadingElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadingElement);

  public:
    HTMLHeadingElement(Document &document) noexcept;

#pragma region HTMLHeadingElement Obsolete members - https://html.spec.whatwg.org/#HTMLHeadingElement-partial

    /// @see https://html.spec.whatwg.org/#dom-hx-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hx-align
    void Align(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLHeadingElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLHeadingElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();