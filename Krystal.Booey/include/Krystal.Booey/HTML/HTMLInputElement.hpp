#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/InputType.hpp"

namespace krys::boo
{
  class FileList;
}

namespace krys::boo::dom
{
  class RadioButtonGroups;
}

namespace krys::boo::html
{
  /// @brief The state of a checkbox or radio button before a click toggled it, to restore if the click is
  /// canceled.
  struct InputElementClickState
  {
    bool Stateful {false};
    bool Checked {false};
    bool Indeterminate {false};
    bool Trusted {false};
    RefPtr<HTMLInputElement> CheckedRadioButton;
  };

  /// @see https://html.spec.whatwg.org/#htmlinputelement
  class HTMLInputElement : public HTMLTextFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLInputElement);

  public:
    /// @brief The longest value a text field holds; longer values are truncated.
    static constexpr uint32 MaxEffectiveLength = 524'288;

  private:
    static constexpr int32 DefaultSize = 20;

    enum AutoCompleteSetting : uint8
    {
      Uninitialized,
      On,
      Off
    };

    dom::DOMString _name;
    Maybe<dom::DOMString> _valueIfDirty;
    uint32 _size {DefaultSize};
    bool _isChecked : 1 {false};
    bool _dirtyCheckednessFlag : 1 {false};
    bool _isDefaultChecked : 1 {false};
    bool _isIndeterminate : 1 {false};
    bool _isActivatedSubmit : 1 {false};
    bool _wasModifiedByUser : 1 {false};
    bool _hasPendingUserAgentShadowTreeUpdate : 1 {false};
    /// @brief Whether the parser made the input and hasn't finished its children.
    bool _parsingInProgress : 1 {false};
    /// @brief Whether the input was given its saved state.
    bool _stateRestored : 1 {false};
    bool _valueAttributeWasUpdatedAfterParsing : 1 {false};
    bool _canReceiveDroppedFiles : 1 {false};
    AutoCompleteSetting _autocomplete {Uninitialized};
    RefPtr<InputType> _inputType;
    InputElementClickState _clickHandlingState;

  public:
    /// @brief Only text fields can start a selection.
    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    HTMLInputElement(dom::Document &document) noexcept;
    ~HTMLInputElement() noexcept override;

