#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputType.hpp"

namespace krys::boo::html
{
  class RadioButtonGroups;

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

  class HTMLImageLoader;
  class ListAttributeTargetObserver;
}

namespace krys::boo
{
  class FileList;
  class HTMLDataListElement;
  class HTMLOptionElement;

  /// @see https://html.spec.whatwg.org/multipage/input.html#htmlinputelement
  class HTMLInputElement : public HTMLTextFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLInputElement);

  public:
    /// @brief The longest value a text field holds; longer values are truncated.
    constexpr static uint32 MaxEffectiveLength = 524'288;

  private:
    constexpr static int32 DefaultSize = 20;

    enum AutoCompleteSetting : uint8
    {
      Uninitialized,
      On,
      Off,
    };

    DOMString _name;
    Maybe<DOMString> _valueIfDirty;
    UniquePtr<html::HTMLImageLoader> _imageLoader;
    UniquePtr<html::ListAttributeTargetObserver> _listAttributeTargetObserver;
    uint32 _size {DefaultSize};
    bool _isChecked : 1 {false};
    bool _dirtyCheckednessFlag : 1 {false};
    bool _isDefaultChecked : 1 {false};
    bool _isIndeterminate : 1 {false};
    bool _hasNonEmptyList : 1 {false};
    bool _isActivatedSubmit : 1 {false};
    bool _wasModifiedByUser : 1 {false};
    bool _hasPendingUserAgentShadowTreeUpdate : 1 {false};
    bool _canReceiveDroppedFiles : 1 {false};
    AutoCompleteSetting _autocomplete {Uninitialized};
    RefPtr<html::InputType> _inputType;
    html::InputElementClickState _clickHandlingState;

  public:
    /// @brief Only text fields can start a selection.
    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    HTMLInputElement(Document &document) noexcept;

    ~HTMLInputElement() noexcept override;

