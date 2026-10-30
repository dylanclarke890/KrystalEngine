#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <array>
#include <charconv>
#include <chrono>
#include <climits>
#include <cmath>

namespace krys::boo
{
  double LocalTimeNow() noexcept
  {
    auto now = std::chrono::system_clock::now();
    double ms = static_cast<double>(
      std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    auto offset = std::chrono::current_zone()->get_info(now).offset;
    return ms + static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(offset).count());
  }

  namespace detail
  {
    /// @brief The characters left to parse.
    struct DateParsingBuffer
    {
      const char8 *Position {};
      const char8 *End {};

      KRYS_NODISCARD bool AtEnd() const noexcept
      {
        return Position == End;
      }

      KRYS_NODISCARD size_t LengthRemaining() const noexcept
      {
        return static_cast<size_t>(End - Position);
      }

      KRYS_NODISCARD char8 operator*() const noexcept
      {
        return *Position;
      }
    };
  }

  namespace
  {
    using detail::DateParsingBuffer;

    constexpr double MinutesPerHour = 60.0;
    constexpr double SecondsPerMinute = 60.0;
    constexpr double MsPerSecond = 1000.0;
    constexpr double MsPerDay = MsPerSecond * SecondsPerMinute * MinutesPerHour * 24.0;

    constexpr std::array<std::array<int, 12>, 2> FirstDayOfMonth {{
      {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
      {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
    }};

    KRYS_NODISCARD bool IsLeapYear(int year) noexcept
    {
      if (year % 4 != 0)
      {
        return false;
      }
      if (year % 400 == 0)
      {
        return true;
      }
      if (year % 100 == 0)
      {
        return false;
      }
      return true;
    }

    KRYS_NODISCARD int DaysInYear(int year) noexcept
    {
      return 365 + IsLeapYear(year);
    }

    KRYS_NODISCARD double DaysFrom1970ToYear(int year) noexcept
    {
      // The Gregorian Calendar rules for leap years:
      // Every fourth year is a leap year. 2004, 2008, and 2012 are leap years.
      // However, every hundredth year is not a leap year. 1900 and 2100 are not leap years.
      // Every four hundred years, there's a leap year after all. 2000 and 2400 are leap years.

      constexpr int leapDaysBefore1971By4Rule = 1'970 / 4;
      constexpr int excludedLeapDaysBefore1971By100Rule = 1'970 / 100;
      constexpr int leapDaysBefore1971By400Rule = 1'970 / 400;

      const double yearMinusOne = static_cast<double>(year) - 1;
      const double yearsToAddBy4Rule = std::floor(yearMinusOne / 4.0) - leapDaysBefore1971By4Rule;
      const double yearsToExcludeBy100Rule =
        std::floor(yearMinusOne / 100.0) - excludedLeapDaysBefore1971By100Rule;
      const double yearsToAddBy400Rule = std::floor(yearMinusOne / 400.0) - leapDaysBefore1971By400Rule;

      return 365.0 * (year - 1970.0) + yearsToAddBy4Rule - yearsToExcludeBy100Rule + yearsToAddBy400Rule;
    }

    KRYS_NODISCARD double MsToDays(double ms) noexcept
    {
      return std::floor(ms / MsPerDay);
    }

    KRYS_NODISCARD int DayInYear(int year, int month, int day) noexcept
    {
      return FirstDayOfMonth[IsLeapYear(year)][month] + day - 1;
    }

    KRYS_NODISCARD int DayInYear(double ms, int year) noexcept
    {
      double result = MsToDays(ms) - DaysFrom1970ToYear(year);
      return std::isnan(result) ? 0 : static_cast<int>(result);
    }

    /// @brief The number of days from 1970-01-01 to the date.
    KRYS_NODISCARD double DateToDaysFrom1970(int year, int month, int day) noexcept
    {
      year += month / 12;

      month %= 12;
      if (month < 0)
      {
        month += 12;
        --year;
      }

      double yearday = std::floor(DaysFrom1970ToYear(year));
      return yearday + DayInYear(year, month, day);
    }

    KRYS_NODISCARD int MsToYear(double ms) noexcept
    {
      double msAsYears = std::floor(ms / (MsPerDay * 365.2425));
      if (std::isnan(msAsYears))
      {
        msAsYears = 0;
      }
      int approxYear = static_cast<int>(msAsYears + 1'970);
      double msFromApproxYearTo1970 = MsPerDay * DaysFrom1970ToYear(approxYear);
      if (msFromApproxYearTo1970 > ms)
      {
        return approxYear - 1;
      }
      if (msFromApproxYearTo1970 + MsPerDay * DaysInYear(approxYear) <= ms)
      {
        return approxYear + 1;
      }
      return approxYear;
    }

    KRYS_NODISCARD int MonthFromDayInYear(int dayInYear, bool leapYear) noexcept
    {
      const int d = dayInYear;
      int step;

      if (d < (step = 31))
      {
        return 0;
      }
      step += (leapYear ? 29 : 28);
      if (d < step)
      {
        return 1;
      }
      if (d < (step += 31))
      {
        return 2;
      }
      if (d < (step += 30))
      {
        return 3;
      }
      if (d < (step += 31))
      {
        return 4;
      }
      if (d < (step += 30))
      {
        return 5;
      }
      if (d < (step += 31))
      {
        return 6;
      }
      if (d < (step += 31))
      {
        return 7;
      }
      if (d < (step += 30))
      {
        return 8;
      }
      if (d < (step += 31))
      {
        return 9;
      }
      if (d < step + 30)
      {
        return 10;
      }
      return 11;
    }

    KRYS_NODISCARD int DayInMonthFromDayInYear(int dayInYear, bool leapYear) noexcept
    {
      int month = MonthFromDayInYear(dayInYear, leapYear);
      return dayInYear - FirstDayOfMonth[leapYear][month] + 1;
    }

    // HTML defines minimum week of year is one.
    constexpr int MinimumWeekNumber = 1;

    // HTML defines maximum week of year is 53.
    constexpr int MaximumWeekNumber = 53;

    constexpr int MaximumMonthInMaximumYear = 8; // This is September, since months are 0 based.
    constexpr int MaximumDayInMaximumMonth = 13;
    constexpr int MaximumWeekInMaximumYear = 37; // The week of 275760-09-13

    constexpr std::array<int, 12> DaysInMonth {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // 'month' is 0-based.
    KRYS_NODISCARD int MaxDayOfMonth(int year, int month) noexcept
    {
      if (month != 1) // February?
      {
        return DaysInMonth[month];
      }
      return IsLeapYear(year) ? 29 : 28;
    }

    // 'month' is 0-based.
    KRYS_NODISCARD int DayOfWeek(int year, int month, int day) noexcept
    {
      int shiftedMonth = month + 2;
      // 2:January, 3:Feburuary, 4:March, ...

      // Zeller's congruence
      if (shiftedMonth <= 3)
      {
        shiftedMonth += 12;
        year--;
      }
      // 4:March, ..., 14:January, 15:February

      int highYear = year / 100;
      int lowYear = year % 100;
      // We add 6 to make the result Sunday-origin.
      int result =
        (day + 13 * shiftedMonth / 5 + lowYear + lowYear / 4 + highYear / 4 + 5 * highYear + 6) % 7;
      return result;
    }

    KRYS_NODISCARD unsigned CountDigits(DateParsingBuffer buffer) noexcept
    {
      auto begin = buffer.Position;
      while (!buffer.AtEnd() && text::IsASCIIDigit(*buffer))
      {
        ++buffer.Position;
      }
      return static_cast<unsigned>(buffer.Position - begin);
    }

    KRYS_NODISCARD bool SkipExactly(DateParsingBuffer &buffer, char8 character) noexcept
    {
      if (buffer.AtEnd() || *buffer != character)
      {
        return false;
      }
      ++buffer.Position;
      return true;
    }

    // Does not allow leading or trailing spaces, or a leading "+".
    KRYS_NODISCARD Maybe<int> ParseInt(DateParsingBuffer &buffer,
                                       unsigned maximumNumberOfDigitsToParse) noexcept
    {
      if (maximumNumberOfDigitsToParse > buffer.LengthRemaining() || !maximumNumberOfDigitsToParse)
      {
        return std::nullopt;
      }

      // We don't need to handle negative numbers for ISO 8601.

      int value = 0;
      unsigned digitsRemaining = 0;
      while (digitsRemaining < maximumNumberOfDigitsToParse)
      {
        if (!text::IsASCIIDigit(*buffer))
        {
          return std::nullopt;
        }
        int digit = *buffer - '0';
        if (value > (INT_MAX - digit) / 10) // Check for overflow.
        {
          return std::nullopt;
        }
        value = value * 10 + digit;
        ++digitsRemaining;
        ++buffer.Position;
      }
      return value;
    }

    KRYS_NODISCARD Maybe<int> ParseIntWithinLimits(DateParsingBuffer &buffer,
                                                   unsigned maximumNumberOfDigitsToParse, int minimumValue,
                                                   int maximumValue) noexcept
    {
      auto value = ParseInt(buffer, maximumNumberOfDigitsToParse);
      if (!(value && *value >= minimumValue && *value <= maximumValue))
      {
        return std::nullopt;
      }
      return value;
    }

    KRYS_NODISCARD bool WithinHTMLDateLimits(int year, int month) noexcept
    {
      if (year < DateComponents::MinimumYear())
      {
        return false;
      }
      if (year < DateComponents::MaximumYear())
      {
        return true;
      }
      return month <= MaximumMonthInMaximumYear;
    }

    KRYS_NODISCARD bool WithinHTMLDateLimits(int year, int month, int monthDay) noexcept
    {
      if (year < DateComponents::MinimumYear())
      {
        return false;
      }
      if (year < DateComponents::MaximumYear())
      {
        return true;
      }
      if (month < MaximumMonthInMaximumYear)
      {
        return true;
      }
      return monthDay <= MaximumDayInMaximumMonth;
    }

    KRYS_NODISCARD bool WithinHTMLDateLimits(int year, int month, int monthDay, int hour, int minute,
                                             int second, int millisecond) noexcept
    {
      if (year < DateComponents::MinimumYear())
      {
        return false;
      }
      if (year < DateComponents::MaximumYear())
      {
        return true;
      }
      if (month < MaximumMonthInMaximumYear)
      {
        return true;
      }
      if (monthDay < MaximumDayInMaximumMonth)
      {
        return true;
      }
      if (monthDay > MaximumDayInMaximumMonth)
      {
        return false;
      }
      // (year, month, monthDay) = (maximumYear, maximumMonthInMaximumYear, maximumDayInMaximumMonth)
      return !hour && !minute && !second && !millisecond;
    }

    template <typename TFunction>
    KRYS_NODISCARD Maybe<DateComponents> CreateFromString(utf8_stringview source,
                                                          TFunction &&parseFunction) noexcept
    {
      if (source.empty())
      {
        return std::nullopt;
      }

      DateParsingBuffer buffer {.Position = source.data(), .End = source.data() + source.size()};
      DateComponents date;
      if (!parseFunction(buffer, date) || !buffer.AtEnd())
      {
        return std::nullopt;
      }
      return date;
    }

    KRYS_NODISCARD double PositiveFmod(double value, double divider) noexcept
    {
      double remainder = std::fmod(value, divider);
      return remainder < 0 ? remainder + divider : remainder;
    }

    template <typename TFunction>
    KRYS_NODISCARD Maybe<DateComponents> CreateFromTimeOffset(double timeOffset,
                                                              TFunction &&function) noexcept
    {
      DateComponents result;
      if (!function(result, timeOffset))
      {
        return std::nullopt;
      }
      return result;
    }

    // Offset from January 1st to Monday of the ISO 8601's first week.
    //   ex. If January 1st is Friday, such Monday is 3 days later. Returns 3.
    KRYS_NODISCARD int OffsetTo1stWeekStart(int year) noexcept
    {
      int offsetTo1stWeekStart = 1 - DayOfWeek(year, 0, 1);
      if (offsetTo1stWeekStart <= -4)
      {
        offsetTo1stWeekStart += 7;
      }
      return offsetTo1stWeekStart;
    }

    void AppendPadded(StringBuilder &builder, int value, int width) noexcept
    {
      char buffer[16];
      auto end = std::to_chars(buffer, buffer + sizeof(buffer), value).ptr;
      for (auto length = static_cast<int>(end - buffer); length < width; ++length)
      {
        builder.Append(u8'0');
      }
      builder.Append(utf8_stringview(reinterpret_cast<const char8 *>(buffer), end - buffer));
    }
  }

  int DateComponents::MaxWeekNumberInYear() const noexcept
  {
    int day = DayOfWeek(_year, 0, 1); // January 1.
    return day == Thursday || (day == Wednesday && IsLeapYear(_year)) ? MaximumWeekNumber
                                                                      : MaximumWeekNumber - 1;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#months
  bool DateComponents::ParseYear(DateParsingBuffer &buffer) noexcept
  {
    unsigned digitsLength = CountDigits(buffer);
    // Needs at least 4 digits according to the standard.
    if (digitsLength < 4)
    {
      return false;
    }

    auto year = ParseIntWithinLimits(buffer, digitsLength, MinimumYear(), MaximumYear());
    if (!year)
    {
      return false;
    }

    _year = *year;
    return true;
  }

  Maybe<DateComponents> DateComponents::FromParsingMonth(utf8_stringview source) noexcept
  {
    return CreateFromString(source, [](auto &buffer, auto &date) { return date.ParseMonth(buffer); });
  }

  Maybe<DateComponents> DateComponents::FromParsingDate(utf8_stringview source) noexcept
  {
    return CreateFromString(source, [](auto &buffer, auto &date) { return date.ParseDate(buffer); });
  }

  Maybe<DateComponents> DateComponents::FromParsingWeek(utf8_stringview source) noexcept
  {
    return CreateFromString(source, [](auto &buffer, auto &date) { return date.ParseWeek(buffer); });
  }

  Maybe<DateComponents> DateComponents::FromParsingTime(utf8_stringview source) noexcept
  {
    return CreateFromString(source, [](auto &buffer, auto &date) { return date.ParseTime(buffer); });
  }

  Maybe<DateComponents> DateComponents::FromParsingDateTimeLocal(utf8_stringview source) noexcept
  {
    return CreateFromString(source, [](auto &buffer, auto &date) { return date.ParseDateTimeLocal(buffer); });
  }

  bool DateComponents::AddDay(int dayDiff) noexcept
  {
    krys_debug_assert(_monthDay);

    int day = _monthDay + dayDiff;
    if (day > MaxDayOfMonth(_year, _month))
    {
      day = _monthDay;
      int year = _year;
      int month = _month;
      int maxDay = MaxDayOfMonth(year, month);
      for (; dayDiff > 0; --dayDiff)
      {
        ++day;
        if (day > maxDay)
        {
          day = 1;
          ++month;
          if (month >= 12) // month is 0-origin.
          {
            month = 0;
            ++year;
          }
          maxDay = MaxDayOfMonth(year, month);
        }
      }
      if (!WithinHTMLDateLimits(year, month, day))
      {
        return false;
      }
      _year = year;
      _month = month;
    }
    else if (day < 1)
    {
      int month = _month;
      int year = _year;
      day = _monthDay;
      for (; dayDiff < 0; ++dayDiff)
      {
        --day;
        if (day < 1)
        {
          --month;
          if (month < 0)
          {
            month = 11;
            --year;
          }
          day = MaxDayOfMonth(year, month);
        }
      }
      if (!WithinHTMLDateLimits(year, month, day))
      {
        return false;
      }
      _year = year;
      _month = month;
    }
    else
    {
      if (!WithinHTMLDateLimits(_year, _month, day))
      {
        return false;
      }
    }
    _monthDay = day;
    return true;
  }

  bool DateComponents::AddMinute(int minute) noexcept
  {
    // This function is used to adjust timezone offset. So _year, _month,
    // _monthDay have values between the lower and higher limits.
    krys_debug_assert(WithinHTMLDateLimits(_year, _month, _monthDay));

    int carry;
    // minute can be negative or greater than 59.
    minute += _minute;
    if (minute > 59)
    {
      carry = minute / 60;
      minute = minute % 60;
    }
    else if (minute < 0)
    {
      carry = (59 - minute) / 60;
      minute += carry * 60;
      carry = -carry;
      krys_debug_assert(minute >= 0 && minute <= 59);
    }
    else
    {
      if (!WithinHTMLDateLimits(_year, _month, _monthDay, _hour, minute, _second, _millisecond))
      {
        return false;
      }
      _minute = minute;
      return true;
    }

    int hour = _hour + carry;
    if (hour > 23)
    {
      carry = hour / 24;
      hour = hour % 24;
    }
    else if (hour < 0)
    {
      carry = (23 - hour) / 24;
      hour += carry * 24;
      carry = -carry;
      krys_debug_assert(hour >= 0 && hour <= 23);
    }
    else
    {
      if (!WithinHTMLDateLimits(_year, _month, _monthDay, hour, minute, _second, _millisecond))
      {
        return false;
      }
      _minute = minute;
      _hour = hour;
      return true;
    }
    if (!AddDay(carry))
    {
      return false;
    }
    if (!WithinHTMLDateLimits(_year, _month, _monthDay, hour, minute, _second, _millisecond))
    {
      return false;
    }
    _minute = minute;
    _hour = hour;
    return true;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#time-zones
  // Parses a timezone part, and adjust year, month, monthDay, hour, minute, second, millisecond.
  bool DateComponents::ParseTimeZone(DateParsingBuffer &buffer) noexcept
  {
    if (SkipExactly(buffer, u8'Z'))
    {
      return true;
    }

    bool minus;
    if (SkipExactly(buffer, u8'+'))
    {
      minus = false;
    }
    else if (SkipExactly(buffer, u8'-'))
    {
      minus = true;
    }
    else
    {
      return false;
    }

    auto hour = ParseIntWithinLimits(buffer, 2, 0, 23);
    if (!hour)
    {
      return false;
    }

    if (!SkipExactly(buffer, u8':'))
    {
      return false;
    }

    auto minute = ParseIntWithinLimits(buffer, 2, 0, 59);
    if (!minute)
    {
      return false;
    }

    if (minus)
    {
      *hour = -*hour;
      *minute = -*minute;
    }

    // Subtract the timezone offset.
    if (!AddMinute(-(*hour * 60 + *minute)))
    {
      return false;
    }
    return true;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#months
  bool DateComponents::ParseMonth(DateParsingBuffer &buffer) noexcept
  {
    if (!ParseYear(buffer))
    {
      return false;
    }

    if (!SkipExactly(buffer, u8'-'))
    {
      return false;
    }

    auto month = ParseIntWithinLimits(buffer, 2, 1, 12);
    if (!month)
    {
      return false;
    }
    --*month;

    if (!WithinHTMLDateLimits(_year, *month))
    {
      return false;
    }

    _month = *month;
    _type = DateComponentsType::Month;
    return true;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#dates
  bool DateComponents::ParseDate(DateParsingBuffer &buffer) noexcept
  {
    if (!ParseMonth(buffer))
    {
      return false;
    }

    if (!SkipExactly(buffer, u8'-'))
    {
      return false;
    }

    auto day = ParseIntWithinLimits(buffer, 2, 1, MaxDayOfMonth(_year, _month));
    if (!day)
    {
      return false;
    }

    if (!WithinHTMLDateLimits(_year, _month, *day))
    {
      return false;
    }

    _monthDay = *day;
    _type = DateComponentsType::Date;
    return true;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#weeks
  bool DateComponents::ParseWeek(DateParsingBuffer &buffer) noexcept
  {
    if (!ParseYear(buffer))
    {
      return false;
    }

    if (!SkipExactly(buffer, u8'-'))
    {
      return false;
    }
    if (!SkipExactly(buffer, u8'W'))
    {
      return false;
    }

    auto week = ParseIntWithinLimits(buffer, 2, MinimumWeekNumber, MaxWeekNumberInYear());
    if (!week)
    {
      return false;
    }

    if (_year == MaximumYear() && *week > MaximumWeekInMaximumYear)
    {
      return false;
    }

    _week = *week;
    _type = DateComponentsType::Week;
    return true;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#times
  bool DateComponents::ParseTime(DateParsingBuffer &buffer) noexcept
  {
    auto hour = ParseIntWithinLimits(buffer, 2, 0, 23);
    if (!hour)
    {
      return false;
    }

    if (!SkipExactly(buffer, u8':'))
    {
      return false;
    }

    auto minute = ParseIntWithinLimits(buffer, 2, 0, 59);
    if (!minute)
    {
      return false;
    }

    // Optional second part.
    // Do not return with false because the part is optional.
    Maybe<int> second;
    Maybe<int> millisecond;

    auto temporaryBuffer = buffer;
    if (SkipExactly(temporaryBuffer, u8':'))
    {
      second = ParseIntWithinLimits(temporaryBuffer, 2, 0, 59);
      if (second)
      {
        // Only after successfully parsing and validating 'second', do we increment the real buffer.
        buffer.Position = temporaryBuffer.Position;

        // Optional fractional second part.
        if (SkipExactly(temporaryBuffer, u8'.'))
        {
          unsigned digitsLength = CountDigits(temporaryBuffer);
          if (digitsLength > 0)
          {
            if (digitsLength == 1)
            {
              millisecond = ParseInt(temporaryBuffer, 1);
              *millisecond *= 100;
            }
            else if (digitsLength == 2)
            {
              millisecond = ParseInt(temporaryBuffer, 2);
              *millisecond *= 10;
            }
            else if (digitsLength == 3)
            {
              millisecond = ParseInt(temporaryBuffer, 3);
            }
            else
            {
              return false;
            }

            // Due to the countDigits above, the parseInt calls should all be successful.
            krys_debug_assert(millisecond);

            // Only after successfully parsing and validating 'millisecond', do we increment the real buffer.
            buffer.Position += 1 + digitsLength;
          }
        }
      }
    }

    _hour = *hour;
    _minute = *minute;
    _second = second.value_or(0);
    _millisecond = millisecond.value_or(0);
    _type = DateComponentsType::Time;
    return true;
  }

  // https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#local-dates-and-times
  bool DateComponents::ParseDateTimeLocal(DateParsingBuffer &buffer) noexcept
  {
    if (!ParseDate(buffer))
    {
      return false;
    }

    // Both 'T' and a space are allowed as the separator (see https://github.com/whatwg/html/issues/2276).
    if (!SkipExactly(buffer, u8'T') && !SkipExactly(buffer, u8' '))
    {
      return false;
    }

    if (!ParseTime(buffer))
    {
      return false;
    }

    if (!WithinHTMLDateLimits(_year, _month, _monthDay, _hour, _minute, _second, _millisecond))
    {
      return false;
    }

    _type = DateComponentsType::DateTimeLocal;
    return true;
  }

  Maybe<DateComponents> DateComponents::FromMillisecondsSinceEpochForDate(double ms) noexcept
  {
    return CreateFromTimeOffset(ms, [](auto &date, auto ms)
                                { return date.SetMillisecondsSinceEpochForDate(ms); });
  }

  Maybe<DateComponents> DateComponents::FromMillisecondsSinceEpochForDateTimeLocal(double ms) noexcept
  {
    return CreateFromTimeOffset(ms, [](auto &date, auto ms)
                                { return date.SetMillisecondsSinceEpochForDateTimeLocal(ms); });
  }

  Maybe<DateComponents> DateComponents::FromMillisecondsSinceEpochForMonth(double ms) noexcept
  {
    return CreateFromTimeOffset(ms, [](auto &date, auto ms)
                                { return date.SetMillisecondsSinceEpochForMonth(ms); });
  }

  Maybe<DateComponents> DateComponents::FromMillisecondsSinceEpochForWeek(double ms) noexcept
  {
    return CreateFromTimeOffset(ms, [](auto &date, auto ms)
                                { return date.SetMillisecondsSinceEpochForWeek(ms); });
  }

  Maybe<DateComponents> DateComponents::FromMillisecondsSinceMidnight(double ms) noexcept
  {
    return CreateFromTimeOffset(ms,
                                [](auto &date, auto ms) { return date.SetMillisecondsSinceMidnight(ms); });
  }

  Maybe<DateComponents> DateComponents::FromMonthsSinceEpoch(double months) noexcept
  {
    return CreateFromTimeOffset(months,
                                [](auto &date, auto months) { return date.SetMonthsSinceEpoch(months); });
  }

  void DateComponents::SetMillisecondsSinceMidnightInternal(double msInDay) noexcept
  {
    krys_debug_assert(msInDay >= 0 && msInDay < MsPerDay);
    _millisecond = static_cast<int>(std::fmod(msInDay, MsPerSecond));
    double value = std::floor(msInDay / MsPerSecond);
    _second = static_cast<int>(std::fmod(value, SecondsPerMinute));
    value = std::floor(value / SecondsPerMinute);
    _minute = static_cast<int>(std::fmod(value, MinutesPerHour));
    _hour = static_cast<int>(value / MinutesPerHour);
  }

  bool DateComponents::SetMillisecondsSinceEpochForDateInternal(double ms) noexcept
  {
    _year = MsToYear(ms);
    int yearDay = DayInYear(ms, _year);
    _month = MonthFromDayInYear(yearDay, IsLeapYear(_year));
    _monthDay = DayInMonthFromDayInYear(yearDay, IsLeapYear(_year));
    return true;
  }

  bool DateComponents::SetMillisecondsSinceEpochForDate(double ms) noexcept
  {
    _type = DateComponentsType::Invalid;
    if (!std::isfinite(ms))
    {
      return false;
    }
    if (!SetMillisecondsSinceEpochForDateInternal(std::round(ms)))
    {
      return false;
    }
    if (!WithinHTMLDateLimits(_year, _month, _monthDay))
    {
      return false;
    }
    _type = DateComponentsType::Date;
    return true;
  }

  bool DateComponents::SetMillisecondsSinceEpochForDateTimeLocal(double ms) noexcept
  {
    _type = DateComponentsType::Invalid;
    if (!std::isfinite(ms))
    {
      return false;
    }
    ms = std::round(ms);
    SetMillisecondsSinceMidnightInternal(PositiveFmod(ms, MsPerDay));
    if (!SetMillisecondsSinceEpochForDateInternal(ms))
    {
      return false;
    }
    if (!WithinHTMLDateLimits(_year, _month, _monthDay, _hour, _minute, _second, _millisecond))
    {
      return false;
    }
    _type = DateComponentsType::DateTimeLocal;
    return true;
  }

  bool DateComponents::SetMillisecondsSinceEpochForMonth(double ms) noexcept
  {
    _type = DateComponentsType::Invalid;
    if (!std::isfinite(ms))
    {
      return false;
    }
    if (!SetMillisecondsSinceEpochForDateInternal(std::round(ms)))
    {
      return false;
    }
    if (!WithinHTMLDateLimits(_year, _month))
    {
      return false;
    }
    _type = DateComponentsType::Month;
    return true;
  }

  bool DateComponents::SetMillisecondsSinceMidnight(double ms) noexcept
  {
    _type = DateComponentsType::Invalid;
    if (!std::isfinite(ms))
    {
      return false;
    }
    SetMillisecondsSinceMidnightInternal(PositiveFmod(std::round(ms), MsPerDay));
    _type = DateComponentsType::Time;
    return true;
  }

  bool DateComponents::SetMonthsSinceEpoch(double months) noexcept
  {
    if (!std::isfinite(months))
    {
      return false;
    }
    months = std::round(months);
    double doubleMonth = PositiveFmod(months, 12);
    double doubleYear = 1'970 + (months - doubleMonth) / 12;
    if (doubleYear < MinimumYear() || MaximumYear() < doubleYear)
    {
      return false;
    }
    int year = static_cast<int>(doubleYear);
    int month = static_cast<int>(doubleMonth);
    if (!WithinHTMLDateLimits(year, month))
    {
      return false;
    }
    _year = year;
    _month = month;
    _type = DateComponentsType::Month;
    return true;
  }

  bool DateComponents::SetMillisecondsSinceEpochForWeek(double ms) noexcept
  {
    _type = DateComponentsType::Invalid;
    if (!std::isfinite(ms))
    {
      return false;
    }
    ms = std::round(ms);

    _year = MsToYear(ms);
    if (_year < MinimumYear() || _year > MaximumYear())
    {
      return false;
    }

    int yearDay = DayInYear(ms, _year);
    int offset = OffsetTo1stWeekStart(_year);
    if (yearDay < offset)
    {
      // The day belongs to the last week of the previous year.
      _year--;
      if (_year <= MinimumYear())
      {
        return false;
      }
      _week = MaxWeekNumberInYear();
    }
    else
    {
      _week = ((yearDay - offset) / 7) + 1;
      if (_week > MaxWeekNumberInYear())
      {
        _year++;
        _week = 1;
      }
      if (_year > MaximumYear() || (_year == MaximumYear() && _week > MaximumWeekInMaximumYear))
      {
        return false;
      }
    }
    _type = DateComponentsType::Week;
    return true;
  }

  double DateComponents::MillisecondsSinceEpochForTime() const noexcept
  {
    krys_debug_assert(_type == DateComponentsType::Time || _type == DateComponentsType::DateTimeLocal);
    return ((_hour * MinutesPerHour + _minute) * SecondsPerMinute + _second) * MsPerSecond + _millisecond;
  }

  double DateComponents::MillisecondsSinceEpoch() const noexcept
  {
    switch (_type)
    {
      case DateComponentsType::Date: return DateToDaysFrom1970(_year, _month, _monthDay) * MsPerDay;
      case DateComponentsType::DateTimeLocal:
        return DateToDaysFrom1970(_year, _month, _monthDay) * MsPerDay + MillisecondsSinceEpochForTime();
      case DateComponentsType::Month: return DateToDaysFrom1970(_year, _month, 1) * MsPerDay;
      case DateComponentsType::Time:  return MillisecondsSinceEpochForTime();
      case DateComponentsType::Week:
        return (DateToDaysFrom1970(_year, 0, 1) + OffsetTo1stWeekStart(_year) + (_week - 1) * 7) * MsPerDay;
      case DateComponentsType::Invalid: break;
    }
    krys_debug_assert(false);
    return InvalidMilliseconds();
  }

  double DateComponents::MonthsSinceEpoch() const noexcept
  {
    krys_debug_assert(_type == DateComponentsType::Month);
    return (_year - 1'970) * 12 + _month;
  }

  utf8_string DateComponents::ToStringForTime(SecondFormat format) const noexcept
  {
    krys_debug_assert(_type == DateComponentsType::DateTimeLocal || _type == DateComponentsType::Time);
    SecondFormat effectiveFormat = format;
    if (_millisecond)
    {
      effectiveFormat = SecondFormat::Millisecond;
    }
    else if (format == SecondFormat::None && _second)
    {
      effectiveFormat = SecondFormat::Second;
    }

    StringBuilder builder;
    AppendPadded(builder, _hour, 2);
    builder.Append(u8':');
    AppendPadded(builder, _minute, 2);
    if (effectiveFormat == SecondFormat::None)
    {
      return krys::move(builder).ToString();
    }

    builder.Append(u8':');
    AppendPadded(builder, _second, 2);
    if (effectiveFormat == SecondFormat::Second)
    {
      return krys::move(builder).ToString();
    }

    builder.Append(u8'.');
    if (!(_millisecond % 100))
    {
      AppendPadded(builder, _millisecond / 100, 1);
    }
    else if (!(_millisecond % 10))
    {
      AppendPadded(builder, _millisecond / 10, 2);
    }
    else
    {
      AppendPadded(builder, _millisecond, 3);
    }
    return krys::move(builder).ToString();
  }

  utf8_string DateComponents::ToString(SecondFormat format) const noexcept
  {
    StringBuilder builder;
    switch (_type)
    {
      case DateComponentsType::Date:
        AppendPadded(builder, _year, 4);
        builder.Append(u8'-');
        AppendPadded(builder, _month + 1, 2);
        builder.Append(u8'-');
        AppendPadded(builder, _monthDay, 2);
        break;
      case DateComponentsType::DateTimeLocal:
        AppendPadded(builder, _year, 4);
        builder.Append(u8'-');
        AppendPadded(builder, _month + 1, 2);
        builder.Append(u8'-');
        AppendPadded(builder, _monthDay, 2);
        builder.Append(u8'T', ToStringForTime(format));
        break;
      case DateComponentsType::Month:
        AppendPadded(builder, _year, 4);
        builder.Append(u8'-');
        AppendPadded(builder, _month + 1, 2);
        break;
      case DateComponentsType::Time: return ToStringForTime(format);
      case DateComponentsType::Week:
        AppendPadded(builder, _year, 4);
        builder.Append(u8"-W");
        AppendPadded(builder, _week, 2);
        break;
      case DateComponentsType::Invalid: krys_debug_assert(false); return u8"(Invalid DateComponents)";
    }
    return krys::move(builder).ToString();
  }
}
