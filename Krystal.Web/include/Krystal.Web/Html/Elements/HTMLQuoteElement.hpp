#pragma once

#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmlquoteelement
  class HTMLQuoteElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLQuoteElement);

  public:
    HTMLQuoteElement(Document &document) noexcept;

#pragma region HTMLQuoteElement - https://html.spec.whatwg.org/multipage/grouping-content.html#htmlquoteelement

    /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#dom-quote-cite
    KRYS_NODISCARD USVString Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#dom-quote-cite
    void Cite(USVString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLQuoteElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLQuoteElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
