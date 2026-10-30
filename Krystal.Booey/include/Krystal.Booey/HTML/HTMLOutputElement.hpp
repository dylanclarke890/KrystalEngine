#pragma once

#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmloutputelement
  class HTMLOutputElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOutputElement);

  private:
    UniquePtr<dom::DOMTokenList> _htmlFor;
    Maybe<dom::DOMString> _defaultValueOverride;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    HTMLOutputElement(dom::Document &document) noexcept;
    ~HTMLOutputElement() noexcept override;

#pragma region HTMLOutputElement - https://html.spec.whatwg.org/#htmloutputelement

    /// @see https://html.spec.whatwg.org/#dom-output-htmlfor
    KRYS_NODISCARD dom::DOMTokenList &HtmlFor() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-output-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @see https://html.spec.whatwg.org/#dom-output-defaultvalue
    KRYS_NODISCARD dom::DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-defaultvalue
    void DefaultValue(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-output-value
    void Value(dom::DOMString &&value) noexcept;

#pragma endregion

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final
    {
      return u8"output";
    }

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsLabelable() const noexcept final
    {
      return true;
    }

    void Reset() noexcept final;

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final
    {
      return false;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLOutputElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLOutputElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLOutputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
