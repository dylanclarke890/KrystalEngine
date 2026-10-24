#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/RadioButtonGroups.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/FileInputType.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/HTML/RadioInputType.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/HTML/TextInputType.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    /// @see https://html.spec.whatwg.org/#limited-to-only-non-negative-numbers-greater-than-zero
    KRYS_NODISCARD uint32 LimitToOnlyHTMLNonNegativeNumbersGreaterThanZero(Maybe<dom::DOMStringView> value,
                                                                           uint32 defaultValue) noexcept
    {
      if (!value)
      {
        return defaultValue;
      }
      auto result = Numbers::ParseNonNegativeInteger(*value);
      if (!result.Success() || result.Value == 0 || result.Value > 2'147'483'647)
      {
        return defaultValue;
      }
      return static_cast<uint32>(result.Value);
    }
  }

  HTMLInputElement::HTMLInputElement(dom::Document &document) noexcept
      : HTMLTextFormControlElement(document, HTMLElementInterface::Input),
        _inputType(CreateRef<TextInputType>(*this))
  {
    // Activation follows the type: see UpdateEventTargetFlags().
    SetEventTargetFlag(dom::EventTargetFlags::HasLegacyPreActivationBehavior);
    SetEventTargetFlag(dom::EventTargetFlags::HasLegacyCanceledActivationBehavior);
  }

  HTMLInputElement::~HTMLInputElement() noexcept
  {
    // The input type must not keep a pointer to this element once it's gone.
    _inputType->DetachFromElement();
  }

  bool HTMLInputElement::CanStartSelection() const noexcept
  {
    if (!IsTextField())
    {
      return false;
    }
    return HTMLTextFormControlElement::CanStartSelection();
  }

#pragma region HTMLInputElement

  dom::DOMString HTMLInputElement::Accept() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"accept").value_or(u8"");
  }

  void HTMLInputElement::Accept(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"accept", krys::move(value));
  }

  bool HTMLInputElement::Alpha() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"alpha");
  }

  void HTMLInputElement::Alpha(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"alpha", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Alt() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLInputElement::Alt(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLInputElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  bool HTMLInputElement::DefaultChecked() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"checked");
  }

  void HTMLInputElement::DefaultChecked(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"checked", krys::move(value));
  }

  void HTMLInputElement::SetChecked(bool isChecked, WasSetByJavaScript wasCheckedByJavaScript) noexcept
  {
    _dirtyCheckednessFlag = true;
    if (Checked() == isChecked)
    {
      return;
    }

    _inputType->WillUpdateCheckedness(isChecked, wasCheckedByJavaScript);

    style::PseudoClassChangeInvalidation checkedInvalidation(*this, css::SelectorPseudoClass::Checked,
                                                             isChecked);

    _isChecked = isChecked;

    if (auto *buttons = RadioButtonGroups())
    {
      buttons->UpdateCheckedState(*this);
    }
    if (auto *renderer = Renderer(); renderer && renderer->Style().HasUsedAppearance())
    {
      renderer->Repaint();
    }
    UpdateValidity();
    // FIXME: Telling accessibility the checked state changed, with accessibility.
  }

  dom::DOMString HTMLInputElement::ColourSpace() const noexcept
  {
    auto value = GetAttribute(u8"colorspace");
    if (value && text::IsASCIICaselessEqual(dom::DOMStringView(*value), dom::DOMStringView(u8"display-p3")))
    {
      return u8"display-p3";
    }
    return u8"limited-srgb";
  }

  void HTMLInputElement::ColourSpace(dom::DOMString &&value) noexcept
  {
    (void)SetAttribute(u8"colorspace", krys::move(value));
  }

  dom::DOMString HTMLInputElement::DirName() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLInputElement::DirName(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dirname", krys::move(value));
  }

  bool HTMLInputElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLInputElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::USVString HTMLInputElement::FormAction() const noexcept
  {
    // https://html.spec.whatwg.org/#dom-fs-formaction
    // Let attributeValue be the result of running get an attribute value given this and formaction. If
    // attributeValue is null or the empty string, return this's node document's URL, serialized.
    auto attributeValue = GetAttribute(u8"formaction");
    if (!attributeValue || attributeValue->empty())
    {
      return dom::USVString(NodeDocument().GetURL().String());
    }
    // Let urlString be the result of encoding-parsing-and-serializing a URL given attributeValue, relative to
    // this's node document. If urlString is not failure, return urlString.
    if (auto url = NodeDocument().CompleteURL(*attributeValue); url.IsValid())
    {
      return dom::USVString(url.String());
    }
    // Return attributeValue, converted to a scalar value string.
    return dom::USVString(krys::move(*attributeValue));
  }

  void HTMLInputElement::FormAction(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectSetter<dom::USVString>(*this, u8"formaction", krys::move(value));
  }

  dom::DOMString HTMLInputElement::FormEnctype() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formenctype").value_or(u8"");
  }

  void HTMLInputElement::FormEnctype(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formenctype", krys::move(value));
  }

  dom::DOMString HTMLInputElement::FormMethod() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formmethod").value_or(u8"");
  }

  void HTMLInputElement::FormMethod(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formmethod", krys::move(value));
  }

  void HTMLInputElement::FormNoValidate(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"formnovalidate", krys::move(value));
  }

  dom::DOMString HTMLInputElement::FormTarget() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLInputElement::FormTarget(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formtarget", krys::move(value));
  }

  uint32 HTMLInputElement::Height() const noexcept
  {
    // FIXME: An image button's rendered height (or its image's), with the image input type's renderer.
    return Reflection::Reflect<uint32>(*this, u8"height");
  }

  void HTMLInputElement::Height(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"height", krys::move(value));
  }

  void HTMLInputElement::SetIndeterminate(bool newValue) noexcept
  {
    if (Indeterminate() == newValue)
    {
      return;
    }

    style::PseudoClassChangeInvalidation indeterminateInvalidation(
      *this, css::SelectorPseudoClass::Indeterminate, newValue);
    _isIndeterminate = newValue;

    if (auto *renderer = Renderer(); renderer && renderer->Style().HasUsedAppearance())
    {
      renderer->Repaint();
    }
    // FIXME: Telling accessibility the value changed, with accessibility.
  }

  dom::DOMString HTMLInputElement::Max() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"max").value_or(u8"");
  }

  void HTMLInputElement::Max(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"max", krys::move(value));
  }

  void HTMLInputElement::MaxLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"maxlength", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Min() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"min").value_or(u8"");
  }

  void HTMLInputElement::Min(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"min", krys::move(value));
  }

  void HTMLInputElement::MinLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"minlength", krys::move(value));
  }

  bool HTMLInputElement::Multiple() const noexcept
  {
    return HasAttribute(u8"multiple");
  }

  void HTMLInputElement::Multiple(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"multiple", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Name() const noexcept
  {
    return _name;
  }

  void HTMLInputElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Pattern() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"pattern").value_or(u8"");
  }

  void HTMLInputElement::Pattern(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"pattern", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Placeholder() const noexcept
  {
    // The line breaks are removed from the attribute's value.
    auto attributeValue = GetAttribute(u8"placeholder").value_or(dom::DOMString());
    if (std::ranges::none_of(attributeValue, IsHTMLLineBreak)) KRYS_LIKELY
    {
      return attributeValue;
    }

    std::erase_if(attributeValue, IsHTMLLineBreak);
    return attributeValue;
  }

  void HTMLInputElement::Placeholder(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"placeholder", krys::move(value));
  }

  bool HTMLInputElement::ReadOnly() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLInputElement::ReadOnly(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"readonly", krys::move(value));
  }

  bool HTMLInputElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLInputElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLInputElement::Size() const noexcept
  {
    return _size;
  }

  dom::ExceptionOr<void> HTMLInputElement::Size(uint32 size) noexcept
  {
    if (!size)
    {
      return dom::Exception {dom::ExceptionCode::IndexSizeError};
    }
    auto limitedSize = size > 2'147'483'647 ? static_cast<uint32>(DefaultSize) : size;
    auto string = std::to_string(limitedSize);
    (void)SetAttribute(u8"size",
                       dom::DOMString(reinterpret_cast<const char8 *>(string.data()), string.size()));
    return {};
  }

  dom::USVString HTMLInputElement::Src() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLInputElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  dom::DOMString HTMLInputElement::Step() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"step").value_or(u8"");
  }

  void HTMLInputElement::Step(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"step", krys::move(value));
  }

  void HTMLInputElement::Type(dom::DOMString &&value) noexcept
  {
    (void)SetAttribute(u8"type", krys::move(value));
  }

  bool HTMLInputElement::IsReplaced(const RenderStyle *) const noexcept
  {
    return _inputType && _inputType->IsImageButton();
  }

  dom::DOMString HTMLInputElement::DefaultValue() const noexcept
  {
    return GetAttribute(u8"value").value_or(dom::DOMString());
  }

  void HTMLInputElement::DefaultValue(dom::DOMString &&value) noexcept
  {
    (void)SetAttribute(u8"value", krys::move(value));
  }

  Maybe<dom::DOMString> HTMLInputElement::ValueOrNull() const noexcept
  {
    if (_inputType->IsFileUpload())
    {
      return static_cast<const FileInputType &>(*_inputType).FirstElementPathForInputValue();
    }

    if (_valueIfDirty)
    {
      return _valueIfDirty;
    }

    if (auto valueString = GetAttribute(u8"value"))
    {
      if (auto sanitizedValue = SanitizeValue(dom::DOMStringView(*valueString)))
      {
        return sanitizedValue;
      }
    }

    return _inputType->FallbackValue();
  }

  dom::DOMString HTMLInputElement::Value() const noexcept
  {
    return ValueOrNull().value_or(dom::DOMString());
  }

  dom::ExceptionOr<void> HTMLInputElement::SetValue(dom::DOMStringView value,
                                                    TextFieldEventBehavior eventBehavior,
                                                    TextControlSetValueSelection selection) noexcept
  {
    if (IsFileUpload() && !value.empty())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    Ref<HTMLInputElement> protectedThis = ShareRef(*this);
    auto sanitizedValue = SanitizeValue(value).value_or(dom::DOMString());
    bool valueChanged = sanitizedValue != Value();

    SetLastChangeWasNotUserEdit();
    SetFormControlValueMatchesRenderer(false);
    _inputType->SetValue(sanitizedValue, valueChanged, eventBehavior, selection);
    // FIXME: Auto directionality (updateEffectiveTextDirection), with dir=auto.

    if (valueChanged && eventBehavior == TextFieldEventBehavior::DispatchNoEvent)
    {
      SetTextAsOfLastFormControlChangeEvent(dom::DOMString(sanitizedValue));
    }

    // FIXME: The chrome client's didProgrammaticallyClearTextFormControl, with the host's autofill.
    return {};
  }

  void HTMLInputElement::SetValueForUser(dom::DOMStringView value) noexcept
  {
    (void)SetValue(value, TextFieldEventBehavior::DispatchInputAndChangeEvent);
  }

  void HTMLInputElement::SetValueInternal(dom::DOMStringView sanitizedValue,
                                          TextFieldEventBehavior eventBehavior) noexcept
  {
    _valueIfDirty = dom::DOMString(sanitizedValue);
    _wasModifiedByUser = eventBehavior != TextFieldEventBehavior::DispatchNoEvent;
    UpdateValidity();
  }

  double HTMLInputElement::ValueAsDate() const noexcept
  {
    return _inputType->ValueAsDate();
  }

  dom::ExceptionOr<void> HTMLInputElement::SetValueAsDate(double value) noexcept
  {
    return _inputType->SetValueAsDate(value);
  }

  double HTMLInputElement::ValueAsNumber() const noexcept
  {
    return _inputType->ValueAsDouble();
  }

  dom::ExceptionOr<void> HTMLInputElement::SetValueAsNumber(double newValue,
                                                            TextFieldEventBehavior eventBehavior) noexcept
  {
    if (!std::isfinite(newValue))
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }
    return _inputType->SetValueAsDouble(newValue, eventBehavior);
  }

  uint32 HTMLInputElement::Width() const noexcept
  {
    // FIXME: An image button's rendered width (or its image's), with the image input type's renderer.
    return Reflection::Reflect<uint32>(*this, u8"width");
  }

  void HTMLInputElement::Width(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"width", krys::move(value));
  }

  dom::ExceptionOr<void> HTMLInputElement::StepUp(int n) noexcept
  {
    return _inputType->StepUp(n);
  }

  dom::ExceptionOr<void> HTMLInputElement::StepDown(int n) noexcept
  {
    return _inputType->StepUp(-n);
  }

  dom::DOMString HTMLInputElement::ValidationMessage() const noexcept
  {
    if (!WillValidate())
    {
      return {};
    }

    if (CustomError())
    {
      return CustomValidationMessage();
    }

    return _inputType->ValidationMessage();
  }