#pragma region HTMLInputElement - https://html.spec.whatwg.org/multipage/input.html#htmlinputelement

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-accept
    KRYS_NODISCARD DOMString Accept() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-accept
    void Accept(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-alpha
    KRYS_NODISCARD bool Alpha() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-alpha
    void Alpha(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-alt
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-alt
    void Alt(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-autocomplete
    void Autocomplete(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-defaultchecked
    KRYS_NODISCARD bool DefaultChecked() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-defaultchecked
    void DefaultChecked(bool value) noexcept;

    /// @brief The checkedness.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-checked
    KRYS_NODISCARD bool Checked() const noexcept
    {
      return _isChecked;
    }

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-checked
    void Checked(bool value) noexcept
    {
      SetChecked(value);
    }

    /// @brief Sets the checkedness, and makes it dirty.
    void SetChecked(bool isChecked,
                    html::WasSetByJavaScript wasCheckedByJavaScript = html::WasSetByJavaScript::Yes) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-colorspace
    KRYS_NODISCARD DOMString ColourSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-colorspace
    void ColourSpace(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-dirname
    KRYS_NODISCARD DOMString DirName() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-dirname
    void DirName(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-disabled
    void Disabled(bool value) noexcept;

    /// @brief A file input's files, or null for other types.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-files
    KRYS_NODISCARD FileList *Files() noexcept;

    /// @brief Replaces a file input's files (other types ignore it); unless set by script, fires `input` and
    /// `change` if their paths changed, or else `cancel`.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-files
    void SetFiles(RefPtr<FileList> &&files,
                  html::WasSetByJavaScript wasSetByJavaScript = html::WasSetByJavaScript::No) noexcept;

    /// @brief Takes a file input's files from @p dragData, dropped on it.
    /// @return Whether it took any.
    bool ReceiveDroppedFiles(const DragData &dragData) noexcept;

    /// @brief Whether a drag over the input carries files it would take.
    KRYS_NODISCARD bool CanReceiveDroppedFiles() const noexcept
    {
      return _canReceiveDroppedFiles;
    }

    /// @brief Sets whether a file input is a drop target for the files being dragged, updating its look.
    void SetCanReceiveDroppedFiles(bool canReceiveDroppedFiles) noexcept;

    /// @brief A file input's text for its files, given with them by the host's chooser.
    KRYS_NODISCARD DOMString DisplayString() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formaction
    KRYS_NODISCARD dom::USVString FormAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formaction
    void FormAction(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formenctype
    KRYS_NODISCARD DOMString FormEnctype() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formenctype
    void FormEnctype(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formmethod
    KRYS_NODISCARD DOMString FormMethod() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formmethod
    void FormMethod(DOMString value) noexcept;

    using HTMLTextFormControlElement::FormNoValidate;

    /// @see https://html.spec.whatwg.org/#dom-input-formnovalidate
    void FormNoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formtarget
    KRYS_NODISCARD DOMString FormTarget() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-formtarget
    void FormTarget(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-height
    void Height(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-indeterminate
    KRYS_NODISCARD bool Indeterminate() const noexcept
    {
      return _isIndeterminate;
    }

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-indeterminate
    void Indeterminate(bool value) noexcept
    {
      SetIndeterminate(value);
    }

    /// @brief Sets the indeterminate flag, restyling :indeterminate.
    void SetIndeterminate(bool newValue) noexcept;

    /// @brief The datalist giving the input's suggestions (its suggestions source element): the `list`
    /// attribute's resolved target, if that's a datalist and the attribute applies to the input's type.
    /// @see https://html.spec.whatwg.org/multipage/input.html#concept-input-list
    KRYS_NODISCARD RefPtr<HTMLDataListElement> List() const noexcept;

    /// @brief The suggestions source element retargeted against the input: a datalist in a shadow tree shows
    /// as its host.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-list
    KRYS_NODISCARD RefPtr<boo::Element> ListForBindings() const noexcept;

    /// @brief The input's datalist (its `list`) or its suggestions may have changed.
    void DataListMayHaveChanged() noexcept;

    /// @brief @p option's value as a number, if it's a valid value for the input.
    KRYS_NODISCARD Maybe<double> ListOptionValueAsDouble(const HTMLOptionElement &option) const noexcept;

    /// @brief The value of the datalist suggestion closest to @p value, for a slider's thumb to snap to.
    KRYS_NODISCARD Maybe<Decimal> FindClosestTickMarkValue(const Decimal &value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-max
    KRYS_NODISCARD DOMString Max() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-max
    void Max(DOMString value) noexcept;

    using HTMLTextFormControlElement::MaxLength;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-maxlength
    void MaxLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-min
    KRYS_NODISCARD DOMString Min() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-min
    void Min(DOMString value) noexcept;

    using HTMLTextFormControlElement::MinLength;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-minlength
    void MinLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-multiple
    KRYS_NODISCARD bool Multiple() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-input-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-input-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-pattern
    KRYS_NODISCARD DOMString Pattern() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-pattern
    void Pattern(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-placeholder
    KRYS_NODISCARD DOMString Placeholder() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-placeholder
    void Placeholder(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-readonly
    KRYS_NODISCARD bool ReadOnly() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-readonly
    void ReadOnly(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-size
    KRYS_NODISCARD uint32 Size() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-size
    ExceptionOr<void> Size(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-src
    void Src(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-step
    KRYS_NODISCARD DOMString Step() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-step
    void Step(DOMString value) noexcept;

    /// @brief The type's name: "text" for a missing or unknown type.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-type
    KRYS_NODISCARD DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-type
    void Type(DOMString value) noexcept;

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-defaultvalue
    KRYS_NODISCARD DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-defaultvalue
    void DefaultValue(DOMString value) noexcept;

    /// @brief The value, by the type's value mode.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-value
    KRYS_NODISCARD DOMString Value() const noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-value
    ExceptionOr<void> Value(const DOMString &value) noexcept
    {
      return SetValue(value);
    }

    ExceptionOr<void>
      SetValue(DOMStringView value,
               html::TextFieldEventBehavior eventBehavior = html::TextFieldEventBehavior::DispatchNoEvent,
               html::TextControlSetValueSelection selection =
                 html::TextControlSetValueSelection::SetSelectionToEnd) noexcept final;

    /// @brief Sets the value as the user would, firing `input` and `change`.
    void SetValueForUser(DOMStringView value) noexcept;

    /// @brief The value as milliseconds since the epoch (NaN if it isn't a date), as `valueAsDate` exposes
    /// it.
    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-valueasdate
    KRYS_NODISCARD double ValueAsDate() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-valueasdate
    ExceptionOr<void> SetValueAsDate(double value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-valueasnumber
    KRYS_NODISCARD double ValueAsNumber() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-valueasnumber
    ExceptionOr<void> SetValueAsNumber(
      double value,
      html::TextFieldEventBehavior eventBehavior = html::TextFieldEventBehavior::DispatchNoEvent) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-stepup
    ExceptionOr<void> StepUp(int n = 1) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-stepdown
    ExceptionOr<void> StepDown(int n = 1) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-validationmessage
    KRYS_NODISCARD DOMString ValidationMessage() const noexcept final;

    /// @brief The selection's start, or null if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    KRYS_NODISCARD Maybe<uint32> SelectionStartForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    ExceptionOr<void> SetSelectionStartForBindings(Maybe<uint32> start) noexcept;

    /// @brief The selection's end, or null if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionend
    KRYS_NODISCARD Maybe<uint32> SelectionEndForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionend
    ExceptionOr<void> SetSelectionEndForBindings(Maybe<uint32> end) noexcept;

    /// @brief The selection's direction, or null if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection
    KRYS_NODISCARD Maybe<DOMString> SelectionDirectionForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection
    ExceptionOr<void> SetSelectionDirectionForBindings(DOMStringView direction) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange
    ExceptionOr<void> SetSelectionRangeForBindings(uint32 start, uint32 end,
                                                   DOMStringView direction = {}) noexcept;

    using HTMLTextFormControlElement::SetRangeText;

    /// @brief Throws `InvalidStateError` if the type has no selection.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setrangetext
    ExceptionOr<void> SetRangeText(DOMStringView replacement, uint32 start, uint32 end,
                                   DOMStringView selectionMode) noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-input-showpicker
    ExceptionOr<void> ShowPicker() noexcept;

    /// @brief Whether the type's picker is showing.
    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept;

#pragma endregion

    // The validity states.
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

    /// @brief The type's minimum, maximum, step and step base, for stepping and validation.
    KRYS_NODISCARD html::StepRange CreateStepRange(html::AnyStepHandling anyStepHandling) const noexcept;

    /// @brief Whether the user can step the value up and down.
    /// @brief Focuses the input and clicks it, except that a date or time input is only focused and a hidden
    /// one does nothing, as in Chrome.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

    /// @brief Whether the type has a step (the number, range, date and time types).
    KRYS_NODISCARD bool IsSteppable() const noexcept;

    /// @brief Whether the input's type is a button with a text label (submit, reset or button).
    KRYS_NODISCARD bool IsTextButton() const noexcept;

    /// @brief Whether the input's type is `radio`.
    KRYS_NODISCARD bool IsRadioButton() const noexcept;

    KRYS_NODISCARD bool IsTextField() const noexcept final;

    KRYS_NODISCARD bool ShouldUseInputMethod() noexcept final;

    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept final;

    /// @brief Whether the input's type is `search`.
    KRYS_NODISCARD bool IsSearchField() const noexcept;

    /// @brief Whether the input's type is `hidden`.
    KRYS_NODISCARD bool IsInputTypeHidden() const noexcept;

    /// @brief Whether the input's type is `password`.
    KRYS_NODISCARD bool IsPasswordField() const noexcept;

    /// @brief Whether the input's type is `checkbox`.
    KRYS_NODISCARD bool IsCheckbox() const noexcept;

    /// @brief Whether the input's type is `range`.
    KRYS_NODISCARD bool IsRangeControl() const noexcept;

    /// @brief Whether the input's type is `color`.
    KRYS_NODISCARD bool IsColorControl() const noexcept;

    /// @brief Whether the input's type is one with a text value (text, search, url, tel, email or password).
    KRYS_NODISCARD bool IsTextType() const noexcept;

    /// @brief Whether the input's type is `email`.
    KRYS_NODISCARD bool IsEmailField() const noexcept;

    /// @brief Whether the input's type is `file`.
    KRYS_NODISCARD bool IsFileUpload() const noexcept;

    /// @brief Whether the input's type is `image`.
    KRYS_NODISCARD bool IsImageButton() const noexcept;

    /// @brief The loader of an image button's image, if the element has been one.
    KRYS_NODISCARD html::HTMLImageLoader *ImageLoaderIfExists() const noexcept
    {
      return _imageLoader.get();
    }

    /// @brief The image button's image loader, made on first use.
    KRYS_NODISCARD html::HTMLImageLoader &EnsureImageLoader() noexcept;

    /// @brief An image button's label when it shows no image: the `alt` attribute, or else the `title` or
    /// `value` attribute, or else a default.
    KRYS_NODISCARD DOMString AltText() const noexcept;

    /// @brief Whether the input's type is `number`.
    KRYS_NODISCARD bool IsNumberField() const noexcept;

    KRYS_NODISCARD bool IsSubmitButton() const noexcept final;

    /// @brief Whether the input's type is `tel`.
    KRYS_NODISCARD bool IsTelephoneField() const noexcept;

    /// @brief Whether the input's type is `url`.
    KRYS_NODISCARD bool IsUrlField() const noexcept;

    /// @brief Whether the input's type is `date`.
    KRYS_NODISCARD bool IsDateField() const noexcept;

    /// @brief Whether the input's type is `datetime-local`.
    KRYS_NODISCARD bool IsDateTimeLocalField() const noexcept;

    /// @brief Whether the input's type is `month`.
    KRYS_NODISCARD bool IsMonthField() const noexcept;

    /// @brief Whether the input's type is `time`.
    KRYS_NODISCARD bool IsTimeField() const noexcept;

    /// @brief Whether the input's type is `week`.
    KRYS_NODISCARD bool IsWeekField() const noexcept;

    /// @brief Which date or time the type holds, or none.
    KRYS_NODISCARD DateComponentsType DateType() const noexcept;

    /// @brief Whether :checked matches: a checked checkbox or radio button.
    KRYS_NODISCARD bool MatchesCheckedPseudoClass() const noexcept;

    KRYS_NODISCARD bool MatchesIndeterminatePseudoClass() const noexcept final;

    /// @brief Sets whether the checked attribute is present, restyling a checkbox's or radio button's
    /// :default.
    void SetDefaultCheckedState(bool isDefaultChecked) noexcept;

    /// @brief Whether @p value would be a valid value; not for types without a string value (checkboxes and
    /// radio buttons).
    KRYS_NODISCARD bool IsValidValue(DOMStringView value) const noexcept;

    /// @brief Whether the value was set (by the user or the API) rather than taken from the attribute.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-fe-dirty
    KRYS_NODISCARD bool HasDirtyValue() const noexcept
    {
      return _valueIfDirty.has_value();
    }

    /// @brief @p value as the type's value sanitization algorithm leaves it.
    /// @see https://html.spec.whatwg.org/multipage/input.html#value-sanitization-algorithm
    KRYS_NODISCARD Maybe<DOMString> SanitizeValue(Maybe<DOMStringView> value) const noexcept;

    /// @brief The value as the input shows it.
    KRYS_NODISCARD DOMString VisibleValue() const noexcept;

    /// @brief The value, or the type's default if there's none (a submit button's label, say).
    KRYS_NODISCARD DOMString ValueWithDefault() const noexcept;

    KRYS_NODISCARD bool IsActivatedSubmit() const noexcept final;

    void SetActivatedSubmit(bool flag) noexcept final;

    /// @brief The maximum length, capped by the engine's own limit.
    KRYS_NODISCARD uint32 EffectiveMaxLength() const noexcept;

    /// @brief The radio buttons in this one's group, in tree order.
    KRYS_NODISCARD krys::List<Ref<HTMLInputElement>> RadioButtonGroup() const noexcept;

    /// @brief The checked button of the radio button's group, if any.
    KRYS_NODISCARD RefPtr<HTMLInputElement> CheckedRadioButtonForGroup() const noexcept;

    /// @brief The groups this radio button is in: its form's, or its tree's; null if it isn't a radio button
    /// or is in neither.
    KRYS_NODISCARD html::RadioButtonGroups *RadioButtonGroups() const noexcept;

    /// @brief Sets the dirty value to @p sanitizedValue, as the type's value setting does; it's modified by
    /// the user unless no events are fired.
    void SetValueInternal(DOMStringView sanitizedValue, html::TextFieldEventBehavior eventBehavior) noexcept;

    /// @brief Takes the text the user edited as the value (dirty, and modified by the user).
    void SetValueFromRenderer(DOMStringView value) noexcept;

    /// @brief Caches a collapsed selection at @p caretOffset, as setting the value does while unfocused.
    void CacheSelectionInResponseToSetValue(uint32 caretOffset) noexcept
    {
      CacheSelection(caretOffset, caretOffset, html::TextFieldSelectionDirection::SelectionHasNoDirection);
    }

    /// @brief Tells the editor a text field stopped being edited.
    void EndEditing() noexcept;

    /// @brief Creates the type's shadow tree, which inserting the element into a document left pending.
    void UpdateUserAgentShadowTree() noexcept override;

    /// @brief The parts of the type's shadow tree, where it has them.
    KRYS_NODISCARD HTMLElement *ContainerElement() const noexcept;

    KRYS_NODISCARD HTMLElement *InnerBlockElement() const noexcept;

    KRYS_NODISCARD HTMLElement *InnerSpinButtonElement() const noexcept;

    /// @brief The list button of a text field with a datalist.
    KRYS_NODISCARD HTMLElement *DataListButtonElement() const noexcept;

    KRYS_NODISCARD HTMLElement *SliderThumbElement() const noexcept;

    KRYS_NODISCARD HTMLElement *SliderTrackElement() const noexcept;

    KRYS_NODISCARD HTMLElement *ResultsButtonElement() const noexcept;

    KRYS_NODISCARD HTMLElement *CancelButtonElement() const noexcept;

    KRYS_NODISCARD HTMLElement *PlaceholderElement() const noexcept final;

    KRYS_NODISCARD RefPtr<html::TextControlInnerTextElement> InnerTextElement() const noexcept final;

    RefPtr<html::TextControlInnerTextElement> InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept final;

    KRYS_NODISCARD RenderStyle CreateInnerTextStyle(const RenderStyle &style) noexcept final;

    /// @brief Whether the field's text ends in an ellipsis: when `text-overflow` says so and it isn't
    /// focused.
    KRYS_NODISCARD bool ShouldTruncateText(const RenderStyle &style) const noexcept;

    /// @brief Sets @p preferredSize to the field's size in characters.
    /// @return Whether the size includes the field's decorations.
    KRYS_NODISCARD bool SizeShouldIncludeDecoration(int32 &preferredSize) const noexcept;

    /// @brief The width the type's decorations (buttons beside the text) take in a field @p inputWidth wide.
    KRYS_NODISCARD float DecorationWidth(float inputWidth) const noexcept;

    /// @brief Whether the input is a text field.
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

    KRYS_NODISCARD Maybe<DOMString> ResultForDialogSubmit() const noexcept final;

    KRYS_NODISCARD DOMString FormControlType() const noexcept final;

    KRYS_NODISCARD bool CanTriggerImplicitSubmission() const noexcept final
    {
      return IsTextField();
    }

    bool AppendFormData(html::DOMFormData &formData) noexcept final;

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

    void LegacyPreActivationBehavior(Event &event) noexcept final;

    void ActivationBehavior(Event &event) noexcept final;

    void LegacyCanceledActivationBehavior(Event &event) noexcept final;

    void DefaultEventHandler(Event &event) noexcept final;

#pragma region HTMLInputElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLInputElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-input-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-input-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-input-usemap
    KRYS_NODISCARD DOMString UseMap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-input-usemap
    void UseMap(DOMString value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    void OnInsert() noexcept final;

    void OnPostConnection() noexcept final;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept final;

    void OnAdopt(Document &oldDocument) noexcept final;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept final;

    void OnClone(Node &copy, bool subtree) noexcept final;

    KRYS_NODISCARD bool IsEmptyValue() const noexcept final;

    void UpdatePlaceholderText() noexcept final;

    void SubtreeHasChanged() noexcept final;

    void HandleFocusEvent(Node *oldFocusedNode) noexcept final;

    void HandleBlurEvent() noexcept final;

    void WillChangeForm() noexcept final;

    void DidChangeForm() noexcept final;

    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final;

    void RequiredStateChanged() noexcept final;

    void DisabledStateChanged() noexcept final;

    void ReadOnlyStateChanged() noexcept final;

  private:
    /// @brief The value, or null if the type has none (no dirty value, value attribute or fallback).
    KRYS_NODISCARD Maybe<DOMString> ValueOrNull() const noexcept;

    /// @brief Whether `minlength` and `maxlength` apply (to the types with a text value).
    KRYS_NODISCARD bool SupportsMinLength() const noexcept
    {
      return IsTextType();
    }

    KRYS_NODISCARD bool SupportsMaxLength() const noexcept
    {
      return IsTextType();
    }

    /// @brief Whether @p value is too short or too long; only a value the user changed can be, unless
    /// @p check says not to look.
    KRYS_NODISCARD bool TooShort(DOMStringView value, NeedsToCheckDirtyFlag check) const noexcept;

    KRYS_NODISCARD bool TooLong(DOMStringView value, NeedsToCheckDirtyFlag check) const noexcept;

    /// @brief Changes the input's type to the one the attribute names, carrying the value between the value
    /// modes.
    /// @see https://html.spec.whatwg.org/#the-input-element:concept-input-type-change
    void UpdateType(DOMStringView typeAttributeValue) noexcept;

    /// @brief Updates what follows the type: its activation behaviour, the renderer, the shadow tree and the
    /// style.
    void RunPostTypeUpdateTasks() noexcept;

    /// @brief Puts the caret where focusing a text field puts it, without a kept selection to restore.
    void SetDefaultSelectionAfterFocus(dom::SelectionRestorationMode restorationMode,
                                       SelectionRevealMode revealMode) noexcept;

    /// @brief Restyles a text field with an ellipsis as it gains or loses the focus.
    void InvalidateStyleOnFocusChangeIfNeeded() noexcept;

    /// @brief Takes a new `maxlength` or `minlength`, updating the validity.
    void MaxLengthAttributeChanged(Maybe<DOMStringView> newValue) noexcept;

    void MinLengthAttributeChanged(Maybe<DOMStringView> newValue) noexcept;

    /// @brief Sanitizes the value again, as the type or an attribute it depends on changed.
    void UpdateValueIfNeeded() noexcept;

    /// @brief Adds the radio button to its group, or removes it.
    void AddToRadioButtonGroup() noexcept;

    /// @brief Watches the element the `list` attribute names, while the input is connected.
    void ResetListAttributeTargetObserver() noexcept;

    void RemoveFromRadioButtonGroup() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLInputElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLInputElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLInputElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
