#include "Krystal.Booey/HTML/InputTypes/InputType.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementTextDirection.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/HTML/InputTypes/ButtonInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/CheckboxInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/ColorInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/DateInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/DateTimeLocalInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/EmailInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/FileInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/HiddenInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/ImageInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/InputTypeNames.hpp"
#include "Krystal.Booey/HTML/InputTypes/MonthInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/NumberInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/PasswordInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/RadioInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/RangeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/ResetInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/SearchInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/SubmitInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/TelephoneInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/TextInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/TimeInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/UrlInputType.hpp"
#include "Krystal.Booey/HTML/InputTypes/WeekInputType.hpp"
#include "Krystal.Booey/HTML/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/Clients/BooeyClient.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include <array>

namespace krys::boo::html
{
  namespace
  {
    using Type = InputType::Type;

    constexpr EnumSet<Type> TextTypes {Type::Email,     Type::Password, Type::Search,
                                       Type::Telephone, Type::Text,     Type::Url};

    constexpr EnumSet<Type> TextFieldTypes {Type::Email,     Type::Number, Type::Password, Type::Search,
                                            Type::Telephone, Type::Text,   Type::Url};

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
      DOMStringView Name;
      InputTypeFactoryFunction FactoryFunction;
    };

    constexpr std::array<InputTypeFactory, 22> InputTypeFactories {{
      {input_type_names::Button, &CreateInputType<ButtonInputType>},
      {input_type_names::Checkbox, &CreateInputType<CheckboxInputType>},
      {input_type_names::Color, &CreateInputType<ColorInputType>},
      {input_type_names::Date, &CreateInputType<DateInputType>},
      {input_type_names::DateTimeLocal, &CreateInputType<DateTimeLocalInputType>},
      {input_type_names::Email, &CreateInputType<EmailInputType>},
      {input_type_names::File, &CreateInputType<FileInputType>},
      {input_type_names::Hidden, &CreateInputType<HiddenInputType>},
      {input_type_names::Image, &CreateInputType<ImageInputType>},
      {input_type_names::Month, &CreateInputType<MonthInputType>},
      {input_type_names::Number, &CreateInputType<NumberInputType>},
      {input_type_names::Password, &CreateInputType<PasswordInputType>},
      {input_type_names::Radio, &CreateInputType<RadioInputType>},
      {input_type_names::Range, &CreateInputType<RangeInputType>},
      {input_type_names::Reset, &CreateInputType<ResetInputType>},
      {input_type_names::Search, &CreateInputType<SearchInputType>},
      {input_type_names::Submit, &CreateInputType<SubmitInputType>},
      {input_type_names::Telephone, &CreateInputType<TelephoneInputType>},
      {input_type_names::Text, &CreateInputType<TextInputType>},
      {input_type_names::Time, &CreateInputType<TimeInputType>},
      {input_type_names::Url, &CreateInputType<UrlInputType>},
      {input_type_names::Week, &CreateInputType<WeekInputType>},
    }};

    KRYS_NODISCARD const InputTypeFactory *FindFactory(DOMStringView typeName) noexcept
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

  RefPtr<InputType> InputType::CreateIfDifferent(HTMLInputElement &element, DOMStringView typeName,
                                                 InputType *currentInputType) noexcept
  {
    if (!typeName.empty())
    {
      auto currentTypeName = currentInputType ? currentInputType->FormControlType() : DOMStringView();
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
    return TextButtonTypes.contains(_type);
  }

  bool InputType::IsTextField() const noexcept
  {
    return TextFieldTypes.contains(_type);
  }

  bool InputType::IsTextType() const noexcept
  {
    return TextTypes.contains(_type);
  }

  bool InputType::IsCheckable() const noexcept
  {
    return CheckableTypes.contains(_type);
  }

  bool InputType::IsSteppable() const noexcept
  {
    return SteppableTypes.contains(_type);
  }

  bool InputType::SupportsValidation() const noexcept
  {
    return !NonValidatingTypes.contains(_type);
  }

  bool InputType::HasActivationBehavior() const noexcept
  {
    return ActivationBehaviorTypes.contains(_type);
  }

  bool InputType::IsValidValue(DOMStringView value) const noexcept
  {
    krys_debug_assert(CanSetStringValue());
    return !TypeMismatchFor(value) && !StepMismatch(value) && !RangeUnderflow(value) && !RangeOverflow(value)
           && !PatternMismatch(value) && !ValueMissing(value);
  }

  bool InputType::IsFormDataAppendable() const noexcept
  {
    krys_debug_assert(Element());
    // Only a named control appends an entry (an image button names its coordinates otherwise).
    return !Element()->Name().empty();
  }

  bool InputType::ReceiveDroppedFiles(const DragData &) noexcept
  {
    krys_debug_assert(false);
    return false;
  }

  DOMString InputType::DisplayString() const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  bool InputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    krys_debug_assert(Element());
    auto &input = *Element();
    formData.Append(input.Name(), input.Value());
    return true;
  }

