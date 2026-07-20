#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmllabelelement
  class HTMLLabelElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLabelElement);

  public:
    HTMLLabelElement(Document &document) noexcept;

#pragma region HTMLLabelElement - https://html.spec.whatwg.org/#htmllabelelement

    // TODO(HTMLLABEL, HTML): Implement the form/control API.
    // readonly attribute HTMLFormElement? form;
    // readonly attribute HTMLElement? control;

    /// @see https://html.spec.whatwg.org/#dom-label-htmlfor
    KRYS_NODISCARD DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-label-htmlfor
    void HtmlFor(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLLabelElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLLabelElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();