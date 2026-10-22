#include "Krystal.Booey/HTML/WeekInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    const StepRange::StepDescription WeekInputTypeStepDescription {1, -259'200'000, 604'800'000,
                                                                   StepRange::ParsedStepValueShouldBeInteger};
  }

  StepRange WeekInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto attribute = [&](dom::DOMStringView name)
    {
      return element.GetAttribute(name).value_or(dom::DOMString());
    };
    const Decimal stepBase = FindStepBase(Decimal(-259'200'000));
    const Decimal minimum =
      ParseToNumber(attribute(u8"min"), Decimal::FromDouble(DateComponents::MinimumWeek()));
    const Decimal maximum =
      ParseToNumber(attribute(u8"max"), Decimal::FromDouble(DateComponents::MaximumWeek()));
    const Decimal step =
      StepRange::ParseStep(anyStepHandling, WeekInputTypeStepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, RangeLimitations::Valid, minimum, maximum, step, WeekInputTypeStepDescription);
  }

  Maybe<DateComponents> WeekInputType::ParseToDateComponents(dom::DOMStringView source) const noexcept
  {
    return DateComponents::FromParsingWeek(source);
  }

  Maybe<DateComponents> WeekInputType::SetMillisecondToDateComponents(double value) const noexcept
  {
    return DateComponents::FromMillisecondsSinceEpochForWeek(value);
  }

}
