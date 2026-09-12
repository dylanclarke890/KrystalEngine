#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlhtmlelement
  class HTMLHtmlElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHtmlElement);

  public:
    HTMLHtmlElement(Document &document) noexcept;

#pragma region HTMLHtmlElement Obsolete members - https://html.spec.whatwg.org/#HTMLHtmlElement-partial

    /// @see https://html.spec.whatwg.org/#dom-html-version
    KRYS_NODISCARD dom::DOMString Version() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-html-version
    void Version(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLHtmlElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLHtmlElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLHtmlElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();