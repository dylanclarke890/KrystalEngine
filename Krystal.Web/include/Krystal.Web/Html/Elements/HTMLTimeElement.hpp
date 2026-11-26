#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmltimeelement
  class HTMLTimeElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTimeElement);

  public:
    HTMLTimeElement(Document &document) noexcept;

#pragma region HTMLTimeElement - https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmltimeelement

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-time-datetime
    KRYS_NODISCARD DOMString DateTime() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-time-datetime
    void DateTime(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLTimeElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLTimeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
