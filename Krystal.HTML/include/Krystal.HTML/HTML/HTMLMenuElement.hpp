#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLMenuElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLMenuElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();