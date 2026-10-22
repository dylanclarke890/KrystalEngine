#include "Krystal.Booey/HTML/TimeInputType.hpp"
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

}
