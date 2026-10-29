#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/BaseClickableWithKeyInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/DateTimeChooserParameters.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include <charconv>
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    constexpr int MsecPerMinute = 60 * 1'000;
    constexpr int MsecPerSecond = 1'000;
  }

  void BaseDateAndTimeInputType::DateTimeFormatValidator::VisitField(DateTimeFormat::FieldType fieldType,
                                                                     int) noexcept
  {
    using FieldType = DateTimeFormat::FieldType;
    switch (fieldType)
    {
      case FieldType::Year:            _results.Add(DateTimeFormatValidationResults::HasYear); break;

      case FieldType::Month:
      case FieldType::MonthStandAlone: _results.Add(DateTimeFormatValidationResults::HasMonth); break;

      case FieldType::WeekOfYear:      _results.Add(DateTimeFormatValidationResults::HasWeek); break;

      case FieldType::DayOfMonth:      _results.Add(DateTimeFormatValidationResults::HasDay); break;

      case FieldType::Period:          _results.Add(DateTimeFormatValidationResults::HasMeridiem); break;

      case FieldType::Hour11:
      case FieldType::Hour12:          _results.Add(DateTimeFormatValidationResults::HasHour); break;

      case FieldType::Hour23:
      case FieldType::Hour24:
        _results.Add(DateTimeFormatValidationResults::HasHour);
        _results.Add(DateTimeFormatValidationResults::HasMeridiem);
        break;

      case FieldType::Minute: _results.Add(DateTimeFormatValidationResults::HasMinute); break;

      case FieldType::Second: _results.Add(DateTimeFormatValidationResults::HasSecond); break;

      default:                break;
    }
  }

  bool BaseDateAndTimeInputType::DateTimeFormatValidator::ValidateFormat(
    dom::DOMStringView format, const BaseDateAndTimeInputType &inputType) noexcept
  {
    if (!DateTimeFormat::Parse(format, *this))
    {
      return false;
    }
    return inputType.IsValidFormat(_results);
  }

  BaseDateAndTimeInputType::~BaseDateAndTimeInputType() noexcept
  {
    CloseDateTimeChooser();
    if (_dateTimeEditElement)
    {
      _dateTimeEditElement->RemoveEditControlOwner();
    }
  }

  double BaseDateAndTimeInputType::ValueAsDate() const noexcept
  {
    return ValueAsDouble();
  }

  dom::ExceptionOr<void> BaseDateAndTimeInputType::SetValueAsDate(double value) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(SerializeWithMilliseconds(value));
    return {};
  }

  double BaseDateAndTimeInputType::ValueAsDouble() const noexcept
  {
    krys_debug_assert(Element());
    const Decimal value = ParseToNumber(Element()->Value(), Decimal::NaN());
    return value.IsFinite() ? value.ToDouble() : DateComponents::InvalidMilliseconds();
  }

  dom::ExceptionOr<void>
    BaseDateAndTimeInputType::SetValueAsDecimal(const Decimal &newValue,
                                                TextFieldEventBehavior eventBehavior) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(Serialize(newValue), eventBehavior);
    return {};
  }

  bool BaseDateAndTimeInputType::TypeMismatchFor(dom::DOMStringView value) const noexcept
  {
    return !value.empty() && !ParseToDateComponents(value);
  }

  bool BaseDateAndTimeInputType::TypeMismatch() const noexcept
  {
    krys_debug_assert(Element());
    return TypeMismatchFor(Element()->Value());
  }

  bool BaseDateAndTimeInputType::HasBadInput() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Value().empty() && _dateTimeEditElement
           && _dateTimeEditElement->EditableFieldsHaveValues();
  }

  Decimal BaseDateAndTimeInputType::DefaultValueForStepUp() const noexcept
  {
    return Decimal::FromDouble(LocalTimeNow());
  }

  Decimal BaseDateAndTimeInputType::ParseToNumber(dom::DOMStringView source,
                                                  const Decimal &defaultValue) const noexcept
  {
    auto date = ParseToDateComponents(source);
    if (!date)
    {
      return defaultValue;
    }
    double msec = date->MillisecondsSinceEpoch();
    krys_debug_assert(std::isfinite(msec));
    return Decimal::FromDouble(msec);
  }

  dom::DOMString BaseDateAndTimeInputType::Serialize(const Decimal &value) const noexcept
  {
    if (!value.IsFinite())
    {
      return {};
    }

    auto date = SetMillisecondToDateComponents(value.ToDouble());
    if (!date)
    {
      return {};
    }

    return SerializeWithComponents(*date);
  }

  dom::DOMString BaseDateAndTimeInputType::SerializeWithComponents(const DateComponents &date) const noexcept
  {
    krys_debug_assert(Element());
    Decimal step;
    if (!Element()->GetAllowedValueStep(&step) || step.Remainder(Decimal(MsecPerMinute)).IsZero())
    {
      return date.ToString();
    }
    if (step.Remainder(Decimal(MsecPerSecond)).IsZero())
    {
      return date.ToString(SecondFormat::Second);
    }
    return date.ToString(SecondFormat::Millisecond);
  }

  dom::DOMString BaseDateAndTimeInputType::SerializeWithMilliseconds(double value) const noexcept
  {
    return Serialize(Decimal::FromDouble(value));
  }

  dom::DOMString BaseDateAndTimeInputType::LocalizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    auto date = ParseToDateComponents(proposedValue);
    if (!date)
    {
      return dom::DOMString(proposedValue);
    }

    krys_debug_assert(Element());
    auto localized = Element()->GetLocale().FormatDateTime(*date);
    return localized.empty() ? dom::DOMString(proposedValue) : localized;
  }

  dom::DOMString BaseDateAndTimeInputType::VisibleValue() const noexcept
  {
    krys_debug_assert(Element());
    return LocalizeValue(Element()->Value());
  }

  dom::DOMString BaseDateAndTimeInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    if (TypeMismatchFor(proposedValue))
    {
      return {};
    }
    return dom::DOMString(proposedValue);
  }

  bool BaseDateAndTimeInputType::SupportsReadOnly() const noexcept
  {
    return true;
  }

  bool BaseDateAndTimeInputType::ValueMissing(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    return Element()->IsMutable() && Element()->IsRequired() && value.empty();
  }

  dom::DOMString BaseDateAndTimeInputType::PadNumber(uint32 value, size_t width) noexcept
  {
    char buffer[16];
    auto end = std::to_chars(buffer, buffer + sizeof(buffer), value).ptr;
    auto length = static_cast<size_t>(end - buffer);
    dom::DOMString result(length < width ? width - length : 0, u8'0');
    result.append(reinterpret_cast<const char8 *>(buffer), length);
    return result;
  }

  bool BaseDateAndTimeInputType::ShouldHaveSecondField(const DateComponents &date) const noexcept
  {
    if (date.Second())
    {
      return true;
    }

    auto stepRange = CreateStepRange(AnyStepHandling::Default);
    return !stepRange.Minimum().Remainder(Decimal(MsecPerMinute)).IsZero()
           || !stepRange.Step().Remainder(Decimal(MsecPerMinute)).IsZero();
  }

  bool BaseDateAndTimeInputType::ShouldHaveMillisecondField(const DateComponents &date) const noexcept
  {
    if (date.Millisecond())
    {
      return true;
    }

    auto stepRange = CreateStepRange(AnyStepHandling::Default);
    return !stepRange.Minimum().Remainder(Decimal(MsecPerSecond)).IsZero()
           || !stepRange.Step().Remainder(Decimal(MsecPerSecond)).IsZero();
  }

  void BaseDateAndTimeInputType::SetValue(dom::DOMStringView value, bool valueChanged,
                                          TextFieldEventBehavior eventBehavior,
                                          TextControlSetValueSelection selection) noexcept
  {
    InputType::SetValue(value, valueChanged, eventBehavior, selection);
    if (valueChanged)
    {
      UpdateInnerTextValue();
    }
  }

  void BaseDateAndTimeInputType::HandleActivationBehavior(dom::Event &event) noexcept
  {
    krys_debug_assert(Element());
    // FIXME: Only while processing a user gesture (UserGestureIndicator).
    if (!Element()->Renderer() || !Element()->IsMutable())
    {
      return;
    }

    _pickerWasActivatedByKeyboard = Is<dom::KeyboardEvent>(event);

    if (_dateTimeChooser)
    {
      return;
    }

    ShowPicker();
  }

  void BaseDateAndTimeInputType::ShowPicker() noexcept
  {
    if (!Element() || !Element()->Renderer())
    {
      return;
    }

    auto *chromeClient = GetChromeClient();
    if (!chromeClient)
    {
      return;
    }

    DateTimeChooserParameters parameters;
    if (!SetupDateTimeChooserParameters(parameters))
    {
      return;
    }

    _dateTimeChooser = chromeClient->CreateDateTimeChooser(*this);
    if (RefPtr<DateTimeChooser> dateTimeChooser = _dateTimeChooser)
    {
      dateTimeChooser->ShowChooser(parameters);
    }
  }

  void BaseDateAndTimeInputType::CreateShadowSubtree() noexcept
  {
    krys_debug_assert(NeedsShadowSubtree());
    krys_debug_assert(Element());

    auto &element = *Element();
    auto &shadowRoot = *element.UserAgentShadowRoot();
    // The fields are focused instead of the input, and sequential navigation moves between them.
    shadowRoot.SetUserAgentDelegatesFocus(true);

    auto dateTimeEditElement = DateTimeEditElement::Create(element.NodeDocument(), *this);
    _dateTimeEditElement = dateTimeEditElement;
    (void)shadowRoot.AppendChild(*dateTimeEditElement);
    UpdateInnerTextValue();
  }

  void BaseDateAndTimeInputType::RemoveShadowSubtree() noexcept
  {
    InputType::RemoveShadowSubtree();
    if (_dateTimeEditElement)
    {
      _dateTimeEditElement->RemoveEditControlOwner();
      _dateTimeEditElement = nullptr;
    }
    if (auto *shadowRoot = Element() ? Element()->UserAgentShadowRoot() : nullptr)
    {
      shadowRoot->SetUserAgentDelegatesFocus(false);
    }
  }

  void BaseDateAndTimeInputType::UpdateInnerTextValue() noexcept
  {
    krys_debug_assert(Element());

    CreateShadowSubtreeIfNeeded();

    if (!_dateTimeEditElement)
    {
      return;
    }

    auto &input = *Element();
    DateTimeEditElement::LayoutParameters layoutParameters(input.GetLocale());

    auto date = ParseToDateComponents(input.Value());
    if (date)
    {
      SetupLayoutParameters(layoutParameters, *date);
    }
    else if (auto dateForLayout =
               SetMillisecondToDateComponents(CreateStepRange(AnyStepHandling::Default).Minimum().ToDouble()))
    {
      SetupLayoutParameters(layoutParameters, *dateForLayout);
    }
    else
    {
      SetupLayoutParameters(layoutParameters, DateComponents());
    }

    if (!DateTimeFormatValidator().ValidateFormat(layoutParameters.DateTimeFormat, *this))
    {
      layoutParameters.DateTimeFormat = layoutParameters.FallbackDateTimeFormat;
    }

    Ref<DateTimeEditElement> dateTimeEditElement = ShareRef(*_dateTimeEditElement);
    if (date)
    {
      dateTimeEditElement->SetValueAsDate(layoutParameters, *date);
    }
    else
    {
      dateTimeEditElement->SetEmptyValue(layoutParameters);
    }
  }

  void BaseDateAndTimeInputType::AttributeChanged(dom::DOMStringAtom name) noexcept
  {
    if (name == u8"max" || name == u8"min")
    {
      if (auto *element = Element())
      {
        element->InvalidateStyleForSubtree();
      }
    }
    else if (name == u8"value")
    {
      if (auto *element = Element())
      {
        if (!element->HasDirtyValue())
        {
          UpdateInnerTextValue();
        }
      }
    }
    else if (name == u8"step")
    {
      if (_dateTimeEditElement)
      {
        UpdateInnerTextValue();
      }
    }

    InputType::AttributeChanged(name);
  }

  void BaseDateAndTimeInputType::ElementDidBlur() noexcept
  {
    if (!_dateTimeEditElement)
    {
      CloseDateTimeChooser();
    }
  }

  void BaseDateAndTimeInputType::Detach() noexcept
  {
    CloseDateTimeChooser();
  }

  bool BaseDateAndTimeInputType::IsPresentingAttachedView() const noexcept
  {
    return !!_dateTimeChooser;
  }

  InputType::ShouldCallBaseEventHandler
    BaseDateAndTimeInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    return BaseClickableWithKeyInputType::HandleKeydownEvent(*Element(), event);
  }

  void BaseDateAndTimeInputType::HandleKeypressEvent(dom::KeyboardEvent &event) noexcept
  {
    // The return key should not activate the element, as it conflicts with the key binding to submit a form.
    if (event.CharCode() == '\r')
    {
      return;
    }

    krys_debug_assert(Element());
    BaseClickableWithKeyInputType::HandleKeypressEvent(*Element(), event);
  }

  void BaseDateAndTimeInputType::HandleKeyupEvent(dom::KeyboardEvent &event) noexcept
  {
    BaseClickableWithKeyInputType::HandleKeyupEvent(*this, event);
  }

  void BaseDateAndTimeInputType::DidBlurFromControl() noexcept
  {
    CloseDateTimeChooser();

    auto *element = Element();
    if (element && element->WasChangedSinceLastFormControlChangeEvent())
    {
      element->DispatchFormControlChangeEvent();
    }
  }

  void BaseDateAndTimeInputType::DidChangeValueFromControl() noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> input = ShareRef(*Element());

    auto value = SanitizeValue(_dateTimeEditElement->Value());
    bool valueChanged = value != input->Value();

    InputType::SetValue(value, valueChanged, TextFieldEventBehavior::DispatchNoEvent,
                        TextControlSetValueSelection::DoNotSet);

    if (!valueChanged)
    {
      // FIXME: Telling accessibility about the partial value (AXObjectCache::valueChanged), with
      // accessibility.
      return;
    }

    if (input->UserAgentShadowRoot()->ContainsFocusedElement())
    {
      input->DispatchFormControlInputEvent();
    }
    else
    {
      input->DispatchFormControlChangeEvent();
    }

    DateTimeChooserParameters parameters;
    if (!SetupDateTimeChooserParameters(parameters))
    {
      return;
    }

    if (RefPtr<DateTimeChooser> dateTimeChooser = _dateTimeChooser)
    {
      dateTimeChooser->ShowChooser(parameters);
    }
  }

  void BaseDateAndTimeInputType::DidReceiveSpaceKeyFromControl() noexcept
  {
    // One of our subfields received a space key event, so let's move focus into the picker.
    _pickerWasActivatedByKeyboard = true;

    RefPtr<DateTimeChooser> chooser = _dateTimeChooser;
    if (!chooser)
    {
      ShowPicker();
      return;
    }

    DateTimeChooserParameters parameters;
    if (SetupDateTimeChooserParameters(parameters))
    {
      chooser->ShowChooser(parameters);
    }
  }

  bool BaseDateAndTimeInputType::IsEditControlOwnerDisabled() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->IsDisabledFormControl();
  }

  bool BaseDateAndTimeInputType::IsEditControlOwnerReadOnly() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->IsReadOnly();
  }

  dom::DOMString BaseDateAndTimeInputType::LocaleIdentifier() const noexcept
  {
    krys_debug_assert(Element());
    return css::EffectiveLang(*Element());
  }

  void BaseDateAndTimeInputType::DidChooseValue(utf8_stringview value) noexcept
  {
    if (auto *element = Element())
    {
      (void)element->SetValue(value, TextFieldEventBehavior::DispatchInputAndChangeEvent);
    }
  }

  bool
    BaseDateAndTimeInputType::SetupDateTimeChooserParameters(DateTimeChooserParameters &parameters) noexcept
  {
    krys_debug_assert(Element());

    auto &element = *Element();
    auto &document = element.NodeDocument();

    auto *view = document.View();
    if (!view)
    {
      return false;
    }

    parameters.Type = element.Type();
    parameters.Minimum = element.Minimum();
    parameters.Maximum = element.Maximum();
    parameters.Required = element.IsRequired();

    if (!document.GetSettings().LangAttributeAwareFormControlUIEnabled)
    {
      parameters.Locale = DefaultLanguage();
    }
    else
    {
      auto computedLocale = css::EffectiveLang(element);
      parameters.Locale = computedLocale.empty() ? DefaultLanguage() : computedLocale;
    }

    auto stepRange = CreateStepRange(AnyStepHandling::Reject);
    if (stepRange.HasStep())
    {
      parameters.Step = stepRange.Step().ToDouble();
      parameters.StepBase = stepRange.StepBase().ToDouble();
    }
    else
    {
      parameters.Step = 1.0;
      parameters.StepBase = 0;
    }

    auto *renderer = element.Renderer();
    parameters.AnchorRectInRootView =
      renderer ? view->ContentsToView(renderer->AbsoluteBoundingBoxRect()) : IntRect();
    parameters.CurrentValue = element.Value();

    if (renderer)
    {
      parameters.IsAnchorElementRTL =
        renderer->Style().GetWritingMode().ComputedTextDirection() == TextDirection::RTL;
      parameters.UseDarkAppearance = renderer->UseDarkAppearance();
    }
    auto date = ParseToDateComponents(element.Value()).value_or(DateComponents());
    parameters.HasSecondField = ShouldHaveSecondField(date);
    parameters.HasMillisecondField = ShouldHaveMillisecondField(date);
    parameters.WasActivatedByKeyboard = _pickerWasActivatedByKeyboard;

    // FIXME: The suggestions of the input's datalist, once there are datalists.

    return true;
  }

  void BaseDateAndTimeInputType::CloseDateTimeChooser() noexcept
  {
    if (RefPtr<DateTimeChooser> dateTimeChooser = _dateTimeChooser)
    {
      dateTimeChooser->EndChooser();
    }

    _pickerWasActivatedByKeyboard = false;
  }
}
