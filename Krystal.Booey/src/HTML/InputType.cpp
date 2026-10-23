#include "Krystal.Booey/HTML/InputType.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/ButtonInputType.hpp"
#include "Krystal.Booey/HTML/CheckboxInputType.hpp"
#include "Krystal.Booey/HTML/ColorInputType.hpp"
#include "Krystal.Booey/HTML/DateInputType.hpp"
#include "Krystal.Booey/HTML/DateTimeLocalInputType.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/EmailInputType.hpp"
#include "Krystal.Booey/HTML/FileInputType.hpp"
#include "Krystal.Booey/HTML/HiddenInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/ImageInputType.hpp"
#include "Krystal.Booey/HTML/InputTypeNames.hpp"
#include "Krystal.Booey/HTML/MonthInputType.hpp"
#include "Krystal.Booey/HTML/NumberInputType.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/HTML/PasswordInputType.hpp"
#include "Krystal.Booey/HTML/RadioInputType.hpp"
#include "Krystal.Booey/HTML/RangeInputType.hpp"
#include "Krystal.Booey/HTML/ResetInputType.hpp"
#include "Krystal.Booey/HTML/SearchInputType.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/HTML/SubmitInputType.hpp"
#include "Krystal.Booey/HTML/TelephoneInputType.hpp"
#include "Krystal.Booey/HTML/TextInputType.hpp"
#include "Krystal.Booey/HTML/TimeInputType.hpp"
#include "Krystal.Booey/HTML/URLInputType.hpp"
#include "Krystal.Booey/HTML/WeekInputType.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include <array>

namespace krys::boo::html
{
  namespace
  {
    using Type = InputType::Type;

    constexpr EnumSet<Type> TextTypes {Type::Email,     Type::Password, Type::Search,
                                       Type::Telephone, Type::Text,     Type::URL};

    constexpr EnumSet<Type> TextFieldTypes {Type::Email,     Type::Number, Type::Password, Type::Search,
                                            Type::Telephone, Type::Text,   Type::URL};

    constexpr EnumSet<Type> TextButtonTypes {Type::Button, Type::Reset, Type::Submit};

    constexpr EnumSet<Type> CheckableTypes {Type::Checkbox, Type::Radio};

    constexpr EnumSet<Type> SteppableTypes {Type::Date, Type::DateTimeLocal, Type::Month, Type::Time,
                                            Type::Week, Type::Number,        Type::Range};

    constexpr EnumSet<Type> NonValidatingTypes {Type::Button, Type::Hidden, Type::Image, Type::Reset};

    // The types whose activation behavior does something: check, submit, reset, or open a picker.
    constexpr EnumSet<Type> ActivationBehaviorTypes {
      Type::Button, Type::Checkbox, Type::Color, Type::Date,   Type::DateTimeLocal, Type::File, Type::Image,
      Type::Month,  Type::Radio,    Type::Reset, Type::Submit, Type::Time,          Type::Week};

    using InputTypeFactoryFunction = Ref<InputType> (*)(HTMLInputElement &);

    template <typename T>
    KRYS_NODISCARD Ref<InputType> CreateInputType(HTMLInputElement &element) noexcept
    {
      return CreateRef<T>(element);
    }

    struct InputTypeFactory
    {
      dom::DOMStringView Name;
      InputTypeFactoryFunction FactoryFunction;
    };

