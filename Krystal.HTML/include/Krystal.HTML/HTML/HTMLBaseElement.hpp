#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlbaseelement
  class HTMLBaseElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBaseElement);

  public:
    HTMLBaseElement(Document &document) noexcept;

#pragma region HTMLBaseElement - https://html.spec.whatwg.org/#htmlbaseelement

    /// @see https://html.spec.whatwg.org/#dom-base-href
    KRYS_NODISCARD ExceptionOr<USVString> Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-base-href
    ExceptionOr<void> Href(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-base-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-base-target
    void Target(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLBaseElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLBaseElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
