#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmldlistelement
  class HTMLDListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDListElement);

  public:
    HTMLDListElement(Document &document) noexcept;

#pragma region HTMLDListElement Obsolete members - https://html.spec.whatwg.org/#HTMLDListElement-partial

    /// @see https://html.spec.whatwg.org/#dom-dl-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dl-compact
    void Compact(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDListElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();