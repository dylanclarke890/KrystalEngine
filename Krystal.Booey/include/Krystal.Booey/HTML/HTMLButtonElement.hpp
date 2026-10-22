#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlbuttonelement
  class HTMLButtonElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLButtonElement);

  private:
    enum class ButtonType : uint8
    {
      Submit,
      Reset,
      Button
    };

    ButtonType _type {ButtonType::Submit};
    bool _isActivatedSubmit {false};

  public:
    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

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

    using HTMLFormControlElement::FormNoValidate;

    /// @see https://html.spec.whatwg.org/#dom-button-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formtarget
    KRYS_NODISCARD dom::DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formtarget
    void FormTarget(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

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

#pragma endregion

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final;

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsLabelable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsSuccessfulSubmitButton() const noexcept final;
    KRYS_NODISCARD bool MatchesDefaultPseudoClass() const noexcept final;
    KRYS_NODISCARD bool IsActivatedSubmit() const noexcept final;
    void SetActivatedSubmit(bool flag) noexcept final;
    KRYS_NODISCARD bool IsSubmitButton() const noexcept final;

    /// @brief Whether the button is a submit button because its `type` says so.
    KRYS_NODISCARD bool IsExplicitlySetSubmitButton() const noexcept;

    bool AppendFormData(DOMFormData &formData) noexcept final;

    /// @see https://html.spec.whatwg.org/#the-button-element:activation-behaviour
    void ActivationBehavior(dom::Event &event) noexcept final;

    void DefaultEventHandler(dom::Event &event) noexcept final;

  protected:
    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final;

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;

  private:
    void ComputeType(dom::DOMStringView typeAttributeValue) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLButtonElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLButtonElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLButtonElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();