#pragma endregion

#pragma region Validity

  bool HTMLInputElement::HasBadInput() const noexcept
  {
    return _inputType->HasBadInput();
  }

  bool HTMLInputElement::PatternMismatch() const noexcept
  {
    return _inputType->PatternMismatch(Value());
  }

  bool HTMLInputElement::RangeUnderflow() const noexcept
  {
    return _inputType->RangeUnderflow(Value());
  }

  bool HTMLInputElement::RangeOverflow() const noexcept
  {
    return _inputType->RangeOverflow(Value());
  }

  bool HTMLInputElement::StepMismatch() const noexcept
  {
    return _inputType->StepMismatch(Value());
  }

  bool HTMLInputElement::TooShort() const noexcept
  {
    return TooShort(Value(), CheckDirtyFlag);
  }

  bool HTMLInputElement::TooLong() const noexcept
  {
    return TooLong(Value(), CheckDirtyFlag);
  }

  bool HTMLInputElement::TypeMismatch() const noexcept
  {
    return _inputType->TypeMismatch();
  }

  bool HTMLInputElement::ValueMissing() const noexcept
  {
    return _inputType->ValueMissing(Value());
  }

  bool HTMLInputElement::ComputeValidity() const noexcept
  {
    auto value = Value();
    bool someError = _inputType->IsInvalid(value) || TooShort(value, CheckDirtyFlag)
                     || TooLong(value, CheckDirtyFlag) || CustomError();
    return !someError;
  }

  bool HTMLInputElement::TooShort(dom::DOMStringView value, NeedsToCheckDirtyFlag check) const noexcept
  {
    if (!SupportsMinLength())
    {
      return false;
    }

    int32 min = MinLength();
    if (min <= 0)
    {
      return false;
    }

    if (check == CheckDirtyFlag)
    {
      // Return false for the default value or a value set by a script even if it is shorter than minLength.
      if (!HasDirtyValue() || !_wasModifiedByUser)
      {
        return false;
      }
    }

    // The empty string is excluded from tooShort validation.
    if (value.empty())
    {
      return false;
    }

    return LengthForAPIValue(value) < static_cast<size_t>(min);
  }

  bool HTMLInputElement::TooLong(dom::DOMStringView value, NeedsToCheckDirtyFlag check) const noexcept
  {
    if (!SupportsMaxLength())
    {
      return false;
    }
    uint32 max = EffectiveMaxLength();
    if (check == CheckDirtyFlag)
    {
      // Return false for the default value or a value set by a script even if it is longer than maxLength.
      if (!HasDirtyValue() || !_wasModifiedByUser)
      {
        return false;
      }
    }
    return LengthForAPIValue(value) > max;
  }

  bool HTMLInputElement::IsValidValue(dom::DOMStringView value) const noexcept
  {
    if (!_inputType->IsValidValue(value))
    {
      return false;
    }

    return !TooShort(value, IgnoreDirtyFlag) && !TooLong(value, IgnoreDirtyFlag);
  }

  double HTMLInputElement::Minimum() const noexcept
  {
    return _inputType->Minimum();
  }

  double HTMLInputElement::Maximum() const noexcept
  {
    return _inputType->Maximum();
  }

  bool HTMLInputElement::GetAllowedValueStep(Decimal *step) const noexcept
  {
    return _inputType->GetAllowedValueStep(step);
  }

  StepRange HTMLInputElement::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    return _inputType->CreateStepRange(anyStepHandling);
  }

  bool HTMLInputElement::IsInRange() const noexcept
  {
    return WillValidate() && _inputType->IsInRange(Value());
  }

  bool HTMLInputElement::IsOutOfRange() const noexcept
  {
    return WillValidate() && _inputType->IsOutOfRange(Value());
  }

  bool HTMLInputElement::ComputeWillValidate() const noexcept
  {
    return _inputType->SupportsValidation() && HTMLTextFormControlElement::ComputeWillValidate();
  }

  void HTMLInputElement::RequiredStateChanged() noexcept
  {
    HTMLTextFormControlElement::RequiredStateChanged();
    if (auto *buttons = RadioButtonGroups())
    {
      buttons->RequiredStateChanged(*this);
    }
    _inputType->RequiredStateChanged();
  }

  bool HTMLInputElement::IsRequiredFormControl() const noexcept
  {
    return _inputType->SupportsRequired() && IsRequired();
  }

