#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmldataelement
  class HTMLDataElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDataElement);

  public:
    HTMLDataElement(Document &document) noexcept;

#pragma region HTMLDataElement - https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmldataelement

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-data-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#dom-data-value
    void Value(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLDataElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLDataElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