    constexpr std::array<InputTypeFactory, 22> InputTypeFactories {{
      {InputTypeNames::Button, &CreateInputType<ButtonInputType>},
      {InputTypeNames::Checkbox, &CreateInputType<CheckboxInputType>},
      {InputTypeNames::Color, &CreateInputType<ColorInputType>},
      {InputTypeNames::Date, &CreateInputType<DateInputType>},
      {InputTypeNames::DateTimeLocal, &CreateInputType<DateTimeLocalInputType>},
      {InputTypeNames::Email, &CreateInputType<EmailInputType>},
      {InputTypeNames::File, &CreateInputType<FileInputType>},
      {InputTypeNames::Hidden, &CreateInputType<HiddenInputType>},
      {InputTypeNames::Image, &CreateInputType<ImageInputType>},
      {InputTypeNames::Month, &CreateInputType<MonthInputType>},
      {InputTypeNames::Number, &CreateInputType<NumberInputType>},
      {InputTypeNames::Password, &CreateInputType<PasswordInputType>},
      {InputTypeNames::Radio, &CreateInputType<RadioInputType>},
      {InputTypeNames::Range, &CreateInputType<RangeInputType>},
      {InputTypeNames::Reset, &CreateInputType<ResetInputType>},
      {InputTypeNames::Search, &CreateInputType<SearchInputType>},
      {InputTypeNames::Submit, &CreateInputType<SubmitInputType>},
      {InputTypeNames::Telephone, &CreateInputType<TelephoneInputType>},
      {InputTypeNames::Text, &CreateInputType<TextInputType>},
      {InputTypeNames::Time, &CreateInputType<TimeInputType>},
      {InputTypeNames::URL, &CreateInputType<URLInputType>},
      {InputTypeNames::Week, &CreateInputType<WeekInputType>},
    }};

    KRYS_NODISCARD const InputTypeFactory *FindFactory(dom::DOMStringView typeName) noexcept
    {
      for (auto &factory : InputTypeFactories)
      {
        if (text::IsASCIICaselessEqual(factory.Name, typeName))
        {
          return &factory;
        }
      }
      return nullptr;
    }
  }

  RefPtr<InputType> InputType::CreateIfDifferent(HTMLInputElement &element, dom::DOMStringView typeName,
                                                 InputType *currentInputType) noexcept
  {
    if (!typeName.empty())
    {
      auto currentTypeName = currentInputType ? currentInputType->FormControlType() : dom::DOMStringView();
      if (typeName == currentTypeName)
      {
        return nullptr;
      }
      if (auto *factory = FindFactory(typeName)) KRYS_LIKELY
      {
        if (factory->Name == currentTypeName)
        {
          return nullptr;
        }
        return factory->FactoryFunction(element);
      }
    }
    if (currentInputType && currentInputType->GetType() == Type::Text)
    {
      return nullptr;
    }
    return CreateRef<TextInputType>(element);
  }

  InputType::~InputType() noexcept = default;

  bool InputType::IsTextButton() const noexcept
  {
    return TextButtonTypes.Contains(_type);
  }

  bool InputType::IsTextField() const noexcept
  {
    return TextFieldTypes.Contains(_type);
  }

  bool InputType::IsTextType() const noexcept
  {
    return TextTypes.Contains(_type);
  }

  bool InputType::IsCheckable() const noexcept
  {
    return CheckableTypes.Contains(_type);
  }

  bool InputType::IsSteppable() const noexcept
  {
    return SteppableTypes.Contains(_type);
  }

  bool InputType::SupportsValidation() const noexcept
  {
    return !NonValidatingTypes.Contains(_type);
  }

  bool InputType::HasActivationBehavior() const noexcept
  {
    return ActivationBehaviorTypes.Contains(_type);
  }

  bool InputType::IsValidValue(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(CanSetStringValue());
    return !TypeMismatchFor(value) && !StepMismatch(value) && !RangeUnderflow(value) && !RangeOverflow(value)
           && !PatternMismatch(value) && !ValueMissing(value);
  }

  bool InputType::IsFormDataAppendable() const noexcept
  {
    krys_debug_assert(Element());
    // There is no form data unless there's a name for non-image types.
    return !Element()->Name().empty();
  }

