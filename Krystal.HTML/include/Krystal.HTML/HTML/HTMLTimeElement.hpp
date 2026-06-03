#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltimeelement
  class HTMLTimeElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTimeElement);

  public:
    HTMLTimeElement(Document &document) noexcept;

#pragma region HTMLTimeElement - https://html.spec.whatwg.org/#htmltimeelement

    /// @see https://html.spec.whatwg.org/#dom-time-datetime
    KRYS_NODISCARD DOMString DateTime() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-time-datetime
    void DateTime(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTimeElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTimeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();