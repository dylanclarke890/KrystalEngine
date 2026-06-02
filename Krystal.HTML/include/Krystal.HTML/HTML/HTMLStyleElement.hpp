#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlstyleelement
  class HTMLStyleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLStyleElement);

  private:
    UniquePtr<DOMTokenList> _blocking;

  public:
    HTMLStyleElement(Document &document) noexcept;

#pragma region HTMLStyleElement - https://html.spec.whatwg.org/#htmlstyleelement

    /// @see https://html.spec.whatwg.org/#dom-style-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-media
    void Media(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-blocking
    KRYS_NODISCARD DOMTokenList &Blocking() noexcept;

#pragma endregion

#pragma region HTMLStyleElement Obsolete members - https://html.spec.whatwg.org/#HTMLStyleElement-partial

    /// @see https://html.spec.whatwg.org/#dom-style-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-type
    void Type(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLStyleElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLStyleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();