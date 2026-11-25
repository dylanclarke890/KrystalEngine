#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmlquoteelement
  class HTMLQuoteElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLQuoteElement);

  public:
    HTMLQuoteElement(Document &document) noexcept;

#pragma region HTMLQuoteElement - https://html.spec.whatwg.org/multipage/grouping-content.html#htmlquoteelement

    /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#dom-quote-cite
    KRYS_NODISCARD dom::USVString Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#dom-quote-cite
    void Cite(dom::USVString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLQuoteElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLQuoteElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