#pragma endregion

#pragma region Type queries

  bool HTMLInputElement::IsSteppable() const noexcept
  {
    return _inputType->IsSteppable();
  }

  bool HTMLInputElement::IsTextButton() const noexcept
  {
    return _inputType->IsTextButton();
  }

  bool HTMLInputElement::IsRadioButton() const noexcept
  {
    return _inputType->IsRadioButton();
  }

  bool HTMLInputElement::IsTextField() const noexcept
  {
    return _inputType->IsTextField();
  }

  bool HTMLInputElement::IsSearchField() const noexcept
  {
    return _inputType->IsSearchField();
  }

  bool HTMLInputElement::IsInputTypeHidden() const noexcept
  {
    return _inputType->IsHiddenType();
  }

  bool HTMLInputElement::IsPasswordField() const noexcept
  {
    return _inputType->IsPasswordField();
  }

  bool HTMLInputElement::IsCheckbox() const noexcept
  {
    return _inputType->IsCheckbox();
  }

  bool HTMLInputElement::IsRangeControl() const noexcept
  {
    return _inputType->IsRangeControl();
  }

  bool HTMLInputElement::IsColorControl() const noexcept
  {
    return _inputType->IsColorControl();
  }

  bool HTMLInputElement::IsText() const noexcept
  {
    return _inputType->IsTextType();
  }

  bool HTMLInputElement::IsTextType() const noexcept
  {
    return _inputType->IsTextType();
  }

  bool HTMLInputElement::IsEmailField() const noexcept
  {
    return _inputType->IsEmailField();
  }

  bool HTMLInputElement::IsFileUpload() const noexcept
  {
    return _inputType->IsFileUpload();
  }

  bool HTMLInputElement::IsImageButton() const noexcept
  {
    return _inputType->IsImageButton();
  }

  bool HTMLInputElement::IsNumberField() const noexcept
  {
    return _inputType->IsNumberField();
  }

  bool HTMLInputElement::IsSubmitButton() const noexcept
  {
    return _inputType->IsSubmitButton();
  }

  bool HTMLInputElement::IsTelephoneField() const noexcept
  {
    return _inputType->IsTelephoneField();
  }

  bool HTMLInputElement::IsURLField() const noexcept
  {
    return _inputType->IsURLField();
  }

  bool HTMLInputElement::IsDateField() const noexcept
  {
    return _inputType->IsDateField();
  }

  bool HTMLInputElement::IsDateTimeLocalField() const noexcept
  {
    return _inputType->IsDateTimeLocalField();
  }

  bool HTMLInputElement::IsMonthField() const noexcept
  {
    return _inputType->IsMonthField();
  }

  bool HTMLInputElement::IsTimeField() const noexcept
  {
    return _inputType->IsTimeField();
  }

  bool HTMLInputElement::IsWeekField() const noexcept
  {
    return _inputType->IsWeekField();
  }

  DateComponentsType HTMLInputElement::DateType() const noexcept
  {
    return _inputType->DateType();
  }

  bool HTMLInputElement::IsEnumeratable() const noexcept
  {
    return _inputType->IsEnumeratable();
  }

  bool HTMLInputElement::IsLabelable() const noexcept
  {
    return _inputType->IsLabelable();
  }

  bool HTMLInputElement::IsInteractiveContent() const noexcept
  {
    return _inputType->IsInteractiveContent();
  }

  bool HTMLInputElement::SupportsReadOnly() const noexcept
  {
    return _inputType->SupportsReadOnly();
  }

  bool HTMLInputElement::SupportsPlaceholder() const noexcept
  {
    return _inputType->SupportsPlaceholder();
  }

  bool HTMLInputElement::IsEmptyValue() const noexcept
  {
    return _inputType->IsEmptyValue();
  }

  void HTMLInputElement::UpdatePlaceholderText() noexcept
  {
    _inputType->UpdatePlaceholderText();
  }

  void HTMLInputElement::SubtreeHasChanged() noexcept
  {
    _inputType->SubtreeHasChanged();
    // FIXME: Auto directionality, which typing doesn't update through childrenChanged
    // (updateEffectiveTextDirection), with dir=auto.
  }

  void HTMLInputElement::HandleFocusEvent(dom::Node *oldFocusedNode) noexcept
  {
    _inputType->HandleFocusEvent(oldFocusedNode);

    InvalidateStyleOnFocusChangeIfNeeded();
  }

  void HTMLInputElement::HandleBlurEvent() noexcept
  {
    _inputType->HandleBlurEvent();

    InvalidateStyleOnFocusChangeIfNeeded();
  }

  void HTMLInputElement::InvalidateStyleOnFocusChangeIfNeeded() noexcept
  {
    if (!IsTextField())
    {
      return;
    }
    // Focus change may affect the result of shouldTruncateText().
    if (auto *style = ExistingComputedStyle(); style && style->GetTextOverflow() == TextOverflow::Ellipsis)
    {
      InvalidateStyleForSubtreeInternal();
    }
  }

  void HTMLInputElement::UpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                                               SelectionRevealMode revealMode) noexcept
  {
    if (IsTextField())
    {
      if (restorationMode == dom::SelectionRestorationMode::RestoreOrSelectAll && HasCachedSelection())
      {
        RestoreCachedSelection(revealMode);
      }
      else
      {
        SetDefaultSelectionAfterFocus(restorationMode, revealMode);
      }
    }
    else
    {
      HTMLTextFormControlElement::UpdateFocusAppearance(restorationMode, revealMode);
    }
  }

  void HTMLInputElement::SetDefaultSelectionAfterFocus(dom::SelectionRestorationMode restorationMode,
                                                       SelectionRevealMode revealMode) noexcept
  {
    krys_debug_assert(IsTextField());
    uint32 start = 0;
    auto direction = TextFieldSelectionDirection::SelectionHasNoDirection;
    if (NodeDocument().Frame()
        && NodeDocument().GetEditor().Behavior().ShouldMoveSelectionToEndWhenFocusingTextInput())
    {
      start = std::numeric_limits<uint32>::max();
      direction = TextFieldSelectionDirection::SelectionHasForwardDirection;
    }
    uint32 end = restorationMode == dom::SelectionRestorationMode::PlaceCaretAtStart
                   ? start
                   : std::numeric_limits<uint32>::max();
    SetSelectionRange(start, end, direction, revealMode);
  }

  void HTMLInputElement::EndEditing() noexcept
  {
    if (!IsTextField())
    {
      return;
    }

    if (NodeDocument().Frame())
    {
      NodeDocument().GetEditor().TextFieldDidEndEditing(*this);
    }
  }

  void HTMLInputElement::DidBlur() noexcept
  {
    _inputType->ElementDidBlur();
  }

  void HTMLInputElement::DidDetachRenderers() noexcept
  {
    SetFormControlValueMatchesRenderer(false);
    _inputType->Detach();
  }

  dom::ExceptionOr<void> HTMLInputElement::ShowPicker() noexcept
  {
    if (!NodeDocument().Frame())
    {
      return {};
    }

    if (!IsMutable())
    {
      return dom::Exception(dom::ExceptionCode::InvalidStateError,
                            u8"Input showPicker() cannot be used on immutable controls.");
    }

    // FIXME: Require transient activation (NotAllowedError), once there is user activation.

    _inputType->ShowPicker();
    return {};
  }

  bool HTMLInputElement::IsPresentingAttachedView() const noexcept
  {
    return _inputType->IsPresentingAttachedView();
  }

  void HTMLInputElement::SetValueFromRenderer(dom::DOMStringView value) noexcept
  {
    // File upload controls will never use this.
    krys_debug_assert(!IsFileUpload());

    // Workaround for bug where trailing \n is included in the result of textContent.
    // The assert macro above may also be simplified by removing the expression
    // that calls isEmpty.
    // http://bugs.webkit.org/show_bug.cgi?id=9661
    _valueIfDirty = value == u8"\n" ? dom::DOMString {} : dom::DOMString(value);

    SetFormControlValueMatchesRenderer(true);
    _wasModifiedByUser = true;

    // Input event is fired by the Node::defaultEventHandler for editable controls.
    if (!IsTextField())
    {
      auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Input), true, false, true);
      (void)dom::EventDispatcher::DispatchToTarget(*event, this);
    }

    UpdateValidity();

    // FIXME: Clearing the autofill flags, with autofill.
  }

  RenderPtr<RenderElement> HTMLInputElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return _inputType->CreateInputRenderer(krys::move(style));
  }

  void HTMLInputElement::DidAttachRenderers() noexcept
  {
    HTMLTextFormControlElement::DidAttachRenderers();

    // FIXME: Attaching the type (InputType::attach, for image buttons' loaders), with image buttons.

    if (NodeDocument().FocusedElement() == this)
    {
      EventLoop::Current().QueueTask(
        TaskSource::UserInteraction, NodeDocument(),
        [weakThis = WeakPtr<HTMLInputElement>::WithRef(*this)]
        {
          RefPtr<HTMLInputElement> protectedThis = ShareRefPtr(weakThis.get());
          if (!protectedThis || protectedThis->NodeDocument().FocusedElement() != protectedThis)
          {
            return;
          }
          protectedThis->UpdateFocusAppearance(dom::SelectionRestorationMode::RestoreOrSelectAll,
                                               SelectionRevealMode::Reveal);
        });
    }
  }

  HTMLElement *HTMLInputElement::ContainerElement() const noexcept
  {
    return _inputType->ContainerElement();
  }

  RefPtr<TextControlInnerTextElement> HTMLInputElement::InnerTextElement() const noexcept
  {
    return _inputType->InnerTextElement();
  }

  RefPtr<TextControlInnerTextElement>
    HTMLInputElement::InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept
  {
    return _inputType->InnerTextElementCreatingShadowSubtreeIfNeeded();
  }

  HTMLElement *HTMLInputElement::InnerBlockElement() const noexcept
  {
    return _inputType->InnerBlockElement();
  }

  HTMLElement *HTMLInputElement::InnerSpinButtonElement() const noexcept
  {
    return _inputType->InnerSpinButtonElement();
  }

  HTMLElement *HTMLInputElement::SliderThumbElement() const noexcept
  {
    return _inputType->SliderThumbElement();
  }

  HTMLElement *HTMLInputElement::SliderTrackElement() const noexcept
  {
    return _inputType->SliderTrackElement();
  }

  HTMLElement *HTMLInputElement::ResultsButtonElement() const noexcept
  {
    return _inputType->ResultsButtonElement();
  }

  HTMLElement *HTMLInputElement::CancelButtonElement() const noexcept
  {
    return _inputType->CancelButtonElement();
  }

  HTMLElement *HTMLInputElement::PlaceholderElement() const noexcept
  {
    return _inputType->PlaceholderElement();
  }

  bool HTMLInputElement::SizeShouldIncludeDecoration(int32 &preferredSize) const noexcept
  {
    return _inputType->SizeShouldIncludeDecoration(DefaultSize, preferredSize);
  }

  float HTMLInputElement::DecorationWidth(float inputWidth) const noexcept
  {
    return _inputType->DecorationWidth(inputWidth);
  }

  bool HTMLInputElement::ShouldTruncateText(const RenderStyle &style) const noexcept
  {
    if (!IsTextField())
    {
      return false;
    }
    return NodeDocument().FocusedElement() != this && style.GetTextOverflow() == TextOverflow::Ellipsis;
  }

  RenderStyle HTMLInputElement::CreateInnerTextStyle(const RenderStyle &style) noexcept
  {
    auto textBlockStyle = RenderStyle::Create();
    textBlockStyle.InheritFrom(style);
    AdjustInnerTextStyle(style, textBlockStyle);

    textBlockStyle.SetWhiteSpaceCollapse(WhiteSpaceCollapse::Preserve);
    textBlockStyle.SetTextWrapMode(TextWrapMode::NoWrap);
    textBlockStyle.SetOverflowWrap(OverflowWrap::Normal);
    textBlockStyle.SetOverflowX(Overflow::Hidden);
    textBlockStyle.SetOverflowY(Overflow::Hidden);
    textBlockStyle.SetTextOverflow(ShouldTruncateText(style) ? TextOverflow::Ellipsis : TextOverflow::Clip);

    textBlockStyle.SetDisplay(DisplayType::Block);

    // FIXME: The strong password AutoFill button's inner text (a masked, translucent inline block), with
    // autofill.

    return textBlockStyle;
  }

  dom::ExceptionOr<void> HTMLInputElement::SetRangeText(dom::DOMStringView replacement, uint32 start,
                                                        uint32 end, dom::DOMStringView selectionMode) noexcept
  {
    if (!_inputType->SupportsSelectionAPI())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    return HTMLTextFormControlElement::SetRangeText(replacement, start, end, selectionMode);
  }

  Maybe<uint32> HTMLInputElement::SelectionStartForBindings() const noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return null;
    }

    return SelectionStart();
  }

  dom::ExceptionOr<void> HTMLInputElement::SetSelectionStartForBindings(Maybe<uint32> start) noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    SetSelectionStart(start.value_or(0));
    return {};
  }

  Maybe<uint32> HTMLInputElement::SelectionEndForBindings() const noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return null;
    }

    return SelectionEnd();
  }

  dom::ExceptionOr<void> HTMLInputElement::SetSelectionEndForBindings(Maybe<uint32> end) noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    SetSelectionEnd(end.value_or(0));
    return {};
  }

  Maybe<dom::DOMString> HTMLInputElement::SelectionDirectionForBindings() const noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return null;
    }

    return dom::DOMString(SelectionDirection());
  }

  dom::ExceptionOr<void>
    HTMLInputElement::SetSelectionDirectionForBindings(dom::DOMStringView direction) noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    SetSelectionDirection(direction);
    return {};
  }

  dom::ExceptionOr<void> HTMLInputElement::SetSelectionRangeForBindings(uint32 start, uint32 end,
                                                                        dom::DOMStringView direction) noexcept
  {
    if (!CanHaveSelection() || !_inputType->SupportsSelectionAPI())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    SetSelectionRange(start, end, direction, ForBindings::Yes);
    return {};
  }

  dom::DOMString HTMLInputElement::FormControlType() const noexcept
  {
    return dom::DOMString(_inputType->FormControlType());
  }

