#include "Krystal.Booey/HTML/TimeInputType.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/HTML/DateTimeFieldsState.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    const StepRange::StepDescription TimeInputTypeStepDescription {60, 0, 1'000,
                                                                   StepRange::ScaledStepValueShouldBeInteger};
  }

  StepRange TimeInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto attribute = [&](dom::DOMStringView name)
    {
      return element.GetAttribute(name).value_or(dom::DOMString());
    };
    const Decimal stepBase = FindStepBase(Decimal(0));
    const Decimal minimum =
      ParseToNumber(attribute(u8"min"), Decimal::FromDouble(DateComponents::MinimumTime()));
    const Decimal maximum =
      ParseToNumber(attribute(u8"max"), Decimal::FromDouble(DateComponents::MaximumTime()));
    const Decimal step =
      StepRange::ParseStep(anyStepHandling, TimeInputTypeStepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, RangeLimitations::Valid, minimum, maximum, step, TimeInputTypeStepDescription,
                     StepRange::IsReversible::Yes);
  }

  Maybe<DateComponents> TimeInputType::ParseToDateComponents(dom::DOMStringView source) const noexcept
  {
    return DateComponents::FromParsingTime(source);
  }

  Maybe<DateComponents> TimeInputType::SetMillisecondToDateComponents(double value) const noexcept
  {
    return DateComponents::FromMillisecondsSinceMidnight(value);
  }

  Decimal TimeInputType::DefaultValueForStepUp() const noexcept
  {
    auto date = DateComponents::FromMillisecondsSinceMidnight(LocalTimeNow());
    if (!date)
    {
      return {};
    }

    double milliseconds = date->MillisecondsSinceEpoch();
    krys_debug_assert(std::isfinite(milliseconds));
    return Decimal::FromDouble(milliseconds);
  }

  void TimeInputType::HandleActivationBehavior(dom::Event &) noexcept
  {
  }

  void TimeInputType::ShowPicker() noexcept
  {
  }

  bool TimeInputType::IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept
  {
    return results.ContainsAll({DateTimeFormatValidationResults::HasHour,
                                DateTimeFormatValidationResults::HasMinute,
                                DateTimeFormatValidationResults::HasMeridiem});
  }

  dom::DOMString TimeInputType::FormatDateTimeFieldsState(const DateTimeFieldsState &state) const noexcept
  {
    if (!state.Hour || !state.Minute || !state.Meridiem)
    {
      return {};
    }

    auto hourMinuteString = PadNumber(*state.Hour23(), 2) + u8':' + PadNumber(*state.Minute, 2);

    if (state.Millisecond)
    {
      return hourMinuteString + u8':' + PadNumber(state.Second.value_or(0), 2) + u8'.'
             + PadNumber(*state.Millisecond, 3);
    }

    if (state.Second)
    {
      return hourMinuteString + u8':' + PadNumber(*state.Second, 2);
    }

    return hourMinuteString;
  }

  void TimeInputType::SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                                            const DateComponents &date) const noexcept
  {
    layoutParameters.ShouldHaveMillisecondField = ShouldHaveMillisecondField(date);

    if (layoutParameters.ShouldHaveMillisecondField || ShouldHaveSecondField(date))
    {
      layoutParameters.DateTimeFormat = layoutParameters.FormatLocale.TimeFormat();
      layoutParameters.FallbackDateTimeFormat = u8"HH:mm:ss";
    }
    else
    {
      layoutParameters.DateTimeFormat = layoutParameters.FormatLocale.ShortTimeFormat();
      layoutParameters.FallbackDateTimeFormat = u8"HH:mm";
    }
  }
}
