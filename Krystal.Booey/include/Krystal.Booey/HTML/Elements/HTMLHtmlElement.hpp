#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlhtmlelement
  class HTMLHtmlElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHtmlElement);

  public:
    HTMLHtmlElement(Document &document) noexcept;

#pragma region HTMLHtmlElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLHtmlElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-html-version
    KRYS_NODISCARD DOMString Version() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-html-version
    void Version(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLHtmlElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLHtmlElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target) && Downcast<krys::boo::HTMLElement>(target).IsHTMLHtmlElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
