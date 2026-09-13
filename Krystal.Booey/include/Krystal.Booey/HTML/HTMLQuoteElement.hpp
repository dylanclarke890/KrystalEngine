#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlquoteelement
  class HTMLQuoteElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLQuoteElement);

  public:
    HTMLQuoteElement(dom::Document &document) noexcept;

#pragma region HTMLQuoteElement - https://html.spec.whatwg.org/#htmlquoteelement

    /// @see https://html.spec.whatwg.org/#dom-quote-cite
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-quote-cite
    dom::ExceptionOr<void> Cite(dom::USVString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLQuoteElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLQuoteElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();