#pragma endregion

#pragma region Pseudo-classes

  bool HTMLInputElement::MatchesCheckedPseudoClass() const noexcept
  {
    return Checked() && _inputType->IsCheckable();
  }

  bool HTMLInputElement::MatchesIndeterminatePseudoClass() const noexcept
  {
    return _inputType->MatchesIndeterminatePseudoClass();
  }

  bool HTMLInputElement::MatchesDefaultPseudoClass() const noexcept
  {
    if (_inputType->CanBeSuccessfulSubmitButton())
    {
      return !IsDisabledFormControl() && Form() && Form()->DefaultButton() == this;
    }
    return _inputType->IsCheckable() && _isDefaultChecked;
  }

  bool HTMLInputElement::MatchesReadWritePseudoClass() const noexcept
  {
    return SupportsReadOnly() && IsMutable();
  }

  void HTMLInputElement::SetDefaultCheckedState(bool isDefaultChecked) noexcept
  {
    if (_isDefaultChecked == isDefaultChecked)
    {
      return;
    }

    Maybe<style::PseudoClassChangeInvalidation> defaultInvalidation;
    if (_inputType->IsCheckable())
    {
      defaultInvalidation.emplace(*this, css::SelectorPseudoClass::Default, isDefaultChecked);
    }
    _isDefaultChecked = isDefaultChecked;
  }

