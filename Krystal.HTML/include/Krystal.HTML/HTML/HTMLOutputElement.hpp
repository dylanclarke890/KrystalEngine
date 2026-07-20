#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmloutputelement
  class HTMLOutputElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOutputElement);

  private:
    UniquePtr<DOMTokenList> _htmlFor;

  public:
    HTMLOutputElement(Document &document) noexcept;

#pragma region HTMLOutputElement - https://html.spec.whatwg.org/#htmloutputelement

    /// @see https://html.spec.whatwg.org/#dom-output-htmlfor
    KRYS_NODISCARD DOMTokenList &HtmlFor() noexcept;

    // TODO(HTMLOUTPUT, HTML): Implement the form API.
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-output-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-name
    void Name(DOMString &&value) noexcept;

    // TODO(HTMLOUTPUT, HTML): Implement the type attribute.
    // readonly attribute DOMString type;
    // [CEReactions] attribute DOMString defaultValue;
    // [CEReactions] attribute DOMString value;
    //
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(DOMString error);
    //
    // readonly attribute NodeList labels;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLOutputElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLOutputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();