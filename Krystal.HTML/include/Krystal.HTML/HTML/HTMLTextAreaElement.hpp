#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltextareaelement
  class HTMLTextAreaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTextAreaElement);

  public:
    HTMLTextAreaElement(Document &document) noexcept;

#pragma region HTMLTextAreaElement - https://html.spec.whatwg.org/#htmltextareaelement

    /// @see https://html.spec.whatwg.org/#dom-textarea-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-autocomplete
    void Autocomplete(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-cols
    KRYS_NODISCARD uint32 Cols() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-cols
    void Cols(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-dirname
    KRYS_NODISCARD DOMString DirName() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-dirname
    void DirName(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLTEXTAREA, HTML): Implement the form API.
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-textarea-maxlength
    KRYS_NODISCARD int32 MaxLength() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-maxlength
    void MaxLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-minlength
    KRYS_NODISCARD int32 MinLength() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-minlength
    void MinLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-placeholder
    KRYS_NODISCARD DOMString Placeholder() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-placeholder
    void Placeholder(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-readonly
    KRYS_NODISCARD bool ReadOnly() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-readonly
    void ReadOnly(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-rows
    KRYS_NODISCARD uint32 Rows() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-rows
    void Rows(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-wrap
    KRYS_NODISCARD DOMString Wrap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-wrap
    void Wrap(DOMString &&value) noexcept;

    // TODO(HTMLTEXTAREA, HTML): Implement the type attribute.
    // readonly attribute DOMString type;
    // [CEReactions] attribute DOMString defaultValue;
    // attribute[LegacyNullToEmptyString] DOMString value;
    // readonly attribute unsigned long textLength;
    //
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(DOMString error);
    //
    // readonly attribute NodeList labels;
    //
    // undefined select();
    // attribute unsigned long selectionStart;
    // attribute unsigned long selectionEnd;
    // attribute DOMString selectionDirection;
    // undefined setRangeText(DOMString replacement);
    // undefined setRangeText(DOMString replacement, unsigned long start, unsigned long end,
    //                       optional SelectionMode selectionMode = "preserve");
    // undefined setSelectionRange(unsigned long start, unsigned long end, optional DOMString direction);

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTextAreaElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTextAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();