#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
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
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-name
    void Name(dom::DOMString &&value) noexcept;

    // TODO(HTMLOUTPUT, HTML): Implement the type attribute.
    // readonly attribute dom::DOMString type;
    // [CEReactions] attribute dom::DOMString defaultValue;
    // [CEReactions] attribute dom::DOMString value;
    //
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute dom::DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(dom::DOMString error);
    //
    // readonly attribute NodeList labels;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLOutputElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLOutputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();