#pragma endregion

#pragma region Value

  Maybe<dom::DOMString> HTMLInputElement::SanitizeValue(Maybe<dom::DOMStringView> value) const noexcept
  {
    if (!value)
    {
      return std::nullopt;
    }
    return _inputType->SanitizeValue(*value);
  }

  dom::DOMString HTMLInputElement::VisibleValue() const noexcept
  {
    return _inputType->VisibleValue();
  }

  dom::DOMString HTMLInputElement::ValueWithDefault() const noexcept
  {
    if (auto value = ValueOrNull())
    {
      return *value;
    }

    return _inputType->DefaultValue().value_or(dom::DOMString());
  }

  bool HTMLInputElement::IsActivatedSubmit() const noexcept
  {
    return _isActivatedSubmit;
  }

  void HTMLInputElement::SetActivatedSubmit(bool flag) noexcept
  {
    _isActivatedSubmit = flag;
  }

  uint32 HTMLInputElement::EffectiveMaxLength() const noexcept
  {
    // The number -1 represents no maximum at all; conveniently it becomes a super-large value when converted
    // to unsigned.
    return std::min<uint32>(static_cast<uint32>(MaxLength()), MaxEffectiveLength);
  }

  void HTMLInputElement::MaxLengthAttributeChanged(Maybe<dom::DOMStringView> newValue) noexcept
  {
    uint32 oldEffectiveMaxLength = EffectiveMaxLength();
    InternalSetMaxLength(ParseLengthAttribute(newValue));
    if (oldEffectiveMaxLength != EffectiveMaxLength())
    {
      UpdateValueIfNeeded();
    }

    UpdateValidity();
  }

  void HTMLInputElement::MinLengthAttributeChanged(Maybe<dom::DOMStringView> newValue) noexcept
  {
    int32 oldMinLength = MinLength();
    InternalSetMinLength(ParseLengthAttribute(newValue));
    if (oldMinLength != MinLength())
    {
      UpdateValueIfNeeded();
    }

    UpdateValidity();
  }

  void HTMLInputElement::UpdateValueIfNeeded() noexcept
  {
    auto newValue = SanitizeValue(_valueIfDirty ? Maybe<dom::DOMStringView>(*_valueIfDirty) : std::nullopt);
    krys_debug_assert(_valueIfDirty || !newValue);
    if (newValue != _valueIfDirty)
    {
      (void)SetValue(newValue.value_or(dom::DOMString()));
    }
  }

#pragma endregion

#pragma region Forms

  Maybe<dom::DOMString> HTMLInputElement::ResultForDialogSubmit() const noexcept
  {
    return _inputType->ResultForDialogSubmit();
  }

  bool HTMLInputElement::AppendFormData(DOMFormData &formData) noexcept
  {
    Ref<InputType> protectedInputType = ShareRef(*_inputType);
    return _inputType->IsFormDataAppendable() && _inputType->AppendFormData(formData);
  }

  bool HTMLInputElement::IsSuccessfulSubmitButton() const noexcept
  {
    // HTML spec says that buttons must have names to be considered successful. However, other browsers do not
    // impose this constraint. So we do not.
    return _inputType->CanBeSuccessfulSubmitButton();
  }

  void HTMLInputElement::Reset() noexcept
  {
    if (_inputType->StoresValueSeparateFromAttribute())
    {
      // Setting the value to null drops the dirty value, so the value follows the attribute again.
      (void)SetValue({});
      _valueIfDirty = std::nullopt;
      UpdatePlaceholderVisibility();
      UpdateValidity();
    }

    SetInteractedWithSinceLastFormSubmitEvent(false);
    SetChecked(HasAttribute(u8"checked"));
    _dirtyCheckednessFlag = false;
  }

  bool HTMLInputElement::ShouldAutocomplete() const noexcept
  {
    if (_autocomplete != Uninitialized)
    {
      return _autocomplete == On;
    }
    return HTMLTextFormControlElement::ShouldAutocomplete();
  }

  void HTMLInputElement::WillChangeForm() noexcept
  {
    RemoveFromRadioButtonGroup();
    HTMLTextFormControlElement::WillChangeForm();
  }

  void HTMLInputElement::DidChangeForm() noexcept
  {
    HTMLTextFormControlElement::DidChangeForm();
    AddToRadioButtonGroup();
  }

  void HTMLInputElement::DisabledStateChanged() noexcept
  {
    HTMLTextFormControlElement::DisabledStateChanged();
    _inputType->DisabledStateChanged();
  }

  void HTMLInputElement::ReadOnlyStateChanged() noexcept
  {
    HTMLTextFormControlElement::ReadOnlyStateChanged();
    _inputType->ReadOnlyStateChanged();
  }

