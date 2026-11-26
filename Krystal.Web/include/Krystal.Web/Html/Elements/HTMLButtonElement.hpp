#pragma once

#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Html/Elements/HTMLFormControlElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmlbuttonelement
  class HTMLButtonElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLButtonElement);

  private:
    enum class ButtonType : uint8
    {
      Submit,
      Reset,
      Button,
    };

    ButtonType _type {ButtonType::Submit};
    bool _isActivatedSubmit {false};

  public:
    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

    HTMLButtonElement(Document &document) noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;

#pragma region HTMLButtonElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmlbuttonelement

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-command
    KRYS_NODISCARD DOMString Command() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-command
    void Command(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-commandforelement
    KRYS_NODISCARD RefPtr<web::Element> CommandForElementForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-commandforelement
    void CommandForElementForBindings(web::Element *element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formaction
    KRYS_NODISCARD USVString FormAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formaction
    void FormAction(USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formenctype
    KRYS_NODISCARD DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formenctype
    void EncType(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formmethod
    KRYS_NODISCARD DOMString FormMethod() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-button-formmethod
    void FormMethod(DOMString value) noexcept;

    using HTMLFormControlElement::FormNoValidate;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-formtarget
    KRYS_NODISCARD DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-formtarget
    void FormTarget(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-button-value
    void Value(DOMString value) noexcept;

#pragma endregion

    KRYS_NODISCARD DOMString FormControlType() const noexcept final;

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

    /// @brief The element the button sends its command: its `commandfor` element.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#attr-button-commandfor
    KRYS_NODISCARD RefPtr<web::Element> CommandForElement() const noexcept;

    /// @brief The state of the button's `command` attribute.
    KRYS_NODISCARD dom::CommandType GetCommandType() const noexcept;

    bool AppendFormData(html::DOMFormData &formData) noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#the-button-element:activation-behaviour
    void ActivationBehavior(Event &event) noexcept final;

    void DefaultEventHandler(Event &event) noexcept final;

  protected:
    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept final;

  private:
    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    /// @brief Computes the type again, which in the Auto state depends on the parent.
    void UpdateTypeForParent() noexcept;

    /// @brief Sets the button's type from its `type` attribute, with the missing and invalid value defaults
    /// (button for a command button or a select's button, else submit).
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#attr-button-type
    void ComputeType(DOMStringView typeAttributeValue) noexcept;

    /// @brief Fires `command` at the button's `commandfor` element, which then carries out a built-in command
    /// unless a listener canceled it.
    void HandleCommand() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLButtonElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLButtonElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLButtonElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
