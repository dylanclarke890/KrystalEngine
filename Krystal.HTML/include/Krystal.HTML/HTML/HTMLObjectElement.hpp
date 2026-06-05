#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlobjectelement
  class HTMLObjectElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLObjectElement);

  public:
    HTMLObjectElement(Document &document) noexcept;

#pragma region HTMLObjectElement - https://html.spec.whatwg.org/#htmlobjectelement

    /// @see https://html.spec.whatwg.org/#dom-object-data
    KRYS_NODISCARD ExceptionOr<USVString> Data() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-data
    ExceptionOr<void> Data(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-type
    void Type(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-name
    void Name(DOMString &&value) noexcept;

    // TODO(HTMLObjectElement, HTML): HTMLFormElement? form();
    /// @see https://html.spec.whatwg.org/#dom-object-form
    // RefPtr<HTMLFormElement> Form() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-width
    void Width(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-height
    void Height(DOMString &&value) noexcept;

    // TODO(HTMLObjectElement, HTML):
    // readonly attribute Document? contentDocument;
    // readonly attribute WindowProxy? contentWindow;
    // Document? getSVGDocument();
    //
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(DOMString error);

#pragma endregion

#pragma region HTMLObjectElement Obsolete members - https://html.spec.whatwg.org/#HTMLObjectElement-partial

    /// @see https://html.spec.whatwg.org/#dom-object-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-archive
    KRYS_NODISCARD DOMString Archive() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-archive
    void Archive(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-code
    KRYS_NODISCARD DOMString Code() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-code
    void Code(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-declare
    KRYS_NODISCARD bool Declare() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-declare
    void Declare(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-standby
    KRYS_NODISCARD DOMString Standby() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-standby
    void Standby(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codebase
    KRYS_NODISCARD DOMString CodeBase() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codebase
    void CodeBase(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codetype
    KRYS_NODISCARD DOMString CodeType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codetype
    void CodeType(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-usemap
    void UseMap(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-border
    void Border(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLObjectElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLObjectElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();