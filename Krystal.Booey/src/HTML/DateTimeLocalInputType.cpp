#include "Krystal.Booey/HTML/DateTimeLocalInputType.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    const StepRange::StepDescription DateTimeLocalInputTypeStepDescription {
      60, 0, 1'000, StepRange::ScaledStepValueShouldBeInteger};
  }

  StepRange DateTimeLocalInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto attribute = [&](dom::DOMStringView name)
    {
      return element.GetAttribute(name).value_or(dom::DOMString());
    };
    const Decimal stepBase = FindStepBase(Decimal(0));
    const Decimal minimum =
      ParseToNumber(attribute(u8"min"), Decimal::FromDouble(DateComponents::MinimumDateTime()));
    const Decimal maximum =
      ParseToNumber(attribute(u8"max"), Decimal::FromDouble(DateComponents::MaximumDateTime()));
    const Decimal step = StepRange::ParseStep(anyStepHandling, DateTimeLocalInputTypeStepDescription,
                                              element.GetAttribute(u8"step"));
    return StepRange(stepBase, RangeLimitations::Valid, minimum, maximum, step,
                     DateTimeLocalInputTypeStepDescription);
  }

  Maybe<DateComponents>
    DateTimeLocalInputType::ParseToDateComponents(dom::DOMStringView source) const noexcept
  {
    return DateComponents::FromParsingDateTimeLocal(source);
  }

  Maybe<DateComponents> DateTimeLocalInputType::SetMillisecondToDateComponents(double value) const noexcept
  {
    return DateComponents::FromMillisecondsSinceEpochForDateTimeLocal(value);
  }

  double DateTimeLocalInputType::ValueAsDate() const noexcept
  {
    // valueAsDate doesn't work for the datetime-local type according to the standard.
    return std::numeric_limits<double>::quiet_NaN();
  }

  dom::ExceptionOr<void> DateTimeLocalInputType::SetValueAsDate(double value) const noexcept
  {
    // valueAsDate doesn't work for the datetime-local type according to the standard.
    return InputType::SetValueAsDate(value);
  }

  dom::DOMString DateTimeLocalInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    if (proposedValue.empty())
    {
      return {};
    }

    if (auto components = DateComponents::FromParsingDateTimeLocal(proposedValue))
    {
      return components->ToString();
    }

    return {};
  }

}
