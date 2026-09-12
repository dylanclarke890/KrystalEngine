#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
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
    KRYS_NODISCARD dom::DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-label-htmlfor
    void HtmlFor(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLabelElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLabelElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();