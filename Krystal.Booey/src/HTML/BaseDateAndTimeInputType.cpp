#include "Krystal.Booey/HTML/BaseDateAndTimeInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <chrono>
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    constexpr int MsecPerMinute = 60 * 1'000;
    constexpr int MsecPerSecond = 1'000;
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

  double BaseDateAndTimeInputType::LocalTimeNow() noexcept
  {
    auto now = std::chrono::system_clock::now();
    double ms = static_cast<double>(
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    auto offset = std::chrono::current_zone()->get_info(now).offset;
    return ms + static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(offset).count());
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
}
