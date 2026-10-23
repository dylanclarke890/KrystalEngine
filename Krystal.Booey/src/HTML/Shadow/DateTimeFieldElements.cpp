#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElements.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/HTML/DateTimeFieldsState.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"

namespace krys::boo::html
{
#pragma region DateTimeDayFieldElement

  DateTimeDayFieldElement::DateTimeDayFieldElement(dom::Document &document,
                                                   DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {1, 31},
                                    fieldOwner.PlaceholderDate().MonthDay())
  {
  }

  Ref<DateTimeDayFieldElement>
    DateTimeDayFieldElement::Create(dom::Document &document,
                                    DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeDayFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditDayField());
    return element;
  }

  void DateTimeDayFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.DayOfMonth = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.DayOfMonth = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeDayFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.MonthDay());
  }

#pragma endregion

#pragma region DateTimeHourFieldElement

  DateTimeHourFieldElement::DateTimeHourFieldElement(dom::Document &document,
                                                     DateTimeFieldElementFieldOwner &fieldOwner, int minimum,
                                                     int maximum) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {minimum, maximum}, (maximum >= 12) ? 12 : 11)
  {
  }

  Ref<DateTimeHourFieldElement> DateTimeHourFieldElement::Create(dom::Document &document,
                                                                 DateTimeFieldElementFieldOwner &fieldOwner,
                                                                 int minimum, int maximum) noexcept
  {
    auto element = CreateRef<DateTimeHourFieldElement>(document, fieldOwner, minimum, maximum);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditHourField());
    return element;
  }

  void DateTimeHourFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (!HasValue() && placeholderIfNoValue == DateTimePlaceholderIfNoValue::No)
    {
      return;
    }

    int value = HasValue() ? ValueAsInteger() : PlaceholderValueAsInteger();
    auto twelveHour = [](int hour)
    {
      return static_cast<uint32>((hour % 12) ? (hour % 12) : 12);
    };

    switch (Maximum())
    {
      case 11: state.Hour = static_cast<uint32>(value ? value : 12); return;
      case 12: state.Hour = static_cast<uint32>(value); return;
      case 23:
        state.Hour = twelveHour(value);
        state.Meridiem = value >= 12 ? DateTimeMeridiem::PM : DateTimeMeridiem::AM;
        return;
      case 24:
        if (value == 24)
        {
          state.Hour = 12;
          state.Meridiem = DateTimeMeridiem::AM;
          return;
        }
        state.Hour = twelveHour(value);
        state.Meridiem = value >= 12 ? DateTimeMeridiem::PM : DateTimeMeridiem::AM;
        return;
    }
  }

  void DateTimeHourFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    int hour = date.Hour();

    switch (Maximum())
    {
      case 11: SetValueAsInteger(hour % 12); return;
      case 12: SetValueAsInteger((hour % 12) ? (hour % 12) : 12); return;
      case 23: SetValueAsInteger(hour); return;
      case 24: SetValueAsInteger(hour + 1); return;
    }
  }

#pragma endregion

#pragma region DateTimeMeridiemFieldElement

  DateTimeMeridiemFieldElement::DateTimeMeridiemFieldElement(dom::Document &document,
                                                             DateTimeFieldElementFieldOwner &fieldOwner,
                                                             const List<utf8_string> &labels) noexcept
      : DateTimeSymbolicFieldElement(document, fieldOwner, labels, static_cast<int>(labels.size()) - 1)
  {
  }

  Ref<DateTimeMeridiemFieldElement>
    DateTimeMeridiemFieldElement::Create(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
                                         const List<utf8_string> &labels) noexcept
  {
    auto element = CreateRef<DateTimeMeridiemFieldElement>(document, fieldOwner, labels);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditMeridiemField());
    return element;
  }

  void DateTimeMeridiemFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Meridiem = ValueAsInteger() ? DateTimeMeridiem::PM : DateTimeMeridiem::AM;
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Meridiem = PlaceholderValueAsInteger() ? DateTimeMeridiem::PM : DateTimeMeridiem::AM;
    }
  }

  void DateTimeMeridiemFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.Hour() >= 12 ? 1 : 0);
  }

#pragma endregion

#pragma region DateTimeMillisecondFieldElement

  DateTimeMillisecondFieldElement::DateTimeMillisecondFieldElement(
    dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {0, 999}, 0)
  {
  }

  Ref<DateTimeMillisecondFieldElement>
    DateTimeMillisecondFieldElement::Create(dom::Document &document,
                                            DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeMillisecondFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditMillisecondField());
    return element;
  }

  void DateTimeMillisecondFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Millisecond = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Millisecond = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeMillisecondFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.Millisecond());
  }

#pragma endregion

#pragma region DateTimeMinuteFieldElement

  DateTimeMinuteFieldElement::DateTimeMinuteFieldElement(dom::Document &document,
                                                         DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {0, 59}, 30)
  {
  }

  Ref<DateTimeMinuteFieldElement>
    DateTimeMinuteFieldElement::Create(dom::Document &document,
                                       DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeMinuteFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditMinuteField());
    return element;
  }

  void DateTimeMinuteFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Minute = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Minute = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeMinuteFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.Minute());
  }

#pragma endregion

