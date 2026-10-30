#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <limits>

namespace krys::boo
{
  enum class SecondFormat : uint8
  {
    None,       // Suppress the second part and the millisecond part if they are 0.
    Second,     // Always show the second part, and suppress the millisecond part if it is 0.
    Millisecond // Always show the second part and the millisecond part.
  };

  enum class DateComponentsType : uint8
  {
    Invalid,
    Date,
    DateTimeLocal,
    Month,
    Time,
    Week,
  };

  /// @brief The local time now, in milliseconds since the epoch.
  KRYS_NODISCARD double LocalTimeNow() noexcept;

  namespace detail
  {
    struct DateParsingBuffer;
  }

  /// @brief One of these date and time combinations:
  /// - Month: year-month
  /// - Date: year-month-day
  /// - Week: year-week
  /// - Time: hour-minute-second-millisecond
  /// - DateTimeLocal: year-month-day hour-minute-second-millisecond
  class DateComponents
  {
  private:
    // _weekDay values
    enum
    {
      Sunday = 0,
      Monday,
      Tuesday,
      Wednesday,
      Thursday,
      Friday,
      Saturday,
    };

    int _millisecond {0}; // 0 - 999
    int _second {0};
    int _minute {0};
    int _hour {0};
    int _monthDay {0}; // 1 - 31
    int _month {0};    // 0:January - 11:December
    int _year {0};     // 1582 -
    int _week {0};     // 1 - 53
    DateComponentsType _type {DateComponentsType::Invalid};

  public:
    DateComponents() noexcept = default;

    KRYS_NODISCARD int Millisecond() const noexcept
    {
      return _millisecond;
    }

    KRYS_NODISCARD int Second() const noexcept
    {
      return _second;
    }

    KRYS_NODISCARD int Minute() const noexcept
    {
      return _minute;
    }

    KRYS_NODISCARD int Hour() const noexcept
    {
      return _hour;
    }

    KRYS_NODISCARD int MonthDay() const noexcept
    {
      return _monthDay;
    }

    KRYS_NODISCARD int Month() const noexcept
    {
      return _month;
    }

    KRYS_NODISCARD int FullYear() const noexcept
    {
      return _year;
    }

    KRYS_NODISCARD int Week() const noexcept
    {
      return _week;
    }

    KRYS_NODISCARD DateComponentsType Type() const noexcept
    {
      return _type;
    }

    /// @brief An ISO 8601 representation; @p format applies to the DateTimeLocal and Time types.
    KRYS_NODISCARD utf8_string ToString(SecondFormat format = SecondFormat::None) const noexcept;

    /// @brief Sets year and month.
    KRYS_NODISCARD static Maybe<DateComponents> FromParsingMonth(utf8_stringview source) noexcept;
    /// @brief Sets year, month and monthDay.
    KRYS_NODISCARD static Maybe<DateComponents> FromParsingDate(utf8_stringview source) noexcept;
    /// @brief Sets year and week.
    KRYS_NODISCARD static Maybe<DateComponents> FromParsingWeek(utf8_stringview source) noexcept;
    /// @brief Sets hour, minute, second and millisecond.
    KRYS_NODISCARD static Maybe<DateComponents> FromParsingTime(utf8_stringview source) noexcept;
    /// @brief Sets year, month, monthDay, hour, minute, second and millisecond.
    KRYS_NODISCARD static Maybe<DateComponents> FromParsingDateTimeLocal(utf8_stringview source) noexcept;

    /// @brief For the Date type: sets the year, month and monthDay.
    KRYS_NODISCARD static Maybe<DateComponents> FromMillisecondsSinceEpochForDate(double ms) noexcept;
    /// @brief For the DateTimeLocal type: sets the year, month, monthDay, hour, minute, second and
    /// millisecond.
    KRYS_NODISCARD static Maybe<DateComponents>
      FromMillisecondsSinceEpochForDateTimeLocal(double ms) noexcept;
    /// @brief For the Month type: sets the year and month.
    KRYS_NODISCARD static Maybe<DateComponents> FromMillisecondsSinceEpochForMonth(double ms) noexcept;
    /// @brief For the Week type: sets the year and week.
    KRYS_NODISCARD static Maybe<DateComponents> FromMillisecondsSinceEpochForWeek(double ms) noexcept;
    /// @brief For the Time type: sets the hour, minute, second and millisecond.
    KRYS_NODISCARD static Maybe<DateComponents> FromMillisecondsSinceMidnight(double ms) noexcept;
    /// @brief For the Month type: sets the year and month.
    KRYS_NODISCARD static Maybe<DateComponents> FromMonthsSinceEpoch(double months) noexcept;

    /// @brief The number of milliseconds from 1970-01-01 00:00:00 UTC; a DateTimeLocal is taken to be in UTC.
    KRYS_NODISCARD double MillisecondsSinceEpoch() const noexcept;

