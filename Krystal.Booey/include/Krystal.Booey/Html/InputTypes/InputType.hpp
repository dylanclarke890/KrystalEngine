/*
 * Copyright (C) 2010-2021 Google Inc. All rights reserved.
 * Copyright (C) 2011-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Html/InputTypes/StepRange.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Decimal.hpp"

namespace krys::boo
{
  class BeforeTextInsertedEvent;
  class BooeyClient;
  class DragData;
  class Event;
  class HTMLElement;
  class HTMLInputElement;
  class KeyboardEvent;
  class MouseEvent;
  class Node;
  class RenderElement;
  class RenderStyle;
}

namespace krys::boo::html
{
  class DOMFormData;
  class TextControlInnerTextElement;

  struct InputElementClickState;

  enum class WasSetByJavaScript : bool
  {
    No,
    Yes,
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
    Url,
    Week,
    Text,
  };

  consteval InputTypeKind MaxEnumValue(InputTypeKind) noexcept
  {
    return InputTypeKind::Text;
  }

  /// @brief The type-specific part of an input element: its value's rules, validity, form data, events,
  /// user agent shadow tree and renderer. Only HTMLInputElement uses it.
  // FIXME: The switch control, and the types' own focus rules
  // (isKeyboardFocusable, isMouseFocusable), which the focus model decides.
  class InputType : public RefCounted<InputType>
  {
  public:
    using Type = InputTypeKind;

    enum class ShouldCallBaseEventHandler : bool
    {
      No,
      Yes,
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
                                                              DOMStringView typeName,
                                                              InputType *currentInputType = nullptr) noexcept;

    virtual ~InputType() noexcept;

    void DetachFromElement() noexcept
    {
      _element = nullptr;
    }

    KRYS_NODISCARD virtual DOMStringView FormControlType() const noexcept = 0;

    KRYS_NODISCARD bool IsValidValue(DOMStringView value) const noexcept;

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

    KRYS_NODISCARD bool IsUrlField() const noexcept
    {
      return _type == Type::Url;
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
    KRYS_NODISCARD virtual bool IsFormDataAppendable() const noexcept;

    virtual bool AppendFormData(DOMFormData &formData) const noexcept;

    /// @brief Takes the files of @p dragData dropped on the input.
    /// @return Whether it took any.
    virtual bool ReceiveDroppedFiles(const DragData &dragData) noexcept;

    /// @brief A file input's text for its files, given with them by the host's chooser.
    KRYS_NODISCARD virtual DOMString DisplayString() const noexcept;

    // DOM property functions.
    /// @brief Checked last, if both the dirty value and the value attribute are missing.
    KRYS_NODISCARD virtual Maybe<DOMString> FallbackValue() const noexcept;

    /// @brief Checked after even FallbackValue, only by HTMLInputElement::ValueWithDefault.
    KRYS_NODISCARD virtual Maybe<DOMString> DefaultValue() const noexcept;

    /// @brief Milliseconds since the epoch, or NaN.
    KRYS_NODISCARD virtual double ValueAsDate() const noexcept;

    virtual ExceptionOr<void> SetValueAsDate(double value) const noexcept;

    KRYS_NODISCARD virtual double ValueAsDouble() const noexcept;

    virtual ExceptionOr<void> SetValueAsDouble(double value,
                                               TextFieldEventBehavior eventBehavior) const noexcept;

    virtual ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                                TextFieldEventBehavior eventBehavior) const noexcept;

    // Validation functions.
    KRYS_NODISCARD virtual DOMString ValidationMessage() const noexcept;

    KRYS_NODISCARD virtual bool TypeMismatchFor(DOMStringView) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool SupportsRequired() const noexcept;

    KRYS_NODISCARD virtual bool ValueMissing(DOMStringView) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool HasBadInput() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool PatternMismatch(DOMStringView) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool RangeUnderflow(DOMStringView value) const noexcept;

    KRYS_NODISCARD bool RangeOverflow(DOMStringView value) const noexcept;

    KRYS_NODISCARD bool IsInRange(DOMStringView value) const noexcept;

    KRYS_NODISCARD bool IsOutOfRange(DOMStringView value) const noexcept;

    KRYS_NODISCARD virtual Decimal DefaultValueForStepUp() const noexcept;

    KRYS_NODISCARD double Minimum() const noexcept;

    KRYS_NODISCARD double Maximum() const noexcept;

    KRYS_NODISCARD bool StepMismatch(DOMStringView value) const noexcept;

    KRYS_NODISCARD virtual bool GetAllowedValueStep(Decimal *step) const noexcept;

    KRYS_NODISCARD virtual StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept;

    virtual ExceptionOr<void> StepUp(int n) noexcept;

    /// @brief Steps the value @p n times as the user does (the arrow keys, the spin button): an empty or
    /// invalid value starts from the default, and a value off the step snaps to it first.
    virtual void StepUpFromRenderer(int n) noexcept;

    KRYS_NODISCARD virtual DOMString BadInputText() const noexcept;

    KRYS_NODISCARD virtual DOMString TypeMismatchText() const noexcept;

    KRYS_NODISCARD virtual DOMString ValueMissingText() const noexcept;

    KRYS_NODISCARD virtual bool CanSetStringValue() const noexcept;

    KRYS_NODISCARD virtual DOMString VisibleValue() const noexcept;

    /// @brief @p proposedValue as the user sees it, in the element's locale.
    KRYS_NODISCARD virtual DOMString LocalizeValue(DOMStringView proposedValue) const noexcept;

    /// @brief Type check for the current value. Some types do nothing here, though TypeMismatchFor() does
    /// something for them because of value sanitization.
    KRYS_NODISCARD virtual bool TypeMismatch() const noexcept
    {
      return false;
    }

    /// @brief Only HTMLInputElement::SanitizeValue() calls this.
    /// @see https://html.spec.whatwg.org/multipage/input.html#value-sanitization-algorithm
    KRYS_NODISCARD virtual DOMString SanitizeValue(DOMStringView proposedValue) const noexcept;

    // Event handlers.
    virtual void HandleMouseDownEvent(MouseEvent &) noexcept
    {
    }

    virtual void HandleClickEvent(MouseEvent &) noexcept
    {
    }

    virtual void WillDispatchClick(InputElementClickState &) noexcept
    {
    }

    virtual void DidDispatchClick(Event &, const InputElementClickState &) noexcept
    {
    }

    /// @brief What a click (or a press of Enter or Space standing in for one) does: submit the form, reset
    /// it, or open a picker.
    virtual void HandleActivationBehavior(Event &) noexcept
    {
    }

    virtual ShouldCallBaseEventHandler HandleKeydownEvent(KeyboardEvent &event) noexcept;

    virtual void HandleKeypressEvent(KeyboardEvent &event) noexcept;

    virtual void HandleKeyupEvent(KeyboardEvent &event) noexcept;

    virtual void HandleBeforeTextInsertedEvent(BeforeTextInsertedEvent &) noexcept
    {
    }

    virtual void ForwardEvent(Event &) noexcept
    {
    }

    virtual void HandleFocusEvent(Node * /* oldFocusedNode */) noexcept
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

    /// @brief Called when the element has taken on this type, after its type attribute changed.
    virtual void DidBecomeElementType() noexcept
    {
    }

    /// @brief Called when the element's renderers have been created.
    virtual void Attach() noexcept
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

    KRYS_NODISCARD virtual HTMLElement *DataListButtonElement() const noexcept
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
    KRYS_NODISCARD virtual bool ShouldSubmitImplicitly(Event &event) noexcept;

    KRYS_NODISCARD virtual bool CanBeSuccessfulSubmitButton() noexcept;

    // Miscellaneous functions.
    KRYS_NODISCARD virtual bool RendererIsNeeded() noexcept;

    KRYS_NODISCARD virtual bool ShouldSendChangeEventAfterCheckedChanged() noexcept;

    KRYS_NODISCARD virtual bool StoresValueSeparateFromAttribute() noexcept;

    virtual void SetValue(DOMStringView sanitizedValue, bool valueChanged,
                          TextFieldEventBehavior eventBehavior,
                          TextControlSetValueSelection selection) noexcept;

    KRYS_NODISCARD virtual bool ShouldRespectHeightAndWidthAttributes() noexcept;

    KRYS_NODISCARD virtual bool SupportsPlaceholder() const noexcept;

    KRYS_NODISCARD virtual bool IsEmptyValue() const noexcept;

    KRYS_NODISCARD virtual bool SupportsReadOnly() const noexcept;

    KRYS_NODISCARD virtual bool ShouldUseInputMethod() const noexcept;

    virtual void AttributeChanged(const DOMStringAtom &) noexcept
    {
    }

    /// @brief Whether the `list` attribute applies to the type, so its datalist gives suggestions.
    /// @see https://html.spec.whatwg.org/multipage/input.html#attr-input-list
    KRYS_NODISCARD virtual bool ShouldRespectListAttribute() const noexcept
    {
      return false;
    }

    /// @brief The element's datalist, or its suggestions, may have changed.
    virtual void DataListMayHaveChanged() noexcept
    {
    }

    /// @brief The value of the datalist suggestion closest to @p value, for a slider's thumb to snap to.
    KRYS_NODISCARD virtual Maybe<Decimal> FindClosestTickMarkValue(const Decimal &value) noexcept
    {
      (void)value;
      return {};
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
    KRYS_NODISCARD virtual Decimal ParseToNumber(DOMStringView string,
                                                 const Decimal &defaultValue) const noexcept;

    /// @brief @p value as a string for the type: empty for NaN or infinity. Not for types without
    /// valueAsNumber.
    KRYS_NODISCARD virtual DOMString Serialize(const Decimal &value) const noexcept;

    /// @brief The width and height of an image button; zero for other types.
    KRYS_NODISCARD virtual uint32 Height() const noexcept;

    KRYS_NODISCARD virtual uint32 Width() const noexcept;

    KRYS_NODISCARD bool IsInvalid(DOMStringView value) const noexcept;

    void DispatchSimulatedClickIfActive(KeyboardEvent &event) const noexcept;

    KRYS_NODISCARD virtual DateComponentsType DateType() const noexcept;

    KRYS_NODISCARD virtual Maybe<DOMString> ResultForDialogSubmit() const noexcept;

  protected:
    InputType(Type type, HTMLInputElement &element) noexcept : _type(type), _element(&element)
    {
    }

    KRYS_NODISCARD HTMLInputElement *Element() const noexcept
    {
      return _element;
    }

    /// @brief The Booey client of the element's page, which makes pickers; null without a page.
    KRYS_NODISCARD BooeyClient *GetBooeyClient() const noexcept;

    /// @brief Whether the element's window has transient activation, which a click or key needs to open a
    /// picker, as in Chrome. It isn't consumed.
    /// @see https://html.spec.whatwg.org/multipage/interaction.html#transient-activation
    KRYS_NODISCARD bool HasTransientActivation() const noexcept;

    KRYS_NODISCARD Decimal ParseToNumberOrNaN(DOMStringView string) const noexcept;

    /// @brief The step base, following the HTML algorithm.
    KRYS_NODISCARD Decimal FindStepBase(const Decimal &defaultValue) const noexcept;

  private:
    /// @brief Adds the step times @p count to the current value, for StepUp() and StepDown().
    ExceptionOr<void> ApplyStep(int count, AnyStepHandling anyStepHandling,
                                TextFieldEventBehavior eventBehavior) noexcept;
  };
}
