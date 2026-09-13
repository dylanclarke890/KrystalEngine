#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlbuttonelement
  class HTMLButtonElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLButtonElement);

  public:
    HTMLButtonElement(dom::Document &document) noexcept;

#pragma region HTMLButtonElement - https://html.spec.whatwg.org/#htmlbuttonelement

    /// @see https://html.spec.whatwg.org/#dom-button-command
    KRYS_NODISCARD dom::DOMString Command() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-command
    void Command(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-commandforelement
    // TODO(HTMLBUTTON, HTML): Implement the command API.
    // [CEReactions] attribute Element? commandForElement;

    /// @see https://html.spec.whatwg.org/#dom-button-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLBUTTON, HTML): Implement Form().
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-button-formaction
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> FormAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formaction
    dom::ExceptionOr<void> FormAction(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formenctype
    KRYS_NODISCARD dom::DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formenctype
    void EncType(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formmethod
    KRYS_NODISCARD dom::DOMString FormMethod() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formmethod
    void FormMethod(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formnovalidate
    KRYS_NODISCARD bool FormNoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formtarget
    KRYS_NODISCARD dom::DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formtarget
    void FormTarget(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-value
    void Value(dom::DOMString &&value) noexcept;

    // TODO(HTMLBUTTON, HTML): Validity API.
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLButtonElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLButtonElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();