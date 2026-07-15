#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlbuttonelement
  class HTMLButtonElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLButtonElement);

  public:
    HTMLButtonElement(Document &document) noexcept;

#pragma region HTMLButtonElement - https://html.spec.whatwg.org/#htmlbuttonelement

    /// @see https://html.spec.whatwg.org/#dom-button-command
    KRYS_NODISCARD DOMString Command() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-command
    void Command(DOMString &&value) noexcept;

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
    KRYS_NODISCARD ExceptionOr<USVString> FormAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formaction
    ExceptionOr<void> FormAction(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formenctype
    KRYS_NODISCARD DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formenctype
    void EncType(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formmethod
    KRYS_NODISCARD DOMString FormMethod() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formmethod
    void FormMethod(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formnovalidate
    KRYS_NODISCARD bool FormNoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formtarget
    KRYS_NODISCARD DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formtarget
    void FormTarget(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-type
    void Type(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-value
    void Value(DOMString &&value) noexcept;

    // TODO(HTMLBUTTON, HTML): Validity API.
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLButtonElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLButtonElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();