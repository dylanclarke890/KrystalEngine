#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/FormController.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/StepRange.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Booey/Platform/Decimal.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class ChromeClient;
  class DragData;
  class RenderElement;
  class RenderStyle;
}

namespace krys::boo::dom
{
  class BeforeTextInsertedEvent;
  class Event;
  class KeyboardEvent;
  class MouseEvent;
  class Node;
}

namespace krys::boo::html
{
  class DOMFormData;
  class HTMLElement;
  class HTMLInputElement;
  class TextControlInnerTextElement;
  struct InputElementClickState;

  enum class WasSetByJavaScript : bool
  {
    No,
    Yes
  };

  enum class InputTypeKind : uint8
  {
    Button,
    Checkbox,
    Color,
    Date,
    DateTimeLocal,
    Email,
    File,
    Hidden,
    Image,
    Month,
    Number,
    Password,
    Radio,
    Range,
    Reset,
    Search,
    Submit,
    Telephone,
    Time,
    URL,
    Week,
    Text,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::InputTypeKind, 22uz);

namespace krys::boo::html
{
  /// @brief The type-specific part of an input element: its value's rules, validity, form data, events,
  /// user agent shadow tree and renderer. Only HTMLInputElement uses it.
  // FIXME: The switch control, datalists (shouldRespectListAttribute), and the types' own focus rules
  // (isKeyboardFocusable, isMouseFocusable), which the focus model decides.
  class InputType : public RefCounted<InputType>
  {
  public:
    using Type = InputTypeKind;

    enum class ShouldCallBaseEventHandler : bool
    {
      No,
      Yes
    };

  private:
    const Type _type;
    bool _hasCreatedShadowSubtree {false};
    // Null once the input type is no longer associated with an element (the element died or changed type).
    HTMLInputElement *_element;

  public:
    /// @brief The input type @p typeName names for @p element, or null if it's @p currentInputType's type.
    /// Unknown types are text.
    KRYS_NODISCARD static RefPtr<InputType> CreateIfDifferent(HTMLInputElement &element,
                                                              dom::DOMStringView typeName,
                                                              InputType *currentInputType = nullptr) noexcept;

    virtual ~InputType() noexcept;

    void DetachFromElement() noexcept
    {
      _element = nullptr;
    }

    KRYS_NODISCARD virtual dom::DOMStringView FormControlType() const noexcept = 0;

    KRYS_NODISCARD bool IsValidValue(dom::DOMStringView value) const noexcept;

    // Type query functions. A virtual function that asks the input type to do the work, or a query that asks
    // a higher-level question, is better than one of these.
    KRYS_NODISCARD bool IsCheckbox() const noexcept
    {
      return _type == Type::Checkbox;
    }

    KRYS_NODISCARD bool IsColorControl() const noexcept
    {
      return _type == Type::Color;
    }

    KRYS_NODISCARD bool IsDateField() const noexcept
    {
      return _type == Type::Date;
    }

    KRYS_NODISCARD bool IsDateTimeLocalField() const noexcept
    {
      return _type == Type::DateTimeLocal;
    }

    KRYS_NODISCARD bool IsEmailField() const noexcept
    {
      return _type == Type::Email;
    }

    KRYS_NODISCARD bool IsFileUpload() const noexcept
    {
      return _type == Type::File;
    }

    KRYS_NODISCARD bool IsHiddenType() const noexcept
    {
      return _type == Type::Hidden;
    }

    KRYS_NODISCARD bool IsImageButton() const noexcept
    {
      return _type == Type::Image;
    }

    KRYS_NODISCARD bool IsMonthField() const noexcept
    {
      return _type == Type::Month;
    }

    KRYS_NODISCARD bool IsNumberField() const noexcept
    {
      return _type == Type::Number;
    }

    KRYS_NODISCARD bool IsPasswordField() const noexcept
    {
      return _type == Type::Password;
    }

    KRYS_NODISCARD bool IsRadioButton() const noexcept
    {
      return _type == Type::Radio;
    }

    KRYS_NODISCARD bool IsRangeControl() const noexcept
    {
      return _type == Type::Range;
    }

    KRYS_NODISCARD bool IsSearchField() const noexcept
    {
      return _type == Type::Search;
    }

    KRYS_NODISCARD bool IsSubmitButton() const noexcept
    {
      return _type == Type::Submit || _type == Type::Image;
    }