#pragma endregion

#pragma region Radio buttons

  List<Ref<HTMLInputElement>> HTMLInputElement::RadioButtonGroup() const noexcept
  {
    if (auto *buttons = RadioButtonGroups())
    {
      return buttons->GroupMembers(*this);
    }
    return {};
  }

  RefPtr<HTMLInputElement> HTMLInputElement::CheckedRadioButtonForGroup() const noexcept
  {
    krys_debug_assert(IsRadioButton());

    if (Checked())
    {
      return ShareRefPtr(const_cast<HTMLInputElement *>(this));
    }

    auto name = Name();
    if (auto *buttons = RadioButtonGroups())
    {
      return buttons->CheckedButtonForGroup(name);
    }

    if (name.empty())
    {
      return nullptr;
    }

    krys_debug_assert(!IsConnected());
    krys_debug_assert(!Form());

    // The input is not managed by a RadioButtonGroups, we'll need to traverse the tree.
    RefPtr<HTMLInputElement> checkedRadio;
    auto &root = dom::TreeQueries::Root(const_cast<HTMLInputElement &>(*this));
    RadioInputType::ForEachButtonInDetachedGroup(Downcast<dom::ContainerNode>(root), name,
                                                 [&](HTMLInputElement &input)
                                                 {
                                                   if (input.Checked())
                                                   {
                                                     checkedRadio = ShareRefPtr(&input);
                                                     return false;
                                                   }
                                                   return true;
                                                 });
    return checkedRadio;
  }

  dom::RadioButtonGroups *HTMLInputElement::RadioButtonGroups() const noexcept
  {
    if (!IsRadioButton())
    {
      return nullptr;
    }
    if (auto *formElement = Form())
    {
      return &formElement->RadioButtonGroups();
    }
    if (IsConnected() || IsInShadowTree())
    {
      return &TreeScope().RadioButtonGroups();
    }
    return nullptr;
  }

  void HTMLInputElement::AddToRadioButtonGroup() noexcept
  {
    if (auto *buttons = RadioButtonGroups())
    {
      buttons->AddButton(*this);
    }
  }

  void HTMLInputElement::RemoveFromRadioButtonGroup() noexcept
  {
    if (auto *buttons = RadioButtonGroups())
    {
      buttons->RemoveButton(*this);
    }
  }

#pragma endregion

#pragma region Type changes

  void HTMLInputElement::UpdateType(dom::DOMStringView typeAttributeValue) noexcept
  {
    krys_debug_assert(_inputType);
    auto newType = InputType::CreateIfDifferent(*this, typeAttributeValue, _inputType.get());
    if (!newType)
    {
      return;
    }
    krys_debug_assert(_inputType->GetType() != newType->GetType());

    style::PseudoClassChangeInvalidation defaultInvalidation(*this, css::SelectorPseudoClass::Default,
                                                             style::PseudoClassChangeInvalidation::AnyValue);

    RemoveFromRadioButtonGroup();

    bool didSupportReadOnly = _inputType->SupportsReadOnly();
    bool willSupportReadOnly = newType->SupportsReadOnly();
    Maybe<style::PseudoClassChangeInvalidation> readWriteInvalidation;

    bool didStoreValue = _inputType->StoresValueSeparateFromAttribute();
    bool willStoreValue = newType->StoresValueSeparateFromAttribute();
    bool didRespectHeightAndWidth = _inputType->ShouldRespectHeightAndWidthAttributes();
    bool wasImageButton = _inputType->IsImageButton();
    bool wasSuccessfulSubmitButtonCandidate = _inputType->CanBeSuccessfulSubmitButton();

    if (didStoreValue && !willStoreValue)
    {
      if (auto dirtyValue = std::exchange(_valueIfDirty, std::nullopt); dirtyValue && !dirtyValue->empty())
      {
        (void)SetAttribute(u8"value", krys::move(*dirtyValue));
      }
    }

    _inputType->RemoveShadowSubtree();
    _inputType->DetachFromElement();

    bool previouslySelectable = _inputType->SupportsSelectionAPI();

    _inputType = krys::move(newType);
    if (!didStoreValue && willStoreValue)
    {
      auto valueAttribute = GetAttribute(u8"value");
      _valueIfDirty =
        SanitizeValue(valueAttribute ? Maybe<dom::DOMStringView>(*valueAttribute) : std::nullopt);
    }
    else
    {
      UpdateValueIfNeeded();
    }
    _inputType->CreateShadowSubtreeIfNeeded();

    // FIXME: Auto directionality when the type changes to or from tel (updateEffectiveTextDirection), with
    // dir=auto.

    if (didSupportReadOnly != willSupportReadOnly && HasAttribute(u8"readonly")) KRYS_UNLIKELY
    {
      readWriteInvalidation.emplace(*this, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                             {css::SelectorPseudoClass::ReadWrite, !willSupportReadOnly},
                                             {css::SelectorPseudoClass::ReadOnly, willSupportReadOnly},
                                           });
      ReadOnlyStateChanged();
    }

    UpdateWillValidateAndValidity();

    SetFormControlValueMatchesRenderer(false);

    _wasModifiedByUser = false;

    if (didRespectHeightAndWidth != _inputType->ShouldRespectHeightAndWidthAttributes()
        || wasImageButton != _inputType->IsImageButton())
    {
      // The presentational hints of the width, height, align, border, vspace and hspace attributes depend on
      // the type.
      InvalidatePresentationalHintStyle();
    }

    if (auto *form = Form();
        form && wasSuccessfulSubmitButtonCandidate != _inputType->CanBeSuccessfulSubmitButton())
    {
      form->ResetDefaultButton();
    }

    RunPostTypeUpdateTasks();

    // https://html.spec.whatwg.org/#input-type-change
    // 8. Let nowSelectable be true if setRangeText() now applies to the element, and false otherwise.
    bool nowSelectable = _inputType->SupportsSelectionAPI();
    // 9. If previouslySelectable is false and nowSelectable is true, set the element's text entry cursor
    // position to the beginning of the text control, and set its selection direction to "none".
    if (!previouslySelectable && nowSelectable)
    {
      auto direction = TextFieldSelectionDirection::SelectionHasNoDirection;
      // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#set-the-selection-direction
      if (IsTextField() && NodeDocument().Frame()
          && NodeDocument().GetEditor().Behavior().ShouldConsiderSelectionAsDirectional())
      {
        direction = TextFieldSelectionDirection::SelectionHasForwardDirection;
      }
      CacheSelection(0, 0, direction);
    }

    UpdateValidity();
    UpdatePlaceholderVisibility();
  }

  void HTMLInputElement::RunPostTypeUpdateTasks() noexcept
  {
    krys_debug_assert(_inputType);

    if (_inputType->HasActivationBehavior())
    {
      SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
    }
    else
    {
      ClearEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
    }

    if (Renderer())
    {
      InvalidateStyleAndRenderersForSubtree();
    }

    if (NodeDocument().FocusedElement() == this)
    {
      UpdateFocusAppearance(dom::SelectionRestorationMode::RestoreOrSelectAll, SelectionRevealMode::Reveal);
    }

    SetChangedSinceLastFormControlChangeEvent(false);

    AddToRadioButtonGroup();
  }

