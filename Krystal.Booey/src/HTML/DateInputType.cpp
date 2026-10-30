#include "Krystal.Booey/HTML/DateInputType.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/HTML/DateTimeFieldsState.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    const StepRange::StepDescription DateInputTypeStepDescription {1, 0, 86'400'000,
                                                                   StepRange::ParsedStepValueShouldBeInteger};
  }

  StepRange DateInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    auto attribute = [&](dom::DOMStringView name)
    {
      return element.GetAttribute(name).value_or(dom::DOMString());
    };
    const Decimal stepBase = FindStepBase(Decimal(0));
    const Decimal minimum =
      ParseToNumber(attribute(u8"min"), Decimal::FromDouble(DateComponents::MinimumDate()));
    const Decimal maximum =
      ParseToNumber(attribute(u8"max"), Decimal::FromDouble(DateComponents::MaximumDate()));
    const Decimal step =
      StepRange::ParseStep(anyStepHandling, DateInputTypeStepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, RangeLimitations::Valid, minimum, maximum, step, DateInputTypeStepDescription);
  }

  Maybe<DateComponents> DateInputType::ParseToDateComponents(dom::DOMStringView source) const noexcept
  {
    return DateComponents::FromParsingDate(source);
  }

  Maybe<DateComponents> DateInputType::SetMillisecondToDateComponents(double value) const noexcept
  {
    return DateComponents::FromMillisecondsSinceEpochForDate(value);
  }

  bool DateInputType::IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept
  {
    return results.ContainsAll({DateTimeFormatValidationResults::HasYear,
                                DateTimeFormatValidationResults::HasMonth,
                                DateTimeFormatValidationResults::HasDay});
  }

  dom::DOMString DateInputType::FormatDateTimeFieldsState(const DateTimeFieldsState &state) const noexcept
  {
    if (!state.DayOfMonth || !state.Month || !state.Year)
    {
      return {};
    }

    return PadNumber(*state.Year, 4) + u8'-' + PadNumber(*state.Month, 2) + u8'-'
           + PadNumber(*state.DayOfMonth, 2);
  }

  void DateInputType::SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                                            const DateComponents &) const noexcept
  {
    layoutParameters.DateTimeFormat = layoutParameters.FormatLocale.DateFormat();
    layoutParameters.FallbackDateTimeFormat = u8"yyyy-MM-dd";
  }
}
