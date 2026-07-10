#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlformelement
  class HTMLFormElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFormElement);

  private:
    UniquePtr<DOMTokenList> _relList;

  public:
    HTMLFormElement(Document &document) noexcept;

#pragma region HTMLFormElement - https://html.spec.whatwg.org/#htmlformelement

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    KRYS_NODISCARD DOMString AcceptCharset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    void AcceptCharset(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    KRYS_NODISCARD ExceptionOr<USVString> Action() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    ExceptionOr<void> Action(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    void Autocomplete(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    KRYS_NODISCARD DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    void EncType(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    KRYS_NODISCARD DOMString Encoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    void Encoding(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    KRYS_NODISCARD DOMString Method() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    void Method(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    KRYS_NODISCARD bool NoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    void NoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    void Target(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    void Rel(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    // TODO(HTMLFORMELEMENT, HTML): rest of form API:
    // [SameObject] readonly attribute HTMLFormControlsCollection elements;
    // readonly attribute unsigned long length;
    // getter Element (unsigned long index);
    // getter (RadioNodeList or Element) (DOMString name);
    //
    // undefined submit();
    // undefined requestSubmit(optional HTMLElement? submitter = null);
    // [CEReactions] undefined reset();
    // boolean checkValidity();
    // boolean reportValidity();

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLFormElement)

  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLFormElement();
  }

  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLFormElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