#pragma endregion

#pragma region DOM hooks

  void HTMLInputElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value,
                                            dom::DOMStringAtom namespaceURI) noexcept
  {
    if (oldValue == value)
    {
      return;
    }

    krys_debug_assert(_inputType);
    Ref<InputType> protectedInputType = ShareRef(*_inputType);

    HTMLTextFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"type")
    {
      UpdateType(value.value_or(dom::DOMStringView()));
    }
    else if (localName == u8"value")
    {
      // We only need to invalidate if the form is looking at the default value right now.
      if (!HasDirtyValue())
      {
        UpdatePlaceholderVisibility();
        InvalidateStyleForSubtree();
        SetFormControlValueMatchesRenderer(false);
      }
      UpdateValidity();
      // FIXME: Auto directionality, with dir=auto.
      _valueAttributeWasUpdatedAfterParsing = !_parsingInProgress;
    }
    else if (localName == u8"name")
    {
      RemoveFromRadioButtonGroup();
      _name = dom::DOMString(value.value_or(dom::DOMStringView()));
      AddToRadioButtonGroup();
    }
    else if (localName == u8"checked")
    {
      SetDefaultCheckedState(value.has_value());
      // Another radio button in the same group might be checked by state restore. We shouldn't call
      // setChecked() even if this has the checked attribute. So, delay the setChecked() call until
      // finishParsingChildren() is called if parsing is in progress.
      if ((!_parsingInProgress || !NodeDocument().GetFormController().HasFormStateToRestore())
          && !_dirtyCheckednessFlag)
      {
        SetChecked(value.has_value());
        // SetChecked() above sets the dirty checkedness flag so we need to reset it.
        _dirtyCheckednessFlag = false;
      }
    }
    else if (localName == u8"autocomplete")
    {
      auto newValue = value.value_or(dom::DOMStringView());
      if (text::IsASCIICaselessEqual(newValue, dom::DOMStringView(u8"off")))
      {
        _autocomplete = Off;
      }
      else
      {
        _autocomplete = newValue.empty() ? Uninitialized : On;
      }
    }
    else if (localName == u8"maxlength")
    {
      MaxLengthAttributeChanged(value);
    }
    else if (localName == u8"minlength")
    {
      MinLengthAttributeChanged(value);
    }
    else if (localName == u8"size")
    {
      uint32 oldSize = _size;
      _size = LimitToOnlyHTMLNonNegativeNumbersGreaterThanZero(value, DefaultSize);
      if (_size != oldSize)
      {
        if (auto *renderer = Renderer())
        {
          renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
        }
      }
    }
    else if (localName == u8"max" || localName == u8"min" || localName == u8"multiple"
             || localName == u8"pattern" || localName == u8"step")
    {
      UpdateValidity();
    }
    else if (localName == u8"alpha" || localName == u8"colorspace")
    {
      if (IsColorControl())
      {
        UpdateValueIfNeeded();
        UpdateValidity();
      }
    }
    // FIXME: The list attribute (its datalist and target observer), with datalists.

    _inputType->AttributeChanged(localName);
  }

  void HTMLInputElement::OnInsert() noexcept
  {
    HTMLTextFormControlElement::OnInsert();
    // FIXME: Resetting the list attribute's target observer, with datalists.
    if (IsRadioButton())
    {
      UpdateValidity();
    }
    if (IsConnected() && _inputType->NeedsShadowSubtree() && !_inputType->HasCreatedShadowSubtree()
        && !_hasPendingUserAgentShadowTreeUpdate)
    {
      NodeDocument().AddElementWithPendingUserAgentShadowTreeUpdate(*this);
      _hasPendingUserAgentShadowTreeUpdate = true;
    }
    if (!IsConnected())
    {
      AddToRadioButtonGroup();
    }
  }

  void HTMLInputElement::OnPostConnection() noexcept
  {
    HTMLTextFormControlElement::OnPostConnection();
    if (!Form())
    {
      AddToRadioButtonGroup();
    }
  }

  void HTMLInputElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLTextFormControlElement::OnRemove(isSubtreeRoot, oldAncestor);
    bool disconnectedFromDocument = oldAncestor.IsConnected();
    bool treeScopeChanged = &oldAncestor.TreeScope() != &TreeScope();
    if (treeScopeChanged && IsRadioButton())
    {
      oldAncestor.TreeScope().RadioButtonGroups().RemoveButton(*this);
    }
    if (disconnectedFromDocument && !Form())
    {
      RemoveFromRadioButtonGroup();
    }
    if (disconnectedFromDocument && _hasPendingUserAgentShadowTreeUpdate)
    {
      NodeDocument().RemoveElementWithPendingUserAgentShadowTreeUpdate(*this);
      _hasPendingUserAgentShadowTreeUpdate = false;
    }
    krys_debug_assert(!IsConnected());
    if (disconnectedFromDocument && !Form() && IsRadioButton())
    {
      UpdateValidity();
    }
    // FIXME: Resetting the list attribute's target observer, with datalists.
  }

  void HTMLInputElement::OnClone(dom::Node &copy, bool subtree) noexcept
  {
    HTMLTextFormControlElement::OnClone(copy, subtree);

    auto &clone = Downcast<HTMLInputElement>(copy);
    clone._valueIfDirty = _valueIfDirty;
    clone._wasModifiedByUser = false;
    clone.SetChecked(_isChecked);
    clone._isDefaultChecked = _isDefaultChecked;
    clone._dirtyCheckednessFlag = _dirtyCheckednessFlag;
    clone._isIndeterminate = _isIndeterminate;

    clone.UpdateValidity();
    clone.SetFormControlValueMatchesRenderer(false);
    if (clone._inputType->HasCreatedShadowSubtree())
    {
      clone._inputType->UpdateInnerTextValue();
    }
  }

  void HTMLInputElement::UpdateUserAgentShadowTree() noexcept
  {
    krys_debug_assert(_hasPendingUserAgentShadowTreeUpdate);
    NodeDocument().RemoveElementWithPendingUserAgentShadowTreeUpdate(*this);
    _hasPendingUserAgentShadowTreeUpdate = false;
    _inputType->CreateShadowSubtreeIfNeeded();
  }

  void HTMLInputElement::FinishParsingChildren() noexcept
  {
    _parsingInProgress = false;
    krys_debug_assert(_inputType);
    HTMLTextFormControlElement::FinishParsingChildren();
    if (!_stateRestored)
    {
      if (HasAttribute(u8"checked"))
      {
        SetChecked(true);
      }
      _dirtyCheckednessFlag = false;
    }
  }

  bool HTMLInputElement::ShouldSaveAndRestoreFormControlState() const noexcept
  {
    return _inputType->ShouldSaveAndRestoreFormControlState();
  }

  FormControlState HTMLInputElement::SaveFormControlState() const noexcept
  {
    return _inputType->SaveFormControlState();
  }

  void HTMLInputElement::RestoreFormControlState(const FormControlState &state) noexcept
  {
    _inputType->RestoreFormControlState(state);
    _stateRestored = true;
  }

  bool HTMLInputElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    return _inputType->RendererIsNeeded() && HTMLTextFormControlElement::RendererIsNeeded(style);
  }

