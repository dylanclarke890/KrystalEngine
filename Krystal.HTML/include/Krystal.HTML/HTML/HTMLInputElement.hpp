#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlinputelement
  class HTMLInputElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLInputElement);

  public:
    HTMLInputElement(Document &document) noexcept;

#pragma region HTMLInputElement - https://html.spec.whatwg.org/#htmlinputelement

    /// @see https://html.spec.whatwg.org/#dom-input-accept
    KRYS_NODISCARD DOMString Accept() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-accept
    void Accept(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alpha
    KRYS_NODISCARD bool Alpha() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alpha
    void Alpha(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alt
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alt
    void Alt(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-autocomplete
    void Autocomplete(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultchecked
    KRYS_NODISCARD bool DefaultChecked() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultchecked
    void DefaultChecked(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-checked
    KRYS_NODISCARD bool Checked() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-checked
    void Checked(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-colorspace
    KRYS_NODISCARD DOMString ColorSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-colorspace
    void ColorSpace(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-dirname
    KRYS_NODISCARD DOMString DirName() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-dirname
    void DirName(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLINPUT, HTML): Implement Form().
    // readonly attribute HTMLFormElement? form;

    // TODO(HTMLINPUT, HTML): Implement Files().
    // attribute FileList? files;

    /// @see https://html.spec.whatwg.org/#dom-input-formaction
    KRYS_NODISCARD ExceptionOr<USVString> FormAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formaction
    ExceptionOr<void> FormAction(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formenctype
    KRYS_NODISCARD DOMString FormEnctype() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formenctype
    void FormEnctype(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formmethod
    KRYS_NODISCARD DOMString FormMethod() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formmethod
    void FormMethod(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formnovalidate
    KRYS_NODISCARD bool FormNoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formtarget
    KRYS_NODISCARD DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formtarget
    void FormTarget(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-indeterminate
    KRYS_NODISCARD bool Indeterminate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-indeterminate
    void Indeterminate(bool value) noexcept;

    // TODO(HTMLINPUT, HTML): Implement List().
    // readonly attribute HTMLDataListElement? list;

    /// @see https://html.spec.whatwg.org/#dom-input-max
    KRYS_NODISCARD DOMString Max() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-max
    void Max(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-maxlength
    KRYS_NODISCARD int32 MaxLength() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-maxlength
    void MaxLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-min
    KRYS_NODISCARD DOMString Min() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-min
    void Min(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-minlength
    KRYS_NODISCARD int32 MinLength() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-minlength
    void MinLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-multiple
    KRYS_NODISCARD bool Multiple() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-pattern
    KRYS_NODISCARD DOMString Pattern() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-pattern
    void Pattern(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-placeholder
    KRYS_NODISCARD DOMString Placeholder() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-placeholder
    void Placeholder(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-readonly
    KRYS_NODISCARD bool ReadOnly() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-readonly
    void ReadOnly(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-size
    KRYS_NODISCARD uint32 Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-size
    void Size(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-src
    KRYS_NODISCARD ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-src
    ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-step
    KRYS_NODISCARD DOMString Step() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-step
    void Step(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-type
    void Type(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultvalue
    KRYS_NODISCARD DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultvalue
    void DefaultValue(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-value
    void Value(DOMString &&value) noexcept;

    // TODO(HTMLINPUT, HTML): Implement ValueAsDate.
    // attribute object? valueAsDate;

    // TODO(HTMLINPUT, HTML): Implement ValueAsNumber.
    // attribute unrestricted double valueAsNumber;

    /// @see https://html.spec.whatwg.org/#dom-input-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-width
    void Width(uint32 value) noexcept;

    // TODO(HTMLINPUT, HTML): Implement StepUp() and StepDown().
    // undefined stepUp(optional long n = 1);
    // undefined stepDown(optional long n = 1);
    //
    // TODO(HTMLINPUT, HTML): Implement validity API.
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(DOMString error);
    //
    // TODO(HTMLINPUT, HTML): Implement selection API.
    // readonly attribute NodeList? labels;
    //
    // undefined select();
    // attribute unsigned long? selectionStart;
    // attribute unsigned long? selectionEnd;
    // attribute DOMString? selectionDirection;
    // undefined setRangeText(DOMString replacement);
    // undefined setRangeText(DOMString replacement, unsigned long start, unsigned long end, optional
    // SelectionMode selectionMode = "preserve");
    // undefined setSelectionRange(unsigned long start, unsigned long end, optional DOMString direction);
    //
    // undefined showPicker();

#pragma endregion

#pragma region HTMLInputElement Obsolete members - https://html.spec.whatwg.org/#HTMLInputElement-partial

    /// @see https://html.spec.whatwg.org/#dom-input-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-usemap
    void UseMap(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLInputElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLInputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();