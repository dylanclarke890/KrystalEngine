#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmldlistelement
  class HTMLDListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDListElement);

  public:
    HTMLDListElement(Document &document) noexcept;

#pragma region HTMLDListElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLDListElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-dl-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-dl-compact
    void Compact(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLDListElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLDListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