    KRYS_NODISCARD bool IsTelephoneField() const noexcept
    {
      return _type == Type::Telephone;
    }

    KRYS_NODISCARD bool IsTimeField() const noexcept
    {
      return _type == Type::Time;
    }

    KRYS_NODISCARD bool IsURLField() const noexcept
    {
      return _type == Type::URL;
    }

    KRYS_NODISCARD bool IsWeekField() const noexcept
    {
      return _type == Type::Week;
    }

    KRYS_NODISCARD bool IsTextButton() const noexcept;
    KRYS_NODISCARD bool IsTextField() const noexcept;
    KRYS_NODISCARD bool IsTextType() const noexcept;
    KRYS_NODISCARD bool IsCheckable() const noexcept;
    KRYS_NODISCARD bool IsSteppable() const noexcept;
    KRYS_NODISCARD bool SupportsValidation() const noexcept;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    /// @brief Whether the type has a user agent shadow tree: all but the buttons, checkboxes, radio buttons
    /// and hidden inputs.
    KRYS_NODISCARD bool NeedsShadowSubtree() const noexcept;

    KRYS_NODISCARD bool HasCreatedShadowSubtree() const noexcept
    {
      return _hasCreatedShadowSubtree;
    }

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept;
    KRYS_NODISCARD bool IsLabelable() const noexcept;
    KRYS_NODISCARD bool IsEnumeratable() const noexcept;

    /// @brief Whether the input has activation behavior of its own (a click checks it, submits its form,
    /// resets it or opens its picker).
    KRYS_NODISCARD bool HasActivationBehavior() const noexcept;

    // Form value functions.
    KRYS_NODISCARD virtual bool ShouldSaveAndRestoreFormControlState() const noexcept;
    /// @brief The input's state to restore: its value, if it isn't its `value` attribute's.
    KRYS_NODISCARD virtual FormControlState SaveFormControlState() const noexcept;
    virtual void RestoreFormControlState(const FormControlState &state) noexcept;
    KRYS_NODISCARD virtual bool IsFormDataAppendable() const noexcept;
    virtual bool AppendFormData(DOMFormData &formData) const noexcept;

    /// @brief Takes the files of @p dragData dropped on the input.
    /// @return Whether it took any.
    virtual bool ReceiveDroppedFiles(const DragData &dragData) noexcept;

    // DOM property functions.
    /// @brief Checked last, if both the dirty value and the value attribute are missing.
    KRYS_NODISCARD virtual Maybe<dom::DOMString> FallbackValue() const noexcept;
    /// @brief Checked after even FallbackValue, only by HTMLInputElement::ValueWithDefault.
    KRYS_NODISCARD virtual Maybe<dom::DOMString> DefaultValue() const noexcept;
    /// @brief Milliseconds since the epoch, or NaN.
    KRYS_NODISCARD virtual double ValueAsDate() const noexcept;
    virtual dom::ExceptionOr<void> SetValueAsDate(double value) const noexcept;
    KRYS_NODISCARD virtual double ValueAsDouble() const noexcept;
    virtual dom::ExceptionOr<void> SetValueAsDouble(double value,
                                                    TextFieldEventBehavior eventBehavior) const noexcept;
    virtual dom::ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                                     TextFieldEventBehavior eventBehavior) const noexcept;

    // Validation functions.
    KRYS_NODISCARD virtual dom::DOMString ValidationMessage() const noexcept;

    KRYS_NODISCARD virtual bool TypeMismatchFor(dom::DOMStringView) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool SupportsRequired() const noexcept;