    /// @brief The number of months from 1970-01; only for the Month type.
    KRYS_NODISCARD double MonthsSinceEpoch() const noexcept;

    KRYS_NODISCARD static constexpr double InvalidMilliseconds() noexcept
    {
      return std::numeric_limits<double>::quiet_NaN();
    }

    // The limits of the From*() functions, MillisecondsSinceEpoch() and MonthsSinceEpoch().
    KRYS_NODISCARD static constexpr double MinimumDate() noexcept
    {
      return -62135596800000.0; // 0001-01-01T00:00Z
    }

    KRYS_NODISCARD static constexpr double MinimumDateTime() noexcept
    {
      return -62135596800000.0; // ditto.
    }

    KRYS_NODISCARD static constexpr double MinimumMonth() noexcept
    {
      return (1 - 1'970) * 12.0 + 1 - 1; // 0001-01
    }

    KRYS_NODISCARD static constexpr double MinimumTime() noexcept
    {
      return 0; // 00:00:00.000
    }

    KRYS_NODISCARD static constexpr double MinimumWeek() noexcept
    {
      return -62135596800000.0; // 0001-01-01, the first Monday of 0001.
    }

    KRYS_NODISCARD static constexpr double MaximumDate() noexcept
    {
      return 8640000000000000.0; // 275760-09-13T00:00Z
    }

    KRYS_NODISCARD static constexpr double MaximumDateTime() noexcept
    {
      return 8640000000000000.0; // ditto.
    }

    KRYS_NODISCARD static constexpr double MaximumMonth() noexcept
    {
      return (275'760 - 1'970) * 12.0 + 9 - 1; // 275760-09
    }

    KRYS_NODISCARD static constexpr double MaximumTime() noexcept
    {
      return 86'399'999; // 23:59:59.999
    }

    KRYS_NODISCARD static constexpr double MaximumWeek() noexcept
    {
      return 8639999568000000.0; // 275760-09-08, the Monday of the week including 275760-09-13.
    }

    /// @brief HTML uses ISO-8601 format with year >= 1. The Gregorian calendar started in 1582, but
    /// 0001-01-01 is supported with its rules.
    KRYS_NODISCARD static constexpr int MinimumYear() noexcept
    {
      return 1;
    }

    /// @brief ECMAScript dates can't represent dates later than 275760-09-13T00:00Z, so neither do HTML's.
    KRYS_NODISCARD static constexpr int MaximumYear() noexcept
    {
      return 275'760;
    }

  private:
    KRYS_NODISCARD bool ParseYear(detail::DateParsingBuffer &buffer) noexcept;
    KRYS_NODISCARD bool ParseTimeZone(detail::DateParsingBuffer &buffer) noexcept;
    KRYS_NODISCARD bool ParseMonth(detail::DateParsingBuffer &buffer) noexcept;
    KRYS_NODISCARD bool ParseDate(detail::DateParsingBuffer &buffer) noexcept;
    KRYS_NODISCARD bool ParseWeek(detail::DateParsingBuffer &buffer) noexcept;
    KRYS_NODISCARD bool ParseTime(detail::DateParsingBuffer &buffer) noexcept;
    KRYS_NODISCARD bool ParseDateTimeLocal(detail::DateParsingBuffer &buffer) noexcept;

    // These take the number of milliseconds since 1970-01-01 00:00:00.000 UTC and update every field of their
    // type, returning whether they succeeded.
    KRYS_NODISCARD bool SetMillisecondsSinceEpochForDate(double ms) noexcept;
    KRYS_NODISCARD bool SetMillisecondsSinceEpochForDateTimeLocal(double ms) noexcept;
    KRYS_NODISCARD bool SetMillisecondsSinceEpochForMonth(double ms) noexcept;
    KRYS_NODISCARD bool SetMillisecondsSinceEpochForWeek(double ms) noexcept;
    KRYS_NODISCARD bool SetMillisecondsSinceMidnight(double ms) noexcept;
    KRYS_NODISCARD bool SetMonthsSinceEpoch(double months) noexcept;
    KRYS_NODISCARD bool SetMillisecondsSinceEpochForDateInternal(double ms) noexcept;
    void SetMillisecondsSinceMidnightInternal(double msInDay) noexcept;

    /// @brief The maximum week number in the year: 52 or 53.
    KRYS_NODISCARD int MaxWeekNumberInYear() const noexcept;

    KRYS_NODISCARD bool AddDay(int dayDiff) noexcept;
    KRYS_NODISCARD bool AddMinute(int minute) noexcept;

    KRYS_NODISCARD double MillisecondsSinceEpochForTime() const noexcept;
    KRYS_NODISCARD utf8_string ToStringForTime(SecondFormat format) const noexcept;
  };
}
