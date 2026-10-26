#pragma once

#include "Krystal.Booey/HTML/InputType.hpp"
#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"

namespace krys::boo::html
{
  /// @brief The base of the text-like input types and number: a field with an inner text element the user
  /// edits, and a placeholder.
  // FIXME: The caps lock indicator, the autofill button and the datalist dropdown and suggestions.
  class TextFieldInputType : public InputType, protected SpinButtonOwner
  {
  private:
    RefPtr<HTMLElement> _container;
    RefPtr<HTMLElement> _innerBlock;
    RefPtr<TextControlInnerTextElement> _innerText;
    RefPtr<HTMLElement> _placeholder;
    RefPtr<SpinButtonElement> _innerSpinButton;

  public:
    ~TextFieldInputType() noexcept override;

    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;

  protected:
    TextFieldInputType(Type type, HTMLInputElement &element) noexcept;

    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept override;
    /// @brief Steps the value with the up and down arrow keys, as the spin button would.
    void HandleKeydownEventForSpinButton(dom::KeyboardEvent &event) noexcept;
    void HandleClickEvent(dom::MouseEvent &event) noexcept final;

    KRYS_NODISCARD HTMLElement *ContainerElement() const noexcept final;
    KRYS_NODISCARD HTMLElement *InnerBlockElement() const noexcept final;
    KRYS_NODISCARD RefPtr<TextControlInnerTextElement> InnerTextElement() const noexcept final;
    KRYS_NODISCARD HTMLElement *InnerSpinButtonElement() const noexcept final;

    KRYS_NODISCARD virtual bool NeedsContainer() const noexcept;
    void CreateShadowSubtree() noexcept override;
    void RemoveShadowSubtree() noexcept override;
    void AttributeChanged(dom::DOMStringAtom name) noexcept override;
    void DisabledStateChanged() noexcept final;
    void ReadOnlyStateChanged() noexcept final;
    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final;
    KRYS_NODISCARD bool ShouldUseInputMethod() const noexcept override;
    void HandleFocusEvent(dom::Node *oldFocusedNode) noexcept final;
    void HandleBlurEvent() noexcept final;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept override;
    void UpdateInnerTextValue() noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept override;

    KRYS_NODISCARD virtual dom::DOMString
      ConvertFromVisibleValue(dom::DOMStringView visibleValue) const noexcept;
    virtual void DidSetValueByUserEdit() noexcept;

  private:
    KRYS_NODISCARD bool IsEmptyValue() const noexcept final;
    void HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &event) noexcept override;
    void ForwardEvent(dom::Event &event) noexcept final;
    KRYS_NODISCARD bool ShouldSubmitImplicitly(dom::Event &event) noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept override;
    KRYS_NODISCARD HTMLElement *PlaceholderElement() const noexcept final;
    void UpdatePlaceholderText() noexcept final;
    bool AppendFormData(DOMFormData &formData) const noexcept final;
    void SubtreeHasChanged() noexcept final;
    void ElementDidBlur() noexcept final;

    // SpinButtonOwner functions.
    void FocusAndSelectSpinButtonOwner() noexcept final;
    KRYS_NODISCARD bool ShouldSpinButtonRespondToMouseEvents() const noexcept final;
    void SpinButtonStepDown() noexcept final;
    void SpinButtonStepUp() noexcept final;

    KRYS_NODISCARD bool ShouldHaveSpinButton() const noexcept;

    enum class PreserveSelectionRange : bool
    {
      No,
      Yes
    };

    void CreateContainer(PreserveSelectionRange preserveSelection = PreserveSelectionRange::Yes) noexcept;
  };
}
