#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "catch2/catch_test_macros.hpp"
#include <cmath>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD utf8_stringview View(const char *string) noexcept
    {
      return utf8_stringview(reinterpret_cast<const char8 *>(string));
    }

    KRYS_NODISCARD std::string ToString(const Maybe<DateComponents> &date,
                                        SecondFormat format = SecondFormat::None) noexcept
    {
      if (!date)
      {
        return "(none)";
      }
      auto string = date->ToString(format);
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }
  }

  TEST_CASE("DateComponents: parsing months, dates and weeks", "[Platform][DateComponents]")
  {
    CHECK(ToString(DateComponents::FromParsingMonth(View("2024-02"))) == "2024-02");
    CHECK(ToString(DateComponents::FromParsingMonth(View("2024-13"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingMonth(View("024-01"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingMonth(View("12345-06"))) == "12345-06");
    CHECK(ToString(DateComponents::FromParsingMonth(View("275760-10"))) == "(none)");

    CHECK(ToString(DateComponents::FromParsingDate(View("2024-02-29"))) == "2024-02-29");
    CHECK(ToString(DateComponents::FromParsingDate(View("2023-02-29"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingDate(View("2024-04-31"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingDate(View("2024-1-01"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingDate(View("2024-01-01 "))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingDate(View("0000-01-01"))) == "(none)");

    // 2020 starts on a Wednesday in a leap year, so it has 53 weeks; 2021 has 52.
    CHECK(ToString(DateComponents::FromParsingWeek(View("2020-W53"))) == "2020-W53");
    CHECK(ToString(DateComponents::FromParsingWeek(View("2021-W53"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingWeek(View("2021-W00"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingWeek(View("2021-w01"))) == "(none)");
  }

  TEST_CASE("DateComponents: parsing times and local dates and times", "[Platform][DateComponents]")
  {
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30"))) == "09:30");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30:15"))) == "09:30:15");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30:15.5"))) == "09:30:15.5");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30:15.25"))) == "09:30:15.25");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30:15.125"))) == "09:30:15.125");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30:15.1234"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingTime(View("24:00"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingTime(View("9:30"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30")), SecondFormat::Second) == "09:30:00");
    CHECK(ToString(DateComponents::FromParsingTime(View("09:30")), SecondFormat::Millisecond)
          == "09:30:00.0");

    CHECK(ToString(DateComponents::FromParsingDateTimeLocal(View("2024-05-06T07:08"))) == "2024-05-06T07:08");
    CHECK(ToString(DateComponents::FromParsingDateTimeLocal(View("2024-05-06 07:08"))) == "2024-05-06T07:08");
    CHECK(ToString(DateComponents::FromParsingDateTimeLocal(View("2024-05-06t07:08"))) == "(none)");
    CHECK(ToString(DateComponents::FromParsingDateTimeLocal(View("275760-09-13T00:00")))
          == "275760-09-13T00:00");
    CHECK(ToString(DateComponents::FromParsingDateTimeLocal(View("275760-09-13T00:01"))) == "(none)");
  }

  TEST_CASE("DateComponents: milliseconds and months since the epoch", "[Platform][DateComponents]")
  {
    auto date = DateComponents::FromParsingDate(View("1970-01-02"));
    REQUIRE(date);
    CHECK(date->MillisecondsSinceEpoch() == 86400000.0);
    CHECK(ToString(DateComponents::FromMillisecondsSinceEpochForDate(86400000.0)) == "1970-01-02");
    CHECK(ToString(DateComponents::FromMillisecondsSinceEpochForDate(-86400000.0)) == "1969-12-31");

    auto week = DateComponents::FromParsingWeek(View("1970-W01"));
    REQUIRE(week);
    // The first week of 1970 starts on Monday 1969-12-29.
    CHECK(week->MillisecondsSinceEpoch() == -3 * 86400000.0);
    CHECK(ToString(DateComponents::FromMillisecondsSinceEpochForWeek(0)) == "1970-W01");

    auto month = DateComponents::FromParsingMonth(View("1971-03"));
    REQUIRE(month);
    CHECK(month->MonthsSinceEpoch() == 14);
    CHECK(ToString(DateComponents::FromMonthsSinceEpoch(14)) == "1971-03");
    CHECK(ToString(DateComponents::FromMillisecondsSinceEpochForMonth(0)) == "1970-01");

    CHECK(ToString(DateComponents::FromMillisecondsSinceMidnight(3'723'004)) == "01:02:03.004");
    CHECK(ToString(DateComponents::FromMillisecondsSinceMidnight(-60'000)) == "23:59");
    CHECK(ToString(DateComponents::FromMillisecondsSinceEpochForDateTimeLocal(86400000.0 + 60'000))
          == "1970-01-02T00:01");

    CHECK(ToString(DateComponents::FromMillisecondsSinceEpochForDate(DateComponents::MaximumDate()))
          == "275760-09-13");
    CHECK_FALSE(
      DateComponents::FromMillisecondsSinceEpochForDate(DateComponents::MaximumDate() + 86400000.0));
    CHECK_FALSE(DateComponents::FromMillisecondsSinceEpochForDate(NAN));
  }
}
