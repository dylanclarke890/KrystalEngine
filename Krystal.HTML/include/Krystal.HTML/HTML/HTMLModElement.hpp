#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlmodelement
  class HTMLModElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLModElement);

  public:
    HTMLModElement(Document &document) noexcept;

#pragma region HTMLModElement - https://html.spec.whatwg.org/#htmlmodelement

    /// @see https://html.spec.whatwg.org/#dom-mod-cite
    KRYS_NODISCARD ExceptionOr<USVString> Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-mod-cite
    ExceptionOr<void> Cite(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-mod-datetime
    KRYS_NODISCARD DOMString DateTime() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-mod-datetime
    void DateTime(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLModElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLModElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();