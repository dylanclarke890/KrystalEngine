#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlolistelement
  class HTMLOListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOListElement);

  public:
    HTMLOListElement(Document &document) noexcept;

#pragma region HTMLOListElement https://html.spec.whatwg.org/#htmlolistelement

    /// @see https://html.spec.whatwg.org/#dom-ol-reversed
    KRYS_NODISCARD bool Reversed() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-reversed
    void Reversed(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-start
    KRYS_NODISCARD int32 Start() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-start
    void Start(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-type
    void Type(DOMString&& value) noexcept;

#pragma endregion

#pragma region HTMLOListElement Obsolete members - https://html.spec.whatwg.org/#HTMLOListElement-partial

    /// @see https://html.spec.whatwg.org/#dom-ol-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-compact
    void Compact(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLOListElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLOListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();