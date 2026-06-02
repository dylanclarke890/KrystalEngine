#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlbodyelement
  class HTMLBodyElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBodyElement);

  public:
    HTMLBodyElement(Document &document) noexcept;

#pragma region HTMLBodyElement Obsolete members - https://html.spec.whatwg.org/#HTMLBodyElement-partial

    /// @see https://html.spec.whatwg.org/#dom-body-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-text
    void Text(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-link
    KRYS_NODISCARD DOMString Link() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-link
    void Link(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-vlink
    KRYS_NODISCARD DOMString VLink() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-vlink
    void VLink(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-alink
    KRYS_NODISCARD DOMString ALink() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-alink
    void ALink(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-bgcolor
    void BgColor(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-background
    KRYS_NODISCARD DOMString Background() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-body-background
    void Background(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLBodyElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLBodyElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();