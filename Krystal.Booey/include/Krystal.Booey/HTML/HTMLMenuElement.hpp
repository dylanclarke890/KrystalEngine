#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlulistelement
  class HTMLMenuElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMenuElement);

  public:
    HTMLMenuElement(Document &document) noexcept;

#pragma region HTMLMenuElement Obsolete members - https://html.spec.whatwg.org/#HTMLMenuElement-partial

    /// @see https://html.spec.whatwg.org/#dom-menu-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-menu-compact
    void Compact(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMenuElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMenuElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();