    KRYS_NODISCARD virtual bool ValueMissing(dom::DOMStringView) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool HasBadInput() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool PatternMismatch(dom::DOMStringView) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool RangeUnderflow(dom::DOMStringView value) const noexcept;
    KRYS_NODISCARD bool RangeOverflow(dom::DOMStringView value) const noexcept;
    KRYS_NODISCARD bool IsInRange(dom::DOMStringView value) const noexcept;
    KRYS_NODISCARD bool IsOutOfRange(dom::DOMStringView value) const noexcept;
    KRYS_NODISCARD virtual Decimal DefaultValueForStepUp() const noexcept;
    KRYS_NODISCARD double Minimum() const noexcept;
    KRYS_NODISCARD double Maximum() const noexcept;
    KRYS_NODISCARD bool StepMismatch(dom::DOMStringView value) const noexcept;
    KRYS_NODISCARD virtual bool GetAllowedValueStep(Decimal *step) const noexcept;
    KRYS_NODISCARD virtual StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept;
    virtual dom::ExceptionOr<void> StepUp(int n) noexcept;
    /// @brief Steps the value @p n times as the user does (the arrow keys, the spin button): an empty or
    /// invalid value starts from the default, and a value off the step snaps to it first.
    virtual void StepUpFromRenderer(int n) noexcept;
    KRYS_NODISCARD virtual dom::DOMString BadInputText() const noexcept;
    KRYS_NODISCARD virtual dom::DOMString TypeMismatchText() const noexcept;
    KRYS_NODISCARD virtual dom::DOMString ValueMissingText() const noexcept;
    KRYS_NODISCARD virtual bool CanSetStringValue() const noexcept;
    KRYS_NODISCARD virtual dom::DOMString VisibleValue() const noexcept;
    /// @brief @p proposedValue as the user sees it, in the element's locale.
    KRYS_NODISCARD virtual dom::DOMString LocalizeValue(dom::DOMStringView proposedValue) const noexcept;

    /// @brief Type check for the current value. Some types do nothing here, though TypeMismatchFor() does
    /// something for them because of value sanitization.
    KRYS_NODISCARD virtual bool TypeMismatch() const noexcept
    {
      return false;
    }

    /// @brief Only HTMLInputElement::SanitizeValue() calls this.
    /// @see https://html.spec.whatwg.org/#value-sanitization-algorithm
    KRYS_NODISCARD virtual dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept;

    // Event handlers.
    virtual void HandleMouseDownEvent(dom::MouseEvent &) noexcept
    {
    }
    virtual void HandleClickEvent(dom::MouseEvent &) noexcept
    {
    }

    virtual void WillDispatchClick(InputElementClickState &) noexcept
    {
    }

    virtual void DidDispatchClick(dom::Event &, const InputElementClickState &) noexcept
    {
    }

    /// @brief What a click (or a press of Enter or Space standing in for one) does: submit the form, reset
    /// it, or open a picker.
    virtual void HandleActivationBehavior(dom::Event &) noexcept
    {
    }

    virtual ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept;
    virtual void HandleKeypressEvent(dom::KeyboardEvent &event) noexcept;
    virtual void HandleKeyupEvent(dom::KeyboardEvent &event) noexcept;
    virtual void HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &) noexcept
    {
    }
    virtual void ForwardEvent(dom::Event &) noexcept
    {
    }

    virtual void HandleFocusEvent(dom::Node * /* oldFocusedNode */) noexcept
    {
    }
    virtual void HandleBlurEvent() noexcept
    {
    }
    virtual void SubtreeHasChanged() noexcept;
    virtual void ElementDidBlur() noexcept
    {
    }

    /// @brief Shows the type's picker (a date or colour chooser), if it has one.
    virtual void ShowPicker() noexcept
    {
    }

    /// @brief Called when the element's renderers are torn down: closes the picker.
    virtual void Detach() noexcept
    {
    }

    /// @brief Whether the type's picker is showing.
    KRYS_NODISCARD virtual bool IsPresentingAttachedView() const noexcept
    {
      return false;
    }

    // Shadow tree handling.

    void CreateShadowSubtreeIfNeeded() noexcept;
    virtual void CreateShadowSubtree() noexcept
    {
    }
    virtual void RemoveShadowSubtree() noexcept;

    KRYS_NODISCARD virtual HTMLElement *ContainerElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual HTMLElement *InnerBlockElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual RefPtr<TextControlInnerTextElement> InnerTextElement() const noexcept;
    KRYS_NODISCARD virtual HTMLElement *InnerSpinButtonElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual HTMLElement *SliderThumbElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual HTMLElement *SliderTrackElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual HTMLElement *ResultsButtonElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual HTMLElement *CancelButtonElement() const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual HTMLElement *PlaceholderElement() const noexcept
    {
      return nullptr;
    }
    RefPtr<TextControlInnerTextElement> InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept;

    KRYS_NODISCARD virtual RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept;

    virtual void UpdateInnerTextValue() noexcept
    {
    }
    virtual void UpdatePlaceholderText() noexcept
    {
    }