#pragma region DateTimeMonthFieldElement

  DateTimeMonthFieldElement::DateTimeMonthFieldElement(dom::Document &document,
                                                       DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {1, 12},
                                    fieldOwner.PlaceholderDate().Month() + 1)
  {
  }

  Ref<DateTimeMonthFieldElement>
    DateTimeMonthFieldElement::Create(dom::Document &document,
                                      DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeMonthFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditMonthField());
    return element;
  }

  void DateTimeMonthFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Month = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Month = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeMonthFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    // DateComponents represents months from 0 (January) to 11 (December).
    SetValueAsInteger(date.Month() + 1);
  }

#pragma endregion

#pragma region DateTimeSecondFieldElement

  DateTimeSecondFieldElement::DateTimeSecondFieldElement(dom::Document &document,
                                                         DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {0, 59}, 0)
  {
  }

  Ref<DateTimeSecondFieldElement>
    DateTimeSecondFieldElement::Create(dom::Document &document,
                                       DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeSecondFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditSecondField());
    return element;
  }

  void DateTimeSecondFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Second = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Second = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeSecondFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.Second());
  }

#pragma endregion

#pragma region DateTimeSymbolicMonthFieldElement

  DateTimeSymbolicMonthFieldElement::DateTimeSymbolicMonthFieldElement(
    dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
    const List<utf8_string> &labels) noexcept
      : DateTimeSymbolicFieldElement(document, fieldOwner, labels, fieldOwner.PlaceholderDate().Month())
  {
  }

  Ref<DateTimeSymbolicMonthFieldElement>
    DateTimeSymbolicMonthFieldElement::Create(dom::Document &document,
                                              DateTimeFieldElementFieldOwner &fieldOwner,
                                              const List<utf8_string> &labels) noexcept
  {
    auto element = CreateRef<DateTimeSymbolicMonthFieldElement>(document, fieldOwner, labels);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditMonthField());
    return element;
  }

  void DateTimeSymbolicMonthFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Month = static_cast<uint32>(ValueAsInteger() + 1);
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Month = static_cast<uint32>(PlaceholderValueAsInteger() + 1);
    }
  }

  void DateTimeSymbolicMonthFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.Month());
  }

#pragma endregion

#pragma region DateTimeWeekFieldElement

  namespace
  {
    /// @brief The week of the year @p date is in.
    KRYS_NODISCARD int WeekOfYear(const DateComponents &date) noexcept
    {
      auto week = DateComponents::FromMillisecondsSinceEpochForWeek(date.MillisecondsSinceEpoch());
      return week ? week->Week() : 1;
    }
  }

  DateTimeWeekFieldElement::DateTimeWeekFieldElement(dom::Document &document,
                                                     DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner, Range {1, 53},
                                    WeekOfYear(fieldOwner.PlaceholderDate()))
  {
  }

  Ref<DateTimeWeekFieldElement>
    DateTimeWeekFieldElement::Create(dom::Document &document,
                                     DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeWeekFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditWeekField());
    return element;
  }

  void DateTimeWeekFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Week = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Week = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeWeekFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.Week());
  }

#pragma endregion

#pragma region DateTimeYearFieldElement

  namespace
  {
    KRYS_NODISCARD int CurrentYear() noexcept
    {
      auto date = DateComponents::FromMillisecondsSinceEpochForDate(LocalTimeNow());
      return date ? date->FullYear() : DateComponents::MinimumYear();
    }
  }

  DateTimeYearFieldElement::DateTimeYearFieldElement(dom::Document &document,
                                                     DateTimeFieldElementFieldOwner &fieldOwner) noexcept
      : DateTimeNumericFieldElement(document, fieldOwner,
                                    Range {DateComponents::MinimumYear(), DateComponents::MaximumYear()},
                                    fieldOwner.PlaceholderDate().FullYear())
  {
  }

  Ref<DateTimeYearFieldElement>
    DateTimeYearFieldElement::Create(dom::Document &document,
                                     DateTimeFieldElementFieldOwner &fieldOwner) noexcept
  {
    auto element = CreateRef<DateTimeYearFieldElement>(document, fieldOwner);
    Initialize(*element, css::UserAgentParts::WebkitDatetimeEditYearField());
    return element;
  }

  void DateTimeYearFieldElement::PopulateDateTimeFieldsState(
    DateTimeFieldsState &state, DateTimePlaceholderIfNoValue placeholderIfNoValue) noexcept
  {
    if (HasValue())
    {
      state.Year = static_cast<uint32>(ValueAsInteger());
    }
    else if (placeholderIfNoValue == DateTimePlaceholderIfNoValue::Yes)
    {
      state.Year = static_cast<uint32>(PlaceholderValueAsInteger());
    }
  }

  void DateTimeYearFieldElement::SetValueAsDate(const DateComponents &date) noexcept
  {
    SetValueAsInteger(date.FullYear());
  }

  void DateTimeYearFieldElement::StepDown() noexcept
  {
    if (!HasValue())
    {
      SetValueAsInteger(CurrentYear(), EventBehavior::DispatchInputAndChangeEvents);
      return;
    }
    DateTimeNumericFieldElement::StepDown();
  }

  void DateTimeYearFieldElement::StepUp() noexcept
  {
    if (!HasValue())
    {
      SetValueAsInteger(CurrentYear(), EventBehavior::DispatchInputAndChangeEvents);
      return;
    }
    DateTimeNumericFieldElement::StepUp();
  }

#pragma endregion
}
