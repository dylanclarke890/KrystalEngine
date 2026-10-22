#include "Krystal.Booey/HTML/RangeInputType.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    constexpr int RangeDefaultMinimum = 0;
    constexpr int RangeDefaultMaximum = 100;
    constexpr int RangeDefaultStep = 1;
    constexpr int RangeDefaultStepBase = 0;
    constexpr int RangeStepScaleFactor = 1;
    const StepRange::StepDescription RangeStepDescription {RangeDefaultStep, RangeDefaultStepBase,
                                                           RangeStepScaleFactor};

    KRYS_NODISCARD Decimal EnsureMaximum(const Decimal &proposedValue, const Decimal &minimum) noexcept
    {
      return proposedValue >= minimum ? proposedValue : minimum;
    }
  }

  double RangeInputType::ValueAsDouble() const noexcept
  {
    krys_debug_assert(Element());
    return ParseToDoubleForNumberType(Element()->Value());
  }

  dom::ExceptionOr<void>
    RangeInputType::SetValueAsDecimal(const Decimal &newValue,
                                      TextFieldEventBehavior eventBehavior) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(Serialize(newValue), eventBehavior);
    return {};
  }

  bool RangeInputType::TypeMismatchFor(dom::DOMStringView value) const noexcept
  {
    return !value.empty() && !std::isfinite(ParseToDoubleForNumberType(value));
  }

  bool RangeInputType::SupportsRequired() const noexcept
  {
    return false;
  }

  StepRange RangeInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto attribute = [&](dom::DOMStringView name)
    {
      return element.GetAttribute(name).value_or(dom::DOMString());
    };
    const Decimal stepBase = FindStepBase(Decimal(RangeDefaultStepBase));
    const Decimal minimum = ParseToNumber(attribute(u8"min"), Decimal(RangeDefaultMinimum));
    const Decimal maximum =
      EnsureMaximum(ParseToNumber(attribute(u8"max"), Decimal(RangeDefaultMaximum)), minimum);

    const Decimal step =
      StepRange::ParseStep(anyStepHandling, RangeStepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, RangeLimitations::Valid, minimum, maximum, step, RangeStepDescription);
  }

  InputType::ShouldCallBaseEventHandler RangeInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    krys_debug_assert(Element());
    Ref<HTMLInputElement> element = ShareRef(*Element());
    if (element->IsDisabledFormControl())
    {
      return ShouldCallBaseEventHandler::Yes;
    }

    const auto &key = event.KeyIdentifier();

    const Decimal current = ParseToNumberOrNaN(element->Value());
    krys_debug_assert(current.IsFinite());

    StepRange stepRange(CreateStepRange(AnyStepHandling::Reject));

    // FIXME: We can't use stepUp() for the step value "any". So, we increase or decrease the value by 1/100
    // of the value range. Is it reasonable?
    bool isAnyStep = text::IsASCIICaselessEqual(
      dom::DOMStringView(element->GetAttribute(u8"step").value_or(dom::DOMString())),
      dom::DOMStringView(u8"any"));
    const Decimal step =
      isAnyStep ? (stepRange.Maximum() - stepRange.Minimum()) / Decimal(100) : stepRange.Step();
    const Decimal bigStep = std::max((stepRange.Maximum() - stepRange.Minimum()) / Decimal(10), step);

    // FIXME: Vertical sliders (StyleAppearance::SliderVertical), with the slider's renderer.
    bool isVertical = false;

    Decimal newValue;
    if (key == u8"Up")
    {
      newValue = current + step;
    }
    else if (key == u8"Down")
    {
      newValue = current - step;
    }
    else if (key == u8"Left")
    {
      newValue = isVertical ? current + step : current - step;
    }
    else if (key == u8"Right")
    {
      newValue = isVertical ? current - step : current + step;
    }
    else if (key == u8"PageUp")
    {
      newValue = current + bigStep;
    }
    else if (key == u8"PageDown")
    {
      newValue = current - bigStep;
    }
    else if (key == u8"Home")
    {
      newValue = isVertical ? stepRange.Maximum() : stepRange.Minimum();
    }
    else if (key == u8"End")
    {
      newValue = isVertical ? stepRange.Minimum() : stepRange.Maximum();
    }
    else
    {
      return ShouldCallBaseEventHandler::Yes; // Did not match any key binding.
    }

    newValue = stepRange.ClampValue(newValue);

    if (newValue != current)
    {
      (void)SetValueAsDecimal(newValue, TextFieldEventBehavior::DispatchInputAndChangeEvent);
    }

    event.SetDefaultHandled();
    return ShouldCallBaseEventHandler::Yes;
  }

  Decimal RangeInputType::ParseToNumber(dom::DOMStringView string, const Decimal &defaultValue) const noexcept
  {
    return ParseToDecimalForNumberType(string, defaultValue);
  }

  dom::DOMString RangeInputType::Serialize(const Decimal &value) const noexcept
  {
    if (!value.IsFinite())
    {
      return {};
    }
    return SerializeForNumberType(value);
  }

  void RangeInputType::AttributeChanged(dom::DOMStringAtom localName) noexcept
  {
    if (localName == u8"max" || localName == u8"min" || localName == u8"value")
    {
      // Sanitize the value.
      if (auto *element = Element())
      {
        if (element->HasDirtyValue())
        {
          (void)element->SetValue(element->Value());
        }
      }
      // FIXME: Moving the slider's thumb, with the slider's shadow tree.
    }
    InputType::AttributeChanged(localName);
  }

  void RangeInputType::SetValue(dom::DOMStringView value, bool valueChanged,
                                TextFieldEventBehavior eventBehavior,
                                TextControlSetValueSelection selection) noexcept
  {
    InputType::SetValue(value, valueChanged, eventBehavior, selection);

    if (!valueChanged)
    {
      return;
    }

    if (eventBehavior == TextFieldEventBehavior::DispatchNoEvent)
    {
      krys_debug_assert(Element());
      Element()->SetTextAsOfLastFormControlChangeEvent(dom::DOMString(value));
    }

    // FIXME: Moving the slider's thumb, with the slider's shadow tree.
  }

  Maybe<dom::DOMString> RangeInputType::FallbackValue() const noexcept
  {
    return SerializeForNumberType(CreateStepRange(AnyStepHandling::Reject).DefaultValue());
  }

  dom::DOMString RangeInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    StepRange stepRange(CreateStepRange(AnyStepHandling::Reject));
    const Decimal proposedNumericValue = ParseToNumber(proposedValue, stepRange.DefaultValue());
    return SerializeForNumberType(stepRange.ClampValue(proposedNumericValue));
  }
}