  double InputType::ValueAsDate() const noexcept
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  ExceptionOr<void> InputType::SetValueAsDate(double) const noexcept
  {
    return Exception {ExceptionCode::InvalidStateError};
  }

  double InputType::ValueAsDouble() const noexcept
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  ExceptionOr<void> InputType::SetValueAsDouble(double doubleValue,
                                                TextFieldEventBehavior eventBehavior) const noexcept
  {
    return SetValueAsDecimal(Decimal::FromDouble(doubleValue), eventBehavior);
  }

  ExceptionOr<void> InputType::SetValueAsDecimal(const Decimal &, TextFieldEventBehavior) const noexcept
  {
    return Exception {ExceptionCode::InvalidStateError};
  }

  bool InputType::SupportsRequired() const noexcept
  {
    // `required` applies to the types that take part in validation, unless a type says otherwise.
    return SupportsValidation();
  }

  bool InputType::RangeUnderflow(DOMStringView value) const noexcept
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

  bool InputType::RangeOverflow(DOMStringView value) const noexcept
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

  bool InputType::IsInvalid(DOMStringView value) const noexcept
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

  bool InputType::IsInRange(DOMStringView value) const noexcept
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

  bool InputType::IsOutOfRange(DOMStringView value) const noexcept
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

  bool InputType::StepMismatch(DOMStringView value) const noexcept
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

  DOMString InputType::BadInputText() const noexcept
  {
    krys_debug_assert(false);
    return ValidationMessageTypeMismatchText();
  }

  DOMString InputType::TypeMismatchText() const noexcept
  {
    return ValidationMessageTypeMismatchText();
  }

  DOMString InputType::ValueMissingText() const noexcept
  {
    return ValidationMessageValueMissingText();
  }

