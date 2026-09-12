#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmllegendelement
  class HTMLLegendElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLegendElement);

  public:
    HTMLLegendElement(Document &document) noexcept;

#pragma region HTMLLegendElement - https://html.spec.whatwg.org/#htmllegendelement

    // TODO(HTMLLegendElement): Form().
    // readonly attribute HTMLFormElement? form;

#pragma endregion

#pragma region HTMLLegendElement Obsolete members - https://html.spec.whatwg.org/#HTMLLegendElement-partial

    /// @see https://html.spec.whatwg.org/#dom-legend-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-legend-align
    void Align(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLegendElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLegendElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
