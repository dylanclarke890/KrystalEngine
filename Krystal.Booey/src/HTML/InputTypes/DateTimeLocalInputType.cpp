/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/InputTypes/DateTimeLocalInputType.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/InputTypes/DateTimeFieldsState.hpp"
#include "Krystal.Booey/Platform/Locale/PlatformLocale.hpp"
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

  bool DateTimeLocalInputType::IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept
  {
    return results.ContainsAll(
      {DateTimeFormatValidationResults::HasYear, DateTimeFormatValidationResults::HasMonth,
       DateTimeFormatValidationResults::HasDay, DateTimeFormatValidationResults::HasHour,
       DateTimeFormatValidationResults::HasMinute, DateTimeFormatValidationResults::HasMeridiem});
  }

  dom::DOMString
    DateTimeLocalInputType::FormatDateTimeFieldsState(const DateTimeFieldsState &state) const noexcept
  {
    if (!state.Year || !state.Month || !state.DayOfMonth || !state.Hour || !state.Minute || !state.Meridiem)
    {
      return {};
    }

    auto dateString = PadNumber(*state.Year, 4) + u8'-' + PadNumber(*state.Month, 2) + u8'-'
                      + PadNumber(*state.DayOfMonth, 2);
    auto hourMinuteString = PadNumber(*state.Hour23(), 2) + u8':' + PadNumber(*state.Minute, 2);

    if (state.Millisecond)
    {
      return dateString + u8'T' + hourMinuteString + u8':' + PadNumber(state.Second.value_or(0), 2) + u8'.'
             + PadNumber(*state.Millisecond, 3);
    }

    if (state.Second)
    {
      return dateString + u8'T' + hourMinuteString + u8':' + PadNumber(*state.Second, 2);
    }

    return dateString + u8'T' + hourMinuteString;
  }

  void DateTimeLocalInputType::SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                                                     const DateComponents &date) const noexcept
  {
    layoutParameters.ShouldHaveMillisecondField = ShouldHaveMillisecondField(date);

    if (layoutParameters.ShouldHaveMillisecondField || ShouldHaveSecondField(date))
    {
      layoutParameters.DateTimeFormat = layoutParameters.FormatLocale.DateTimeFormatWithSeconds();
      layoutParameters.FallbackDateTimeFormat = u8"yyyy-MM-dd'T'HH:mm:ss";
    }
    else
    {
      layoutParameters.DateTimeFormat = layoutParameters.FormatLocale.DateTimeFormatWithoutSeconds();
      layoutParameters.FallbackDateTimeFormat = u8"yyyy-MM-dd'T'HH:mm";
    }
  }
}
