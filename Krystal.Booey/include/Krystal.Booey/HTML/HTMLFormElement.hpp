#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlformelement
  class HTMLFormElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFormElement);

  private:
    UniquePtr<dom::DOMTokenList> _relList;

  public:
    HTMLFormElement(dom::Document &document) noexcept;

#pragma region HTMLFormElement - https://html.spec.whatwg.org/#htmlformelement

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    KRYS_NODISCARD dom::DOMString AcceptCharset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    void AcceptCharset(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Action() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    dom::ExceptionOr<void> Action(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    KRYS_NODISCARD dom::DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    void Autocomplete(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    KRYS_NODISCARD dom::DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    void EncType(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    KRYS_NODISCARD dom::DOMString Encoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    void Encoding(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    KRYS_NODISCARD dom::DOMString Method() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    void Method(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    KRYS_NODISCARD bool NoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    void NoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    void Target(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    // TODO(HTMLFORMELEMENT, HTML): rest of form API:
    // [SameObject] readonly attribute HTMLFormControlsCollection elements;
    // readonly attribute unsigned long length;
    // getter Element (unsigned long index);
    // getter (RadioNodeList or Element) (dom::DOMString name);
    //
    // undefined submit();
    // undefined requestSubmit(optional HTMLElement? submitter = null);
    // [CEReactions] undefined reset();
    // boolean checkValidity();
    // boolean reportValidity();

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLFormElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLFormElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLFormElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