  bool InputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    // Always successful.
    auto &input = *Element();
    formData.Append(input.Name(), input.Value());
    return true;
  }

  double InputType::ValueAsDate() const noexcept
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  dom::ExceptionOr<void> InputType::SetValueAsDate(double) const noexcept
  {
    return dom::Exception {dom::ExceptionCode::InvalidStateError};
  }

  double InputType::ValueAsDouble() const noexcept
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  dom::ExceptionOr<void> InputType::SetValueAsDouble(double doubleValue,
                                                     TextFieldEventBehavior eventBehavior) const noexcept
  {
    return SetValueAsDecimal(Decimal::FromDouble(doubleValue), eventBehavior);
  }

  dom::ExceptionOr<void> InputType::SetValueAsDecimal(const Decimal &, TextFieldEventBehavior) const noexcept
  {
    return dom::Exception {dom::ExceptionCode::InvalidStateError};
  }

  bool InputType::SupportsRequired() const noexcept
  {
    // Almost all validatable types support @required.
    return SupportsValidation();
  }

  bool InputType::RangeUnderflow(dom::DOMStringView value) const noexcept
  {
    if (!IsSteppable())
    {
      return false;
    }

    const Decimal numericValue = ParseToNumberOrNaN(value);
    if (!numericValue.IsFinite())
    {
      return false;
    }

    auto range = CreateStepRange(AnyStepHandling::Reject);
    if (range.IsReversibleRange() && range.Maximum() < range.Minimum())
    {
      return numericValue > range.Maximum() && numericValue < range.Minimum();
    }

    return numericValue < range.Minimum();
  }

  bool InputType::RangeOverflow(dom::DOMStringView value) const noexcept
  {
    if (!IsSteppable())
    {
      return false;
    }

    const Decimal numericValue = ParseToNumberOrNaN(value);
    if (!numericValue.IsFinite())
    {
      return false;
    }

    auto range = CreateStepRange(AnyStepHandling::Reject);
    if (range.IsReversibleRange() && range.Maximum() < range.Minimum())
    {
      return numericValue > range.Maximum() && numericValue < range.Minimum();
    }

    return numericValue > range.Maximum();
  }

  bool InputType::IsInvalid(dom::DOMStringView value) const noexcept
  {
    return TypeMismatch() || StepMismatch(value) || RangeUnderflow(value) || RangeOverflow(value)
           || PatternMismatch(value) || ValueMissing(value) || HasBadInput();
  }

  Decimal InputType::DefaultValueForStepUp() const noexcept
  {
    return Decimal(0);
  }

  double InputType::Minimum() const noexcept
  {
    return CreateStepRange(AnyStepHandling::Reject).Minimum().ToDouble();
  }

  double InputType::Maximum() const noexcept
  {
    return CreateStepRange(AnyStepHandling::Reject).Maximum().ToDouble();
  }

  bool InputType::IsInRange(dom::DOMStringView value) const noexcept
  {
    if (!IsSteppable())
    {
      return false;
    }

    StepRange stepRange(CreateStepRange(AnyStepHandling::Reject));
    if (!stepRange.HasRangeLimitations())
    {
      return false;
    }

    // This function should return true if both of validity.rangeUnderflow and validity.rangeOverflow are
    // false. If the INPUT has no value, they are false.
    const Decimal numericValue = ParseToNumberOrNaN(value);
    if (!numericValue.IsFinite())
    {
      return true;
    }

    return numericValue >= stepRange.Minimum() && numericValue <= stepRange.Maximum();
  }

  bool InputType::IsOutOfRange(dom::DOMStringView value) const noexcept
  {
    if (!IsSteppable() || value.empty())
    {
      return false;
    }

    StepRange stepRange(CreateStepRange(AnyStepHandling::Reject));
    if (!stepRange.HasRangeLimitations())
    {
      return false;
    }

    // This function should return true if both of validity.rangeUnderflow and validity.rangeOverflow are
    // true. If the INPUT has no value, they are false.
    const Decimal numericValue = ParseToNumberOrNaN(value);
    if (!numericValue.IsFinite())
    {
      return false;
    }

    return numericValue < stepRange.Minimum() || numericValue > stepRange.Maximum();
  }

  bool InputType::StepMismatch(dom::DOMStringView value) const noexcept
  {
    if (!IsSteppable())
    {
      return false;
    }

    const Decimal numericValue = ParseToNumberOrNaN(value);
    if (!numericValue.IsFinite())
    {
      return false;
    }

    return CreateStepRange(AnyStepHandling::Reject).StepMismatch(numericValue);
  }

  dom::DOMString InputType::BadInputText() const noexcept
  {
    krys_debug_assert(false);
    return ValidationMessageTypeMismatchText();
  }

  dom::DOMString InputType::TypeMismatchText() const noexcept
  {
    return ValidationMessageTypeMismatchText();
  }

  dom::DOMString InputType::ValueMissingText() const noexcept
  {
    return ValidationMessageValueMissingText();
  }

  dom::DOMString InputType::ValidationMessage() const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto value = element.Value();

    // The order of the following checks is meaningful. e.g. We'd like to show the badInput message even if
    // the control has other validation errors.
    if (HasBadInput())
    {
      return BadInputText();
    }

    if (ValueMissing(value))
    {
      return ValueMissingText();
    }

    if (TypeMismatch())
    {
      return TypeMismatchText();
    }

    if (PatternMismatch(value))
    {
      auto title = infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(
        element.GetAttribute(u8"title").value_or(dom::DOMString()));
      if (title.empty())
      {
        return ValidationMessagePatternMismatchText();
      }
      return ValidationMessagePatternMismatchText(title);
    }

    auto length = static_cast<int>(HTMLTextFormControlElement::LengthForAPIValue(value));
    if (element.TooShort())
    {
      return ValidationMessageTooShortText(length, element.MinLength());
    }

    if (element.TooLong())
    {
      return ValidationMessageTooLongText(length, static_cast<int>(element.EffectiveMaxLength()));
    }

    if (!IsSteppable())
    {
      return {};
    }

    const Decimal numericValue = ParseToNumberOrNaN(value);
    if (!numericValue.IsFinite())
    {
      return {};
    }

    StepRange stepRange(CreateStepRange(AnyStepHandling::Reject));

    if (numericValue < stepRange.Minimum())
    {
      return ValidationMessageRangeUnderflowText(Serialize(stepRange.Minimum()));
    }

    if (numericValue > stepRange.Maximum())
    {
      return ValidationMessageRangeOverflowText(Serialize(stepRange.Maximum()));
    }

    if (stepRange.StepMismatch(numericValue))
    {
      auto stepString = stepRange.HasStep()
                          ? SerializeForNumberType(stepRange.Step() / Decimal(stepRange.StepScaleFactor()))
                          : dom::DOMString();
      return ValidationMessageStepMismatchText(Serialize(stepRange.StepBase()), stepString);
    }

    return {};
  }

  InputType::ShouldCallBaseEventHandler InputType::HandleKeydownEvent(dom::KeyboardEvent &) noexcept
  {
    return ShouldCallBaseEventHandler::Yes;
  }

  void InputType::HandleKeypressEvent(dom::KeyboardEvent &) noexcept
  {
  }

  void InputType::HandleKeyupEvent(dom::KeyboardEvent &) noexcept
  {
  }

  bool InputType::ShouldSubmitImplicitly(dom::Event &event) noexcept
  {
    auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event);
    return keyboardEvent && event.Type() == EventNames::KeyPress && keyboardEvent->CharCode() == '\r';
  }

  Decimal InputType::ParseToNumber(dom::DOMStringView, const Decimal &defaultValue) const noexcept
  {
    krys_debug_assert(false);
    return defaultValue;
  }

  Decimal InputType::ParseToNumberOrNaN(dom::DOMStringView string) const noexcept
  {
    return ParseToNumber(string, Decimal::NaN());
  }

  dom::DOMString InputType::Serialize(const Decimal &) const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  DateComponentsType InputType::DateType() const noexcept
  {
    return DateComponentsType::Invalid;
  }

  void InputType::DispatchSimulatedClickIfActive(dom::KeyboardEvent &event) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    if (element.Active())
    {
      (void)element.DispatchSimulatedClick(&event);
    }
    event.SetDefaultHandled();
  }

  bool InputType::CanSetStringValue() const noexcept
  {
    return true;
  }

  bool InputType::CanBeSuccessfulSubmitButton() noexcept
  {
    return false;
  }

  bool InputType::NeedsShadowSubtree() const noexcept
  {
    switch (_type)
    {
      case Type::Button:
      case Type::Checkbox:
      case Type::Hidden:
      case Type::Image:
      case Type::Radio:
      case Type::Reset:
      case Type::Submit:   return false;
      default:             return true;
    }
  }

  void InputType::SubtreeHasChanged() noexcept
  {
    krys_debug_assert_msg(false, "Only text fields have a subtree the user edits.");
  }

  void InputType::CreateShadowSubtreeIfNeeded() noexcept
  {
    if (_hasCreatedShadowSubtree || !NeedsShadowSubtree())
    {
      return;
    }
    Ref<InputType> protectedThis = ShareRef(*this);
    Element()->EnsureUserAgentShadowRoot();
    _hasCreatedShadowSubtree = true;
    CreateShadowSubtree();
  }

  void InputType::RemoveShadowSubtree() noexcept
  {
    krys_debug_assert(Element());
    auto *root = Element()->UserAgentShadowRoot();
    if (!root)
    {
      return;
    }

    while (auto *child = root->FirstChild())
    {
      (void)root->RemoveChild(*child);
    }
    _hasCreatedShadowSubtree = false;
  }

  void InputType::StepUpFromRenderer(int n) noexcept
  {
    // The differences from stepUp()/stepDown():
    //
    // Difference 1: the current value
    // If the current value is not a number, including empty, the current value is assumed as 0.
    //   * If 0 is in-range, and matches to step value
    //     - The value should be the +step if n > 0
    //     - The value should be the -step if n < 0
    //     If -step or +step is out of range, new value should be 0.
    //   * If 0 is smaller than the minimum value
    //     - The value should be the minimum value for any n
    //   * If 0 is larger than the maximum value
    //     - The value should be the maximum value for any n
    //   * If 0 is in-range, but not matched to step value
    //     - The value should be the larger matched value nearest to 0 if n > 0
    //       e.g. <input type=number min=-100 step=3> -> 2
    //     - The value should be the smaller matched value nearest to 0 if n < 0
    //       e.g. <input type=number min=-100 step=3> -> -1
    //   As for date/datetime-local/month/time/week types, the current value is assumed as "the current local
    //   date/time". As for datetime type, the current value is assumed as "the current date/time in UTC".
    // If the current value is smaller than the minimum value:
    //  - The value should be the minimum value if n > 0
    //  - Nothing should happen if n < 0
    // If the current value is larger than the maximum value:
    //  - The value should be the maximum value if n < 0
    //  - Nothing should happen if n > 0
    //
    // Difference 2: clamping steps
    // If the current value is not matched to step value:
    // - The value should be the larger matched value nearest to 0 if n > 0
    //   e.g. <input type=number value=3 min=-100 step=3> -> 5
    // - The value should be the smaller matched value nearest to 0 if n < 0
    //   e.g. <input type=number value=3 min=-100 step=3> -> 2
    //
    // n is assumed as -n if step < 0.

    krys_debug_assert(IsSteppable());
    if (!IsSteppable())
    {
      return;
    }
    krys_debug_assert(n);
    if (!n)
    {
      return;
    }

    StepRange stepRange(CreateStepRange(AnyStepHandling::Default));

    // FIXME: Not any changes after stepping, even if it is an invalid value, may be better.
    // (e.g. Stepping-up for <input type="number" value="foo" step="any" /> => "foo")
    if (!stepRange.HasStep())
    {
      return;
    }

    const Decimal step = stepRange.Step();

    int sign;
    if (step > 0)
    {
      sign = n;
    }
    else if (step < 0)
    {
      sign = -n;
    }
    else
    {
      sign = 0;
    }

    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    auto currentStringValue = element->Value();
    Decimal current = ParseToNumberOrNaN(currentStringValue);
    if (!current.IsFinite())
    {
      current = DefaultValueForStepUp();
      const Decimal nextDiff = step * n;
      if (current < stepRange.Minimum() - nextDiff)
      {
        current = stepRange.Minimum() - nextDiff;
      }
      if (current > stepRange.Maximum() - nextDiff)
      {
        current = stepRange.Maximum() - nextDiff;
      }
      (void)SetValueAsDecimal(current, TextFieldEventBehavior::DispatchNoEvent);
    }
    if ((sign > 0 && current < stepRange.Minimum()) || (sign < 0 && current > stepRange.Maximum()))
    {
      (void)SetValueAsDecimal(sign > 0 ? stepRange.Minimum() : stepRange.Maximum(),
                              TextFieldEventBehavior::DispatchInputAndChangeEvent);
    }
    else
    {
      if (StepMismatch(element->Value()))
      {
        krys_debug_assert(!step.IsZero());
        const Decimal base = stepRange.StepBase();
        Decimal newValue;
        if (sign < 0)
        {
          newValue = base + ((current - base) / step).Floor() * step;
        }
        else if (sign > 0)
        {
          newValue = base + ((current - base) / step).Ceil() * step;
        }
        else
        {
          newValue = current;
        }

        if (newValue < stepRange.Minimum())
        {
          newValue = stepRange.Minimum();
        }
        if (newValue > stepRange.Maximum())
        {
          newValue = stepRange.Maximum();
        }

        (void)SetValueAsDecimal(newValue, n == 1 || n == -1
                                            ? TextFieldEventBehavior::DispatchInputAndChangeEvent
                                            : TextFieldEventBehavior::DispatchNoEvent);
        if (n > 1)
        {
          (void)ApplyStep(n - 1, AnyStepHandling::Default,
                          TextFieldEventBehavior::DispatchInputAndChangeEvent);
        }
        else if (n < -1)
        {
          (void)ApplyStep(n + 1, AnyStepHandling::Default,
                          TextFieldEventBehavior::DispatchInputAndChangeEvent);
        }
      }
      else
      {
        (void)ApplyStep(n, AnyStepHandling::Default, TextFieldEventBehavior::DispatchInputAndChangeEvent);
      }
    }
  }

  RefPtr<TextControlInnerTextElement> InputType::InnerTextElement() const noexcept
  {
    return nullptr;
  }

  RefPtr<TextControlInnerTextElement> InputType::InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept
  {
    CreateShadowSubtreeIfNeeded();
    return InnerTextElement();
  }

  RenderPtr<RenderElement> InputType::CreateInputRenderer(RenderStyle &&style) noexcept
  {
    krys_debug_assert(Element());
    return RenderElement::CreateFor(*Element(), krys::move(style));
  }

  bool InputType::SizeShouldIncludeDecoration(int32, int32 &preferredSize) const noexcept
  {
    krys_debug_assert(Element());
    preferredSize = static_cast<int32>(Element()->Size());
    return false;
  }

  float InputType::DecorationWidth(float) const noexcept
  {
    return 0;
  }

  bool InputType::RendererIsNeeded() noexcept
  {
    return true;
  }

  Maybe<dom::DOMString> InputType::FallbackValue() const noexcept
  {
    return std::nullopt;
  }

  Maybe<dom::DOMString> InputType::DefaultValue() const noexcept
  {
    return std::nullopt;
  }

  bool InputType::ShouldSendChangeEventAfterCheckedChanged() noexcept
  {
    return true;
  }

  bool InputType::StoresValueSeparateFromAttribute() noexcept
  {
    return true;
  }

  void InputType::SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                           TextFieldEventBehavior eventBehavior, TextControlSetValueSelection) noexcept
  {
    auto *element = Element();
    krys_debug_assert(element);
    if (!valueChanged)
    {
      element->SetValueInternal(sanitizedValue, eventBehavior);
      return;
    }

    bool wasInRange = IsInRange(element->Value());
    bool inRange = IsInRange(sanitizedValue);

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (wasInRange != inRange)
    {
      styleInvalidation.emplace(*element, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                            {css::SelectorPseudoClass::InRange, inRange},
                                            {css::SelectorPseudoClass::OutOfRange, !inRange},
                                          });
    }

    element->SetValueInternal(sanitizedValue, eventBehavior);

    // FIXME: Updating the auto directionality the value decides, once there's auto directionality.

    switch (eventBehavior)
    {
      case TextFieldEventBehavior::DispatchChangeEvent: element->DispatchFormControlChangeEvent(); break;
      case TextFieldEventBehavior::DispatchInputAndChangeEvent:
        element->DispatchFormControlInputEvent();
        element->DispatchFormControlChangeEvent();
        break;
      case TextFieldEventBehavior::DispatchNoEvent: break;
    }

    // FIXME: Telling accessibility (AXObjectCache::valueChanged), once there's accessibility.
  }

  dom::DOMString InputType::VisibleValue() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Value();
  }

  dom::DOMString InputType::LocalizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    return dom::DOMString(proposedValue);
  }

  ChromeClient *InputType::GetChromeClient() const noexcept
  {
    if (!Element())
    {
      return nullptr;
    }
    auto *page = Element()->NodeDocument().GetPage();
    return page ? &page->GetChromeClient() : nullptr;
  }

  dom::DOMString InputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    return dom::DOMString(proposedValue);
  }

  bool InputType::IsInteractiveContent() const noexcept
  {
    return _type != Type::Hidden;
  }

  bool InputType::IsLabelable() const noexcept
  {
    return _type != Type::Hidden;
  }

  bool InputType::IsEnumeratable() const noexcept
  {
    return _type != Type::Image;
  }

  bool InputType::ShouldRespectHeightAndWidthAttributes() noexcept
  {
    return false;
  }

  bool InputType::SupportsPlaceholder() const noexcept
  {
    return false;
  }

  bool InputType::IsEmptyValue() const noexcept
  {
    return true;
  }

  bool InputType::SupportsReadOnly() const noexcept
  {
    return false;
  }

  bool InputType::MatchesIndeterminatePseudoClass() const noexcept
  {
    return false;
  }

  bool InputType::SupportsSelectionAPI() const noexcept
  {
    return false;
  }

  bool InputType::DirAutoUsesValue() const noexcept
  {
    return false;
  }

  uint32 InputType::Height() const noexcept
  {
    return 0;
  }

  uint32 InputType::Width() const noexcept
  {
    return 0;
  }

  dom::ExceptionOr<void> InputType::ApplyStep(int count, AnyStepHandling anyStepHandling,
                                              TextFieldEventBehavior eventBehavior) noexcept
  {
    // https://html.spec.whatwg.org/#dom-input-stepup

    StepRange stepRange(CreateStepRange(anyStepHandling));

    // 2. If the element has no allowed value step, then throw an InvalidStateError exception, and abort these
    // steps.
    if (!stepRange.HasStep())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    // 3. If the element has a minimum and a maximum and the minimum is greater than the maximum, then abort
    // these steps.
    if (stepRange.Minimum() > stepRange.Maximum())
    {
      return {};
    }

    // 4. If the element has a minimum and a maximum and there is no value greater than or equal to the
    // element's minimum and less than or equal to the element's maximum that, when subtracted from the step
    // base, is an integral multiple of the allowed value step, then abort these steps.
    Decimal alignedMaximum = stepRange.StepSnappedMaximum();
    if (!alignedMaximum.IsFinite())
    {
      return {};
    }

    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    const Decimal current = ParseToNumber(element->Value(), Decimal(0));
    Decimal base = stepRange.StepBase();
    Decimal step = stepRange.Step();

    Decimal newValue = current;
    auto stepString = element->GetAttribute(u8"step").value_or(dom::DOMString());
    bool isAnyStep = text::IsASCIICaselessEqual(dom::DOMStringView(stepString), dom::DOMStringView(u8"any"));
    if (!isAnyStep && stepRange.StepMismatch(current))
    {
      // Snap-to-step / clamping steps
      // If the current value is not matched to step value:
      // - The value should be the larger matched value nearest to 0 if count > 0
      //   e.g. <input type=number value=3 min=-100 step=3> -> 5
      // - The value should be the smaller matched value nearest to 0 if count < 0
      //   e.g. <input type=number value=3 min=-100 step=3> -> 2
      krys_debug_assert(!step.IsZero());
      if (count < 0)
      {
        newValue = base + ((newValue - base) / step).Floor() * step;
        ++count;
      }
      else if (count > 0)
      {
        newValue = base + ((newValue - base) / step).Ceil() * step;
        --count;
      }
    }

    newValue = newValue + stepRange.Step() * Decimal::FromDouble(count);

    if (!isAnyStep)
    {
      newValue = stepRange.AlignValueForStep(current, newValue);
    }

    // 8. If the element has a minimum, and value is less than that minimum, then set value to the smallest
    // value that, when subtracted from the step base, is an integral multiple of the allowed value step, and
    // that is more than or equal to minimum.
    if (newValue < stepRange.Minimum())
    {
      const Decimal alignedMinimum = base + ((stepRange.Minimum() - base) / step).Ceil() * step;
      krys_debug_assert(alignedMinimum >= stepRange.Minimum());
      newValue = alignedMinimum;
    }

    // 9. If the element has a maximum, and value is greater than that maximum, then set value to the largest
    // value that, when subtracted from the step base, is an integral multiple of the allowed value step, and
    // that is less than or equal to maximum.
    if (newValue > stepRange.Maximum())
    {
      newValue = alignedMaximum;
    }

    // 10. If either the method invoked was the stepDown() method and value is greater than
    // valueBeforeStepping, or the method invoked was the stepUp() method and value is less than
    // valueBeforeStepping, then return.
    if ((count < 0 && current < newValue) || (count > 0 && current > newValue))
    {
      return {};
    }

    // 11. Let value as string be the result of running the algorithm to convert a number to a string, as
    // defined for the input element's type attribute's current state, on value.
    // 12. Set the value of the element to value as string.
    Ref<InputType> protectedThis = ShareRef(*this);
    return SetValueAsDecimal(newValue, eventBehavior);
  }

  bool InputType::GetAllowedValueStep(Decimal *step) const noexcept
  {
    StepRange stepRange(CreateStepRange(AnyStepHandling::Reject));
    *step = stepRange.Step();
    return stepRange.HasStep();
  }

  StepRange InputType::CreateStepRange(AnyStepHandling) const noexcept
  {
    krys_debug_assert(false);
    return StepRange();
  }

  dom::ExceptionOr<void> InputType::StepUp(int n) noexcept
  {
    if (!IsSteppable())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError};
    }

    return ApplyStep(n, AnyStepHandling::Reject, TextFieldEventBehavior::DispatchNoEvent);
  }

  dom::DOMString InputType::ResultForDialogSubmit() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Value();
  }

  Decimal InputType::FindStepBase(const Decimal &defaultValue) const noexcept
  {
    auto &element = *Element();
    Decimal stepBase =
      ParseToNumber(element.GetAttribute(u8"min").value_or(dom::DOMString()), Decimal::NaN());
    if (!stepBase.IsFinite())
    {
      stepBase = ParseToNumber(element.GetAttribute(u8"value").value_or(dom::DOMString()), defaultValue);
    }
    return stepBase;
  }
}
