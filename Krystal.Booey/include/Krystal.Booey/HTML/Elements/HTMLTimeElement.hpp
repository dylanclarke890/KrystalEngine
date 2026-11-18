#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
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
    void DateTime(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTimeElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTimeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
