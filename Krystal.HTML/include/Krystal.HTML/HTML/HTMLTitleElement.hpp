#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltitleelement
  class HTMLTitleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTitleElement);

  public:
    HTMLTitleElement(Document &document) noexcept;

#pragma region HTMLTitleElement - https://html.spec.whatwg.org/#htmltitleelement

    KRYS_NODISCARD DOMString Text() const noexcept;

    ExceptionOr<void> Text(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTitleElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTitleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();