  DOMString InputType::ValidationMessage() const noexcept
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
        element.GetAttribute(u8"title").value_or(DOMString()));
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
                          : DOMString();
      return ValidationMessageStepMismatchText(Serialize(stepRange.StepBase()), stepString);
    }

    return {};
  }

  InputType::ShouldCallBaseEventHandler InputType::HandleKeydownEvent(KeyboardEvent &) noexcept
  {
    return ShouldCallBaseEventHandler::Yes;
  }

  void InputType::HandleKeypressEvent(KeyboardEvent &) noexcept
  {
  }

  void InputType::HandleKeyupEvent(KeyboardEvent &) noexcept
  {
  }

  bool InputType::ShouldSubmitImplicitly(Event &event) noexcept
  {
    auto *keyboardEvent = DynamicDowncast<KeyboardEvent>(event);
    return keyboardEvent && event.Type() == event_names::KeyPress && keyboardEvent->CharCode() == '\r';
  }

  Decimal InputType::ParseToNumber(DOMStringView, const Decimal &defaultValue) const noexcept
  {
    krys_debug_assert(false);
    return defaultValue;
  }

  Decimal InputType::ParseToNumberOrNaN(DOMStringView string) const noexcept
  {
    return ParseToNumber(string, Decimal::NaN());
  }

  DOMString InputType::Serialize(const Decimal &) const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  DateComponentsType InputType::DateType() const noexcept
  {
    return DateComponentsType::Invalid;
  }

  void InputType::DispatchSimulatedClickIfActive(KeyboardEvent &event) const noexcept
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
    // Stepping from the user's controls (arrow keys, spin buttons) differs from stepUp() and stepDown():
    // - A value that isn't a number starts from the type's default (0, or now for the date and time types),
    //   moved back into the range if a step from it would leave it.
    // - Below the range, stepping up goes to the minimum; above it, stepping down goes to the maximum. The
    //   other direction does nothing.
    // - A value off the step goes to the step next to it in the direction of the stepping.
    // A negative step reverses the direction.

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

    // Without an allowed value step (step=any), there's nothing to step by.
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

  Maybe<DOMString> InputType::FallbackValue() const noexcept
  {
    return std::nullopt;
  }

  Maybe<DOMString> InputType::DefaultValue() const noexcept
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

  void InputType::SetValue(DOMStringView sanitizedValue, bool valueChanged,
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

    auto oldDirection = dom::ComputeTextDirectionIfDirIsAuto(*element);

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (wasInRange != inRange)
    {
      styleInvalidation.emplace(*element, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                            {css::SelectorPseudoClass::InRange, inRange},
                                            {css::SelectorPseudoClass::OutOfRange, !inRange},
                                          });
    }

    element->SetValueInternal(sanitizedValue, eventBehavior);

    if (oldDirection.value_or(text::TextDirection::LTR)
        != dom::ComputeTextDirectionIfDirIsAuto(*element).value_or(text::TextDirection::LTR))
    {
      element->InvalidateStyleInternal();
    }

    switch (eventBehavior)
    {
      case TextFieldEventBehavior::DispatchChangeEvent: element->DispatchFormControlChangeEvent(); break;
      case TextFieldEventBehavior::DispatchInputAndChangeEvent:
        element->DispatchFormControlInputEvent();
        element->DispatchFormControlChangeEvent();
        break;
      case TextFieldEventBehavior::DispatchNoEvent: break;
    }

    if (auto *cache = element->NodeDocument().ExistingAXObjectCache())
    {
      cache->ValueChanged(*element);
    }
  }

  DOMString InputType::VisibleValue() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Value();
  }

  DOMString InputType::LocalizeValue(DOMStringView proposedValue) const noexcept
  {
    return DOMString(proposedValue);
  }

  BooeyClient *InputType::GetBooeyClient() const noexcept
  {
    if (!Element())
    {
      return nullptr;
    }
    auto *frame = Element()->NodeDocument().Frame();
    return frame ? &frame->GetBooeyClient() : nullptr;
  }

  bool InputType::HasTransientActivation() const noexcept
  {
    auto *window = _element ? _element->NodeDocument().Window() : nullptr;
    return window && window->HasTransientActivation();
  }

  DOMString InputType::SanitizeValue(DOMStringView proposedValue) const noexcept
  {
    return DOMString(proposedValue);
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

  bool InputType::ShouldUseInputMethod() const noexcept
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

  ExceptionOr<void> InputType::ApplyStep(int count, AnyStepHandling anyStepHandling,
                                         TextFieldEventBehavior eventBehavior) noexcept
  {
    // https://html.spec.whatwg.org/#dom-input-stepup

    StepRange stepRange(CreateStepRange(anyStepHandling));

    // 2. If the element has no allowed value step, then throw an InvalidStateError exception, and abort these
    // steps.
    if (!stepRange.HasStep())
    {
      return Exception {ExceptionCode::InvalidStateError};
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
    auto stepString = element->GetAttribute(u8"step").value_or(DOMString());
    bool isAnyStep = text::IsASCIICaselessEqual(DOMStringView(stepString), u8"any");
    if (!isAnyStep && stepRange.StepMismatch(current))
    {
      // A value off the step goes to the step next to it in the direction of the stepping, and that counts as
      // one of the steps.
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
    //     defined for the input element's type attribute's current state, on value.
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

  ExceptionOr<void> InputType::StepUp(int n) noexcept
  {
    if (!IsSteppable())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }

    return ApplyStep(n, AnyStepHandling::Reject, TextFieldEventBehavior::DispatchNoEvent);
  }

  Maybe<DOMString> InputType::ResultForDialogSubmit() const noexcept
  {
    krys_debug_assert(Element());
    return Element()->Value();
  }

  Decimal InputType::FindStepBase(const Decimal &defaultValue) const noexcept
  {
    auto &element = *Element();
    Decimal stepBase = ParseToNumber(element.GetAttribute(u8"min").value_or(DOMString()), Decimal::NaN());
    if (!stepBase.IsFinite())
    {
      stepBase = ParseToNumber(element.GetAttribute(u8"value").value_or(DOMString()), defaultValue);
    }
    return stepBase;
  }
}
