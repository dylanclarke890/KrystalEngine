#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmldataelement
  class HTMLDataElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDataElement);

  public:
    HTMLDataElement(Document &document) noexcept;

#pragma region HTMLDataElement - https://html.spec.whatwg.org/#htmldataelement

    /// @see https://html.spec.whatwg.org/#dom-data-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-data-value
    void Value(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLDataElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLDataElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();