    /// @brief Sets @p preferredSize to the field's size in characters.
    /// @return Whether the size includes the field's decorations.
    KRYS_NODISCARD virtual bool SizeShouldIncludeDecoration(int32 defaultSize,
                                                            int32 &preferredSize) const noexcept;
    KRYS_NODISCARD virtual float DecorationWidth(float inputWidth) const noexcept;

    // Helpers for event handlers.
    KRYS_NODISCARD virtual bool ShouldSubmitImplicitly(dom::Event &event) noexcept;
    KRYS_NODISCARD virtual bool CanBeSuccessfulSubmitButton() noexcept;

    // Miscellaneous functions.
    KRYS_NODISCARD virtual bool RendererIsNeeded() noexcept;
    KRYS_NODISCARD virtual bool ShouldSendChangeEventAfterCheckedChanged() noexcept;
    KRYS_NODISCARD virtual bool StoresValueSeparateFromAttribute() noexcept;
    virtual void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                          TextFieldEventBehavior eventBehavior,
                          TextControlSetValueSelection selection) noexcept;
    KRYS_NODISCARD virtual bool ShouldRespectHeightAndWidthAttributes() noexcept;
    KRYS_NODISCARD virtual bool SupportsPlaceholder() const noexcept;
    KRYS_NODISCARD virtual bool IsEmptyValue() const noexcept;
    KRYS_NODISCARD virtual bool SupportsReadOnly() const noexcept;
    KRYS_NODISCARD virtual bool ShouldUseInputMethod() const noexcept;

    virtual void AttributeChanged(dom::DOMStringAtom) noexcept
    {
    }

    virtual void DisabledStateChanged() noexcept
    {
    }

    virtual void ReadOnlyStateChanged() noexcept
    {
    }

    virtual void RequiredStateChanged() noexcept
    {
    }

    KRYS_NODISCARD virtual bool MatchesIndeterminatePseudoClass() const noexcept;
    KRYS_NODISCARD virtual bool SupportsSelectionAPI() const noexcept;
    KRYS_NODISCARD virtual bool DirAutoUsesValue() const noexcept;

    virtual void WillUpdateCheckedness(bool /* nowChecked */, WasSetByJavaScript) noexcept
    {
    }

    /// @brief Parses @p string for the type, or returns @p defaultValue; NaN or infinity only if
    /// @p defaultValue is.
    KRYS_NODISCARD virtual Decimal ParseToNumber(dom::DOMStringView string,
                                                 const Decimal &defaultValue) const noexcept;

    /// @brief @p value as a string for the type: empty for NaN or infinity. Not for types without
    /// valueAsNumber.
    KRYS_NODISCARD virtual dom::DOMString Serialize(const Decimal &value) const noexcept;

    /// @brief The width and height of an image button; zero for other types.
    KRYS_NODISCARD virtual uint32 Height() const noexcept;
    KRYS_NODISCARD virtual uint32 Width() const noexcept;

    KRYS_NODISCARD bool IsInvalid(dom::DOMStringView value) const noexcept;
    void DispatchSimulatedClickIfActive(dom::KeyboardEvent &event) const noexcept;

    KRYS_NODISCARD virtual DateComponentsType DateType() const noexcept;
    KRYS_NODISCARD virtual Maybe<dom::DOMString> ResultForDialogSubmit() const noexcept;

  protected:
    InputType(Type type, HTMLInputElement &element) noexcept : _type(type), _element(&element)
    {
    }

    KRYS_NODISCARD HTMLInputElement *Element() const noexcept
    {
      return _element;
    }

    /// @brief The chrome client of the element's page, which makes pickers; null without a page.
    KRYS_NODISCARD ChromeClient *GetChromeClient() const noexcept;

    KRYS_NODISCARD Decimal ParseToNumberOrNaN(dom::DOMStringView string) const noexcept;

    /// @brief The step base, following the HTML algorithm.
    KRYS_NODISCARD Decimal FindStepBase(const Decimal &defaultValue) const noexcept;

  private:
    /// @brief Adds the step times @p count to the current value, for StepUp() and StepDown().
    dom::ExceptionOr<void> ApplyStep(int count, AnyStepHandling anyStepHandling,
                                     TextFieldEventBehavior eventBehavior) noexcept;
  };
}
