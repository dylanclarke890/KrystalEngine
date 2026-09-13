#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlobjectelement
  class HTMLObjectElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLObjectElement);

  public:
    HTMLObjectElement(dom::Document &document) noexcept;

#pragma region HTMLObjectElement - https://html.spec.whatwg.org/#htmlobjectelement

    /// @see https://html.spec.whatwg.org/#dom-object-data
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Data() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-data
    dom::ExceptionOr<void> Data(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-name
    void Name(dom::DOMString &&value) noexcept;

    // TODO(HTMLObjectElement, HTML): HTMLFormElement? form();
    /// @see https://html.spec.whatwg.org/#dom-object-form
    // RefPtr<HTMLFormElement> Form() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-height
    void Height(dom::DOMString &&value) noexcept;

    // TODO(HTMLObjectElement, HTML):
    // readonly attribute Document? contentDocument;
    // readonly attribute WindowProxy? contentWindow;
    // Document? getSVGDocument();
    //
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute dom::DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(dom::DOMString error);

#pragma endregion

#pragma region HTMLObjectElement Obsolete members - https://html.spec.whatwg.org/#HTMLObjectElement-partial

    /// @see https://html.spec.whatwg.org/#dom-object-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-archive
    KRYS_NODISCARD dom::DOMString Archive() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-archive
    void Archive(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-code
    KRYS_NODISCARD dom::DOMString Code() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-code
    void Code(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-declare
    KRYS_NODISCARD bool Declare() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-declare
    void Declare(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-standby
    KRYS_NODISCARD dom::DOMString Standby() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-standby
    void Standby(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-vspace
    void VSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codebase
    KRYS_NODISCARD dom::DOMString CodeBase() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codebase
    void CodeBase(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codetype
    KRYS_NODISCARD dom::DOMString CodeType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-codetype
    void CodeType(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-usemap
    KRYS_NODISCARD dom::DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-usemap
    void UseMap(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-border
    KRYS_NODISCARD dom::DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-object-border
    void Border(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLObjectElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLObjectElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();