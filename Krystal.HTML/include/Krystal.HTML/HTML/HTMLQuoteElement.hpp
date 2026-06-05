#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlquoteelement
  class HTMLQuoteElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLQuoteElement);

  public:
    HTMLQuoteElement(Document &document) noexcept;

#pragma region HTMLQuoteElement - https://html.spec.whatwg.org/#htmlquoteelement

    /// @see https://html.spec.whatwg.org/#dom-quote-cite
    KRYS_NODISCARD ExceptionOr<USVString> Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-quote-cite
    ExceptionOr<void> Cite(USVString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLQuoteElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLQuoteElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();