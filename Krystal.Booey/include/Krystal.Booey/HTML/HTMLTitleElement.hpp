#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltitleelement
  class HTMLTitleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTitleElement);

  public:
    HTMLTitleElement(Document &document) noexcept;

#pragma region HTMLTitleElement - https://html.spec.whatwg.org/#htmltitleelement

    /// @see https://html.spec.whatwg.org/#dom-title-text
    KRYS_NODISCARD dom::DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-title-text
    dom::ExceptionOr<void> Text(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTitleElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTitleElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLTitleElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();