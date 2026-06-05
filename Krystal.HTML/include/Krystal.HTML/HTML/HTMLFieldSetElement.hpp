#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlfieldsetelement
  class HTMLFieldSetElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFieldSetElement);

  public:
    HTMLFieldSetElement(Document &document) noexcept;

#pragma region HTMLFieldSetElement - https://html.spec.whatwg.org/#htmlfieldsetelement

    /// @see https://html.spec.whatwg.org/#dom-fieldset-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLFieldSetElement): Implement the rest of the API.
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-type
    KRYS_NODISCARD DOMStringAtom Type() const noexcept
    {
      return u8"fieldset";
    }

    // [SameObject] readonly attribute HTMLCollection elements;
    //
    // readonly attribute boolean willValidate;
    // [SameObject] readonly attribute ValidityState validity;
    // readonly attribute DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(DOMString error);

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLFieldSetElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLFieldSetElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();