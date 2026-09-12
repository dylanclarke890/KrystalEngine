#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmldataelement
  class HTMLDataElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDataElement);

  public:
    HTMLDataElement(Document &document) noexcept;

#pragma region HTMLDataElement - https://html.spec.whatwg.org/#htmldataelement

    /// @see https://html.spec.whatwg.org/#dom-data-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-data-value
    void Value(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDataElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDataElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();