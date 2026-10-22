#include "Krystal.Booey/HTML/NumberInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    constexpr int NumberDefaultStep = 1;
    constexpr int NumberDefaultStepBase = 0;
    constexpr int NumberStepScaleFactor = 1;
  }

  double NumberInputType::ValueAsDouble() const noexcept
  {
    krys_debug_assert(Element());
    return ParseToDoubleForNumberType(Element()->Value());
  }

  dom::ExceptionOr<void>
    NumberInputType::SetValueAsDouble(double newValue, TextFieldEventBehavior eventBehavior) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(SerializeForNumberType(newValue), eventBehavior);
    return {};
  }

  dom::ExceptionOr<void>
    NumberInputType::SetValueAsDecimal(const Decimal &newValue,
                                       TextFieldEventBehavior eventBehavior) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(SerializeForNumberType(newValue), eventBehavior);
    return {};
  }

  bool NumberInputType::TypeMismatchFor(dom::DOMStringView value) const noexcept
  {
    return !value.empty() && !std::isfinite(ParseToDoubleForNumberType(value));
  }

  bool NumberInputType::TypeMismatch() const noexcept
  {
    krys_debug_assert(Element());
    krys_debug_assert(!TypeMismatchFor(Element()->Value()));
    return false;
  }

  StepRange NumberInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    static const StepRange::StepDescription stepDescription {NumberDefaultStep, NumberDefaultStepBase,
                                                             NumberStepScaleFactor};

    krys_debug_assert(Element());
    const Decimal stepBase = FindStepBase(Decimal(NumberDefaultStepBase));

    const Decimal doubleMax = Decimal::DoubleMax();
    auto &element = *Element();

    RangeLimitations rangeLimitations = RangeLimitations::Invalid;
    auto extractBound = [&](dom::DOMStringView attributeName, const Decimal &defaultValue) -> Decimal
    {
      Decimal valueFromAttribute =
        ParseToNumberOrNaN(element.GetAttribute(attributeName).value_or(dom::DOMString()));
      if (valueFromAttribute.IsFinite())
      {
        rangeLimitations = RangeLimitations::Valid;
        return valueFromAttribute;
      }
      return defaultValue;
    };
    Decimal minimum = extractBound(u8"min", -doubleMax);
    Decimal maximum = extractBound(u8"max", doubleMax);

    const Decimal step =
      StepRange::ParseStep(anyStepHandling, stepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, rangeLimitations, minimum, maximum, step, stepDescription);
  }

  Decimal NumberInputType::ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept
  {
    return ParseToDecimalForNumberType(string, defaultValue);
  }

  dom::DOMString NumberInputType::Serialize(const Decimal &value) const noexcept
  {
    if (!value.IsFinite())
    {
      return {};
    }
    return SerializeForNumberType(value);
  }

  dom::DOMString NumberInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    if (proposedValue.empty())
    {
      return {};
    }
    if (std::isfinite(ParseToDoubleForNumberType(proposedValue)))
    {
      return dom::DOMString(proposedValue);
    }
    return {};
  }

  bool NumberInputType::HasBadInput() const noexcept
  {
    // FIXME: What the user typed that isn't a number (the inner text value), with text fields.
    return false;
  }

  dom::DOMString NumberInputType::BadInputText() const noexcept
  {
    return ValidationMessageBadInputForNumberText();
  }

  bool NumberInputType::SupportsPlaceholder() const noexcept
  {
    return true;
  }
}
