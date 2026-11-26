#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmlmenuelement
  class HTMLMenuElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMenuElement);

  public:
    HTMLMenuElement(Document &document) noexcept;

#pragma region HTMLMenuElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLMenuElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-menu-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-menu-compact
    void Compact(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLMenuElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLMenuElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