#pragma endregion

#pragma region Events

  void HTMLInputElement::LegacyPreActivationBehavior(dom::Event &event) noexcept
  {
    _clickHandlingState = {};
    _clickHandlingState.Trusted = event.IsTrusted();
    auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
    if (event.Type() == EventNames::Click && mouseEvent && mouseEvent->Button() == 0)
    {
      _inputType->WillDispatchClick(_clickHandlingState);
      _clickHandlingState.Stateful = true;
    }
  }

  void HTMLInputElement::ActivationBehavior(dom::Event &event) noexcept
  {
    Ref<InputType> protectedInputType = ShareRef(*_inputType);
    if (std::exchange(_clickHandlingState.Stateful, false))
    {
      _inputType->DidDispatchClick(event, _clickHandlingState);
    }
    _clickHandlingState.CheckedRadioButton = nullptr;

    // Image and submit inputs submit their form, and reset inputs reset it.
    _inputType->HandleActivationBehavior(event);

    // Buttons with a form owner submit or reset it, other than plain buttons.
    if (Form() && _inputType->GetType() != InputType::Type::Button)
    {
      return;
    }
    HandlePopoverTargetAction(event.Target().get());
  }

  void HTMLInputElement::LegacyCanceledActivationBehavior(dom::Event &event) noexcept
  {
    Ref<InputType> protectedInputType = ShareRef(*_inputType);
    if (std::exchange(_clickHandlingState.Stateful, false))
    {
      _inputType->DidDispatchClick(event, _clickHandlingState);
    }
    _clickHandlingState.CheckedRadioButton = nullptr;
  }

  void HTMLInputElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    Ref<InputType> protectedInputType = ShareRef(*_inputType);

    if (auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event); mouseEvent && mouseEvent->Button() == 0)
    {
      if (mouseEvent->Type() == EventNames::Click || mouseEvent->Type() == EventNames::AuxClick)
      {
        _inputType->HandleClickEvent(*mouseEvent);
      }
      else if (mouseEvent->Type() == EventNames::MouseDown)
      {
        _inputType->HandleMouseDownEvent(*mouseEvent);
      }
      // FIXME: Mouse moves (handleMouseMoveEvent), which only the color input type's picker uses.

      if (mouseEvent->DefaultHandled())
      {
        return;
      }
    }

    if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
        keyboardEvent && keyboardEvent->Type() == EventNames::KeyDown)
    {
      auto shouldCallBaseEventHandler = _inputType->HandleKeydownEvent(*keyboardEvent);
      if (event.DefaultHandled() || shouldCallBaseEventHandler == InputType::ShouldCallBaseEventHandler::No)
      {
        return;
      }
    }

    // Call the base event handler before any of our own event handling for almost all events in text fields.
    // Makes editing keyboard handling take precedence over the keydown and keypress handling in this
    // function.
    bool callBaseClassEarly =
      IsTextField() && (event.Type() == EventNames::KeyDown || event.Type() == EventNames::KeyPress);
    if (callBaseClassEarly)
    {
      HTMLTextFormControlElement::DefaultEventHandler(event);
      if (event.DefaultHandled())
      {
        return;
      }
    }

    // Use key press event here since sending simulated mouse events on key down blocks the proper sending of
    // the key press event.
    if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event))
    {
      if (keyboardEvent->Type() == EventNames::KeyPress)
      {
        _inputType->HandleKeypressEvent(*keyboardEvent);
        if (keyboardEvent->DefaultHandled())
        {
          return;
        }
      }
      else if (keyboardEvent->Type() == EventNames::KeyUp)
      {
        _inputType->HandleKeyupEvent(*keyboardEvent);
        if (keyboardEvent->DefaultHandled())
        {
          return;
        }
      }
    }

    if (_inputType->ShouldSubmitImplicitly(event))
    {
      // FIXME: Adding a recent search (addSearchResult) for search fields, with recent searches.

      // Form submission finishes editing, just as loss of focus does. If there was a change, send the event
      // now.
      if (WasChangedSinceLastFormControlChangeEvent())
      {
        DispatchFormControlChangeEvent();
      }

      // Form may never have been present, or may have been destroyed by code responding to the change event.
      if (RefPtr<HTMLFormElement> formElement = ShareRefPtr(Form()))
      {
        formElement->SubmitImplicitly(event, CanTriggerImplicitSubmission());
      }

      event.SetDefaultHandled();
      return;
    }

    if (auto *beforeTextInsertedEvent = DynamicDowncast<dom::BeforeTextInsertedEvent>(event))
    {
      _inputType->HandleBeforeTextInsertedEvent(*beforeTextInsertedEvent);
    }

    _inputType->ForwardEvent(event);

    if (!callBaseClassEarly && !event.DefaultHandled())
    {
      HTMLTextFormControlElement::DefaultEventHandler(event);
    }
  }

#pragma endregion

#pragma region HTMLInputElement Obsolete members

  dom::DOMString HTMLInputElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLInputElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLInputElement::UseMap() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"usemap").value_or(u8"");
  }

  void HTMLInputElement::UseMap(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"usemap", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLInputElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      if (localName == u8"vspace" || localName == u8"hspace" || localName == u8"width"
          || localName == u8"height")
      {
        return true;
      }
      if (localName == u8"border")
      {
        return IsImageButton();
      }
    }
    return HTMLTextFormControlElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLInputElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                dom::DOMStringAtom namespaceURI,
                                                                dom::DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLTextFormControlElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value,
                                                                         style);
      return;
    }
    if (localName == u8"vspace")
    {
      if (IsImageButton())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      }
      return;
    }
    if (localName == u8"hspace")
    {
      if (IsImageButton())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
        AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      }
      return;
    }
    if (localName == u8"align")
    {
      // Only image buttons respect the align attribute.
      if (IsImageButton())
      {
        ApplyAlignmentAttributeToStyle(value, style);
      }
      return;
    }
    if (localName == u8"width")
    {
      if (_inputType->ShouldRespectHeightAndWidthAttributes())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      }
      if (IsImageButton())
      {
        ApplyAspectRatioFromWidthAndHeightAttributesToStyle(value, AttributeOrEmpty(u8"height"), style);
      }
      return;
    }
    if (localName == u8"height")
    {
      if (_inputType->ShouldRespectHeightAndWidthAttributes())
      {
        AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      }
      if (IsImageButton())
      {
        ApplyAspectRatioFromWidthAndHeightAttributesToStyle(AttributeOrEmpty(u8"width"), value, style);
      }
      return;
    }
    if (localName == u8"border")
    {
      if (IsImageButton())
      {
        ApplyBorderAttributeToStyle(value, style);
      }
      return;
    }
    HTMLTextFormControlElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
