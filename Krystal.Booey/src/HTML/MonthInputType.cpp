#include "Krystal.Booey/HTML/MonthInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    const StepRange::StepDescription MonthInputTypeStepDescription {
      1, 0, 1, StepRange::ParsedStepValueShouldBeInteger};
  }

  StepRange MonthInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto attribute = [&](dom::DOMStringView name)
    {
      return element.GetAttribute(name).value_or(dom::DOMString());
    };
    const Decimal stepBase = FindStepBase(Decimal(0));
    const Decimal minimum =
      ParseToNumber(attribute(u8"min"), Decimal::FromDouble(DateComponents::MinimumMonth()));
    const Decimal maximum =
      ParseToNumber(attribute(u8"max"), Decimal::FromDouble(DateComponents::MaximumMonth()));
    const Decimal step =
      StepRange::ParseStep(anyStepHandling, MonthInputTypeStepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, RangeLimitations::Valid, minimum, maximum, step,
                     MonthInputTypeStepDescription);
  }

  Maybe<DateComponents> MonthInputType::ParseToDateComponents(dom::DOMStringView source) const noexcept
  {
    return DateComponents::FromParsingMonth(source);
  }

  Maybe<DateComponents> MonthInputType::SetMillisecondToDateComponents(double value) const noexcept
  {
    return DateComponents::FromMonthsSinceEpoch(value);
  }

  double MonthInputType::ValueAsDate() const noexcept
  {
    krys_debug_assert(Element());
    auto date = ParseToDateComponents(Element()->Value());
    if (!date)
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    double msec = date->MillisecondsSinceEpoch();
    krys_debug_assert(std::isfinite(msec));
    return msec;
  }

  dom::DOMString MonthInputType::SerializeWithMilliseconds(double value) const noexcept
  {
    auto date = DateComponents::FromMillisecondsSinceEpochForMonth(value);
    if (!date)
    {
      return {};
    }
    return SerializeWithComponents(*date);
  }

  Decimal MonthInputType::DefaultValueForStepUp() const noexcept
  {
    auto date = DateComponents::FromMillisecondsSinceEpochForMonth(LocalTimeNow());
    if (!date)
    {
      return {};
    }

    double months = date->MonthsSinceEpoch();
    krys_debug_assert(std::isfinite(months));
    return Decimal::FromDouble(months);
  }

  Decimal MonthInputType::ParseToNumber(dom::DOMStringView src, const Decimal &defaultValue) const noexcept
  {
    auto date = ParseToDateComponents(src);
    if (!date)
    {
      return defaultValue;
    }
    double months = date->MonthsSinceEpoch();
    krys_debug_assert(std::isfinite(months));
    return Decimal::FromDouble(months);
  }

}
