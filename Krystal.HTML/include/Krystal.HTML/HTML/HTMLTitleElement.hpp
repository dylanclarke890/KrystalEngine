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

    /// @see https://html.spec.whatwg.org/#dom-title-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-title-text
    ExceptionOr<void> Text(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTitleElement)

  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTitleElement();
  }

  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return Is<Krys::HTML::HTMLElement>(target)
           && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLTitleElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();