#pragma region HTMLInputElement - https://html.spec.whatwg.org/#htmlinputelement

    /// @see https://html.spec.whatwg.org/#dom-input-accept
    KRYS_NODISCARD dom::DOMString Accept() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-accept
    void Accept(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alpha
    KRYS_NODISCARD bool Alpha() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alpha
    void Alpha(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alt
    KRYS_NODISCARD dom::DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-alt
    void Alt(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-autocomplete
    KRYS_NODISCARD dom::DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-autocomplete
    void Autocomplete(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultchecked
    KRYS_NODISCARD bool DefaultChecked() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultchecked
    void DefaultChecked(bool value) noexcept;

    /// @brief The checkedness.
    /// @see https://html.spec.whatwg.org/#dom-input-checked
    KRYS_NODISCARD bool Checked() const noexcept
    {
      return _isChecked;
    }

    /// @see https://html.spec.whatwg.org/#dom-input-checked
    void Checked(bool value) noexcept
    {
      SetChecked(value);
    }

    /// @brief Sets the checkedness, and makes it dirty.
    void SetChecked(bool isChecked,
                    WasSetByJavaScript wasCheckedByJavaScript = WasSetByJavaScript::Yes) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-colorspace
    KRYS_NODISCARD dom::DOMString ColourSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-colorspace
    void ColourSpace(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-dirname
    KRYS_NODISCARD dom::DOMString DirName() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-dirname
    void DirName(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-disabled
    void Disabled(bool value) noexcept;

    /// @brief A file input's files, or null for other types.
    /// @see https://html.spec.whatwg.org/#dom-input-files
    KRYS_NODISCARD FileList *Files() noexcept;

    /// @brief Replaces a file input's files (other types ignore it); unless set by script, fires `input` and
    /// `change` if their paths changed, or else `cancel`.
    /// @see https://html.spec.whatwg.org/#dom-input-files
    void SetFiles(RefPtr<FileList> &&files,
                  WasSetByJavaScript wasSetByJavaScript = WasSetByJavaScript::No) noexcept;

    /// @brief Takes a file input's files from @p dragData, dropped on it.
    /// @return Whether it took any.
    bool ReceiveDroppedFiles(const DragData &dragData) noexcept;
    /// @brief Whether a drag over the input carries files it would take.
    KRYS_NODISCARD bool CanReceiveDroppedFiles() const noexcept
    {
      return _canReceiveDroppedFiles;
    }
    void SetCanReceiveDroppedFiles(bool canReceiveDroppedFiles) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formaction
    KRYS_NODISCARD dom::USVString FormAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formaction
    void FormAction(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formenctype
    KRYS_NODISCARD dom::DOMString FormEnctype() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formenctype
    void FormEnctype(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formmethod
    KRYS_NODISCARD dom::DOMString FormMethod() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formmethod
    void FormMethod(dom::DOMString &&value) noexcept;

    using HTMLTextFormControlElement::FormNoValidate;

    /// @see https://html.spec.whatwg.org/#dom-input-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formtarget
    KRYS_NODISCARD dom::DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formtarget
    void FormTarget(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-indeterminate
    KRYS_NODISCARD bool Indeterminate() const noexcept
    {
      return _isIndeterminate;
    }

    /// @see https://html.spec.whatwg.org/#dom-input-indeterminate
    void Indeterminate(bool value) noexcept
    {
      SetIndeterminate(value);
    }

    void SetIndeterminate(bool newValue) noexcept;

    // TODO(HTMLINPUT, HTML): Implement List().
    // readonly attribute HTMLDataListElement? list;

    /// @see https://html.spec.whatwg.org/#dom-input-max
    KRYS_NODISCARD dom::DOMString Max() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-max
    void Max(dom::DOMString &&value) noexcept;

    using HTMLTextFormControlElement::MaxLength;

    /// @see https://html.spec.whatwg.org/#dom-input-maxlength
    void MaxLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-min
    KRYS_NODISCARD dom::DOMString Min() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-min
    void Min(dom::DOMString &&value) noexcept;

    using HTMLTextFormControlElement::MinLength;

    /// @see https://html.spec.whatwg.org/#dom-input-minlength
    void MinLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-multiple
    KRYS_NODISCARD bool Multiple() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-input-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-pattern
    KRYS_NODISCARD dom::DOMString Pattern() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-pattern
    void Pattern(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-placeholder
    KRYS_NODISCARD dom::DOMString Placeholder() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-placeholder
    void Placeholder(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-readonly
    KRYS_NODISCARD bool ReadOnly() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-readonly
    void ReadOnly(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-size
    KRYS_NODISCARD uint32 Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-size
    dom::ExceptionOr<void> Size(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-src
    void Src(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-step
    KRYS_NODISCARD dom::DOMString Step() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-step
    void Step(dom::DOMString &&value) noexcept;

    /// @brief The type's name: "text" for a missing or unknown type.
    /// @see https://html.spec.whatwg.org/#dom-input-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @see https://html.spec.whatwg.org/#dom-input-type
    void Type(dom::DOMString &&value) noexcept;

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultvalue
    KRYS_NODISCARD dom::DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-defaultvalue
    void DefaultValue(dom::DOMString &&value) noexcept;

    /// @brief The value, by the type's value mode.
    /// @see https://html.spec.whatwg.org/#dom-input-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept final;

    /// @see https://html.spec.whatwg.org/#dom-input-value
    dom::ExceptionOr<void> Value(dom::DOMString &&value) noexcept
    {
      return SetValue(value);
    }

    dom::ExceptionOr<void>
      SetValue(dom::DOMStringView value,
               TextFieldEventBehavior eventBehavior = TextFieldEventBehavior::DispatchNoEvent,
               TextControlSetValueSelection selection =
                 TextControlSetValueSelection::SetSelectionToEnd) noexcept final;

    /// @brief Sets the value as the user would, firing `input` and `change`.
    void SetValueForUser(dom::DOMStringView value) noexcept;

    /// @brief The value as milliseconds since the epoch (NaN if it isn't a date), as `valueAsDate` exposes
    /// it.
    /// @see https://html.spec.whatwg.org/#dom-input-valueasdate
    KRYS_NODISCARD double ValueAsDate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-valueasdate
    dom::ExceptionOr<void> SetValueAsDate(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-valueasnumber
    KRYS_NODISCARD double ValueAsNumber() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-valueasnumber
    dom::ExceptionOr<void> SetValueAsNumber(
      double value, TextFieldEventBehavior eventBehavior = TextFieldEventBehavior::DispatchNoEvent) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-stepup
    dom::ExceptionOr<void> StepUp(int n = 1) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-stepdown
    dom::ExceptionOr<void> StepDown(int n = 1) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-cva-validationmessage
    KRYS_NODISCARD dom::DOMString ValidationMessage() const noexcept final;

    /// @brief The selection's start, or null if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    KRYS_NODISCARD Maybe<uint32> SelectionStartForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    dom::ExceptionOr<void> SetSelectionStartForBindings(Maybe<uint32> start) noexcept;

    /// @brief The selection's end, or null if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionend
    KRYS_NODISCARD Maybe<uint32> SelectionEndForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionend
    dom::ExceptionOr<void> SetSelectionEndForBindings(Maybe<uint32> end) noexcept;

    /// @brief The selection's direction, or null if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection
    KRYS_NODISCARD Maybe<dom::DOMString> SelectionDirectionForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection
    dom::ExceptionOr<void> SetSelectionDirectionForBindings(dom::DOMStringView direction) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange
    dom::ExceptionOr<void> SetSelectionRangeForBindings(uint32 start, uint32 end,
                                                        dom::DOMStringView direction = {}) noexcept;

    using HTMLTextFormControlElement::SetRangeText;

    /// @brief Throws `InvalidStateError` if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setrangetext
    dom::ExceptionOr<void> SetRangeText(dom::DOMStringView replacement, uint32 start, uint32 end,
                                        dom::DOMStringView selectionMode) noexcept final;

    /// @see https://html.spec.whatwg.org/#dom-input-showpicker
    dom::ExceptionOr<void> ShowPicker() noexcept;

    /// @brief Whether the type's picker is showing.
    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept;

#pragma endregion

    // For ValidityState
    KRYS_NODISCARD bool HasBadInput() const noexcept final;
    KRYS_NODISCARD bool PatternMismatch() const noexcept final;
    KRYS_NODISCARD bool RangeUnderflow() const noexcept final;
    KRYS_NODISCARD bool RangeOverflow() const noexcept final;
    KRYS_NODISCARD bool StepMismatch() const noexcept final;
    KRYS_NODISCARD bool TooShort() const noexcept final;
    KRYS_NODISCARD bool TooLong() const noexcept final;
    KRYS_NODISCARD bool TypeMismatch() const noexcept final;
    KRYS_NODISCARD bool ValueMissing() const noexcept final;
    KRYS_NODISCARD bool ComputeValidity() const noexcept final;

    /// @brief The minimum of a date, number or range input.
    KRYS_NODISCARD double Minimum() const noexcept;

    /// @brief The maximum of a date, number or range input; never less than its minimum.
    KRYS_NODISCARD double Maximum() const noexcept;

    /// @brief Sets @p step to the allowed value step, if there is one.
    KRYS_NODISCARD bool GetAllowedValueStep(Decimal *step) const noexcept;
    KRYS_NODISCARD StepRange CreateStepRange(AnyStepHandling anyStepHandling) const noexcept;

    KRYS_NODISCARD InputType &GetInputType() const noexcept
    {
      return *_inputType;
    }

    /// @brief Whether the user can step the value up and down.
    KRYS_NODISCARD bool IsSteppable() const noexcept;
    KRYS_NODISCARD bool IsTextButton() const noexcept;
    KRYS_NODISCARD bool IsRadioButton() const noexcept;
    KRYS_NODISCARD bool IsTextField() const noexcept final;
    KRYS_NODISCARD bool ShouldUseInputMethod() noexcept final;
    KRYS_NODISCARD bool IsSearchField() const noexcept;
    KRYS_NODISCARD bool IsInputTypeHidden() const noexcept;
    KRYS_NODISCARD bool IsPasswordField() const noexcept;
    KRYS_NODISCARD bool IsCheckbox() const noexcept;
    KRYS_NODISCARD bool IsRangeControl() const noexcept;
    KRYS_NODISCARD bool IsColorControl() const noexcept;
    KRYS_NODISCARD bool IsText() const noexcept;
    KRYS_NODISCARD bool IsTextType() const noexcept;
    KRYS_NODISCARD bool IsEmailField() const noexcept;
    KRYS_NODISCARD bool IsFileUpload() const noexcept;
    KRYS_NODISCARD bool IsImageButton() const noexcept;
    KRYS_NODISCARD bool IsNumberField() const noexcept;
    KRYS_NODISCARD bool IsSubmitButton() const noexcept final;
    KRYS_NODISCARD bool IsTelephoneField() const noexcept;
    KRYS_NODISCARD bool IsURLField() const noexcept;
    KRYS_NODISCARD bool IsDateField() const noexcept;
    KRYS_NODISCARD bool IsDateTimeLocalField() const noexcept;
    KRYS_NODISCARD bool IsMonthField() const noexcept;
    KRYS_NODISCARD bool IsTimeField() const noexcept;
    KRYS_NODISCARD bool IsWeekField() const noexcept;
    KRYS_NODISCARD DateComponentsType DateType() const noexcept;

    KRYS_NODISCARD bool MatchesCheckedPseudoClass() const noexcept;
    KRYS_NODISCARD bool MatchesIndeterminatePseudoClass() const noexcept final;
    void SetDefaultCheckedState(bool isDefaultChecked) noexcept;

    /// @brief Whether @p value would be a valid value; not for types without a string value (checkboxes and
    /// radio buttons).
    KRYS_NODISCARD bool IsValidValue(dom::DOMStringView value) const noexcept;

    KRYS_NODISCARD bool HasDirtyValue() const noexcept
    {
      return _valueIfDirty.has_value();
    }

    KRYS_NODISCARD Maybe<dom::DOMString> SanitizeValue(Maybe<dom::DOMStringView> value) const noexcept;

    /// @brief The value as the input shows it.
    KRYS_NODISCARD dom::DOMString VisibleValue() const noexcept;

    /// @brief The value, or the type's default if there's none (a submit button's label, say).
    KRYS_NODISCARD dom::DOMString ValueWithDefault() const noexcept;

    KRYS_NODISCARD bool IsActivatedSubmit() const noexcept final;
    void SetActivatedSubmit(bool flag) noexcept final;

    KRYS_NODISCARD uint32 EffectiveMaxLength() const noexcept;

    /// @brief The radio buttons in this one's group, in tree order.
    KRYS_NODISCARD List<Ref<HTMLInputElement>> RadioButtonGroup() const noexcept;
    KRYS_NODISCARD RefPtr<HTMLInputElement> CheckedRadioButtonForGroup() const noexcept;

    /// @brief The groups this radio button is in: its form's, or its tree's; null if it isn't a radio button
    /// or is in neither.
    KRYS_NODISCARD dom::RadioButtonGroups *RadioButtonGroups() const noexcept;

    // Functions for InputType classes.
    void SetValueInternal(dom::DOMStringView sanitizedValue, TextFieldEventBehavior eventBehavior) noexcept;

    /// @brief Takes the text the user edited as the value (dirty, and modified by the user).
    void SetValueFromRenderer(dom::DOMStringView value) noexcept;

    /// @brief Caches a collapsed selection at @p caretOffset, as setting the value does while unfocused.
    void CacheSelectionInResponseToSetValue(uint32 caretOffset) noexcept
    {
      CacheSelection(caretOffset, caretOffset, TextFieldSelectionDirection::SelectionHasNoDirection);
    }

    /// @brief Tells the editor a text field stopped being edited.
    void EndEditing() noexcept;

    /// @brief Creates the type's shadow tree, which inserting the element into a document left pending.
    void UpdateUserAgentShadowTree() noexcept;

    KRYS_NODISCARD HTMLElement *ContainerElement() const noexcept;
    KRYS_NODISCARD HTMLElement *InnerBlockElement() const noexcept;
    KRYS_NODISCARD HTMLElement *InnerSpinButtonElement() const noexcept;
    KRYS_NODISCARD HTMLElement *SliderThumbElement() const noexcept;
    KRYS_NODISCARD HTMLElement *SliderTrackElement() const noexcept;
    KRYS_NODISCARD HTMLElement *ResultsButtonElement() const noexcept;
    KRYS_NODISCARD HTMLElement *CancelButtonElement() const noexcept;
    KRYS_NODISCARD HTMLElement *PlaceholderElement() const noexcept final;
    KRYS_NODISCARD RefPtr<TextControlInnerTextElement> InnerTextElement() const noexcept final;
    RefPtr<TextControlInnerTextElement> InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept final;
    KRYS_NODISCARD RenderStyle CreateInnerTextStyle(const RenderStyle &style) noexcept final;

    /// @brief Whether the field's text ends in an ellipsis: when `text-overflow` says so and it isn't
    /// focused.
    KRYS_NODISCARD bool ShouldTruncateText(const RenderStyle &style) const noexcept;

    /// @brief Sets @p preferredSize to the field's size in characters.
    /// @return Whether the size includes the field's decorations.
    KRYS_NODISCARD bool SizeShouldIncludeDecoration(int32 &preferredSize) const noexcept;
    KRYS_NODISCARD float DecorationWidth(float inputWidth) const noexcept;

    KRYS_NODISCARD bool CanHaveSelection() const noexcept
    {
      return IsTextField();
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
    void DidAttachRenderers() noexcept final;
    void DidDetachRenderers() noexcept final;
    void UpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                               SelectionRevealMode revealMode = SelectionRevealMode::Reveal) noexcept final;
    void DidBlur() noexcept final;

    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept final;
    KRYS_NODISCARD Maybe<dom::DOMString> ResultForDialogSubmit() const noexcept final;

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final;

    KRYS_NODISCARD bool CanTriggerImplicitSubmission() const noexcept final
    {
      return IsTextField();
    }

    bool AppendFormData(DOMFormData &formData) noexcept final;
    KRYS_NODISCARD bool IsSuccessfulSubmitButton() const noexcept final;
    KRYS_NODISCARD bool MatchesDefaultPseudoClass() const noexcept final;
    void Reset() noexcept final;
    KRYS_NODISCARD bool IsInRange() const noexcept final;
    KRYS_NODISCARD bool IsOutOfRange() const noexcept final;
    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final;
    KRYS_NODISCARD bool SupportsPlaceholder() const noexcept final;
    KRYS_NODISCARD bool IsEnumeratable() const noexcept final;
    KRYS_NODISCARD bool IsLabelable() const noexcept final;
    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final;

    KRYS_NODISCARD bool IsOptionalFormControl() const noexcept final
    {
      return !IsRequiredFormControl();
    }

    KRYS_NODISCARD bool IsRequiredFormControl() const noexcept final;

    KRYS_NODISCARD bool ShouldAutocomplete() const noexcept final;

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &style) noexcept final;

    void FinishParsingChildren() noexcept final;

    /// @brief The parser made the input: its `checked` attribute waits until its children are parsed, in case
    /// its saved state is restored.
    void SetCreatedByParser() noexcept
    {
      _parsingInProgress = true;
    }

    /// @brief Whether the `value` attribute was set after the parser made the input.
    KRYS_NODISCARD bool ValueAttributeWasUpdatedAfterParsing() const noexcept
    {
      return _valueAttributeWasUpdatedAfterParsing;
    }

    KRYS_NODISCARD bool ShouldSaveAndRestoreFormControlState() const noexcept final;
    KRYS_NODISCARD FormControlState SaveFormControlState() const noexcept final;
    void RestoreFormControlState(const FormControlState &state) noexcept final;

    void LegacyPreActivationBehavior(dom::Event &event) noexcept final;
    void ActivationBehavior(dom::Event &event) noexcept final;
    void LegacyCanceledActivationBehavior(dom::Event &event) noexcept final;
    void DefaultEventHandler(dom::Event &event) noexcept final;

#pragma region HTMLInputElement Obsolete members - https://html.spec.whatwg.org/#HTMLInputElement-partial

    /// @see https://html.spec.whatwg.org/#dom-input-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-usemap
    KRYS_NODISCARD dom::DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-usemap
    void UseMap(dom::DOMString &&value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    void OnInsert() noexcept final;
    void OnPostConnection() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;
    void OnClone(dom::Node &copy, bool subtree) noexcept final;

    KRYS_NODISCARD bool IsEmptyValue() const noexcept final;
    void UpdatePlaceholderText() noexcept final;
    void SubtreeHasChanged() noexcept final;
    void HandleFocusEvent(dom::Node *oldFocusedNode) noexcept final;
    void HandleBlurEvent() noexcept final;

    void WillChangeForm() noexcept final;
    void DidChangeForm() noexcept final;
    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final;
    void RequiredStateChanged() noexcept final;
    void DisabledStateChanged() noexcept final;
    void ReadOnlyStateChanged() noexcept final;

  private:
    /// @brief The value, or null if the type has none (no dirty value, value attribute or fallback).
    KRYS_NODISCARD Maybe<dom::DOMString> ValueOrNull() const noexcept;

    KRYS_NODISCARD bool SupportsMinLength() const noexcept
    {
      return IsTextType();
    }

    KRYS_NODISCARD bool SupportsMaxLength() const noexcept
    {
      return IsTextType();
    }

    KRYS_NODISCARD bool TooShort(dom::DOMStringView value, NeedsToCheckDirtyFlag check) const noexcept;
    KRYS_NODISCARD bool TooLong(dom::DOMStringView value, NeedsToCheckDirtyFlag check) const noexcept;

    void UpdateType(dom::DOMStringView typeAttributeValue) noexcept;
    void RunPostTypeUpdateTasks() noexcept;
    void SetDefaultSelectionAfterFocus(dom::SelectionRestorationMode restorationMode,
                                       SelectionRevealMode revealMode) noexcept;
    void InvalidateStyleOnFocusChangeIfNeeded() noexcept;
    void MaxLengthAttributeChanged(Maybe<dom::DOMStringView> newValue) noexcept;
    void MinLengthAttributeChanged(Maybe<dom::DOMStringView> newValue) noexcept;
    void UpdateValueIfNeeded() noexcept;
    void AddToRadioButtonGroup() noexcept;
    void RemoveFromRadioButtonGroup() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLInputElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLInputElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLInputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();