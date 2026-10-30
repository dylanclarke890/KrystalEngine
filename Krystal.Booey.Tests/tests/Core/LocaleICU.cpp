#include "Krystal.Booey/Core/LocaleICU.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"

namespace krys::boo::tests
{
  TEST_CASE("LocaleICU: date and time formats", "[Core][LocaleICU]")
  {
    SECTION("en-US")
    {
      auto locale = Locale::Create(u8"en-US");
      CHECK(locale->DateFormat() == u8"M/d/yy");
      CHECK(locale->MonthFormat() == u8"MMMM yyyy");
      CHECK(locale->ShortMonthFormat() == u8"MMM yyyy");
      // A narrow no-break space before the period.
      CHECK(locale->TimeFormat() == u8"h:mm:ss\u202fa");
      CHECK(locale->ShortTimeFormat() == u8"h:mm\u202fa");
      CHECK(locale->DateTimeFormatWithSeconds() == u8"M/d/yy, h:mm:ss\u202fa");
      CHECK(locale->DateTimeFormatWithoutSeconds() == u8"M/d/yy, h:mm\u202fa");
      CHECK(locale->TimeAMPMLabels() == List<utf8_string> {u8"AM", u8"PM"});
      CHECK(locale->DefaultWritingDirection() == Locale::WritingDirection::LeftToRight);
    }

    SECTION("en-GB and de: 24 hour clocks")
    {
      auto british = Locale::Create(u8"en-GB");
      CHECK(british->DateFormat() == u8"dd/MM/y");
      CHECK(british->ShortTimeFormat() == u8"HH:mm");
      CHECK(british->DateTimeFormatWithSeconds() == u8"dd/MM/y, HH:mm:ss");

      auto german = Locale::Create(u8"de");
      CHECK(german->DateFormat() == u8"dd.MM.yy");
      CHECK(german->TimeFormat() == u8"HH:mm:ss");
    }

    SECTION("ja and ko: literals and periods first")
    {
      auto japanese = Locale::Create(u8"ja");
      CHECK(japanese->DateFormat() == u8"y/MM/dd");
      CHECK(japanese->ShortMonthFormat() == u8"yyyy\u5e74M\u6708");

      auto korean = Locale::Create(u8"ko");
      CHECK(korean->ShortTimeFormat() == u8"a h:mm");
    }

    SECTION("ar-EG and fa: right to left")
    {
      CHECK(Locale::Create(u8"ar-EG")->DefaultWritingDirection() == Locale::WritingDirection::RightToLeft);
      CHECK(Locale::Create(u8"fa")->DefaultWritingDirection() == Locale::WritingDirection::RightToLeft);
    }
  }

  TEST_CASE("LocaleICU: month labels", "[Core][LocaleICU]")
  {
    auto english = Locale::Create(u8"en-US");
    REQUIRE(english->MonthLabels().size() == 12);
    CHECK(english->MonthLabels()[0] == u8"January");
    CHECK(english->MonthLabels()[11] == u8"December");
    CHECK(english->ShortMonthLabels()[8] == u8"Sep");
    CHECK(english->StandAloneMonthLabels()[4] == u8"May");
    CHECK(english->ShortStandAloneMonthLabels()[1] == u8"Feb");

    auto german = Locale::Create(u8"de");
    CHECK(german->MonthLabels()[2] == u8"M\u00e4rz");
    CHECK(german->ShortMonthLabels()[0] == u8"Jan.");
  }

  TEST_CASE("LocaleICU: numbers", "[Core][LocaleICU]")
  {
    SECTION("en-US")
    {
      auto locale = Locale::Create(u8"en-US");
      CHECK(locale->LocalizedDecimalSeparator() == u8".");
      CHECK(locale->ConvertToLocalizedNumber(u8"-12.5") == u8"-12.5");
      CHECK(locale->ConvertFromLocalizedNumber(u8" 12.5 ") == u8"12.5");
      CHECK(locale->ConvertFromLocalizedNumber(u8"+7") == u8"7");
      CHECK(locale->ConvertFromLocalizedNumber(u8"7.") == u8"7");
      // Group separators aren't numbers.
      CHECK(locale->ConvertFromLocalizedNumber(u8"1,000") == u8"1,000");
    }

    SECTION("de: a decimal comma")
    {
      auto locale = Locale::Create(u8"de");
      CHECK(locale->LocalizedDecimalSeparator() == u8",");
      CHECK(locale->ConvertToLocalizedNumber(u8"3.25") == u8"3,25");
      CHECK(locale->ConvertFromLocalizedNumber(u8"3,25") == u8"3.25");
    }

    SECTION("ar-EG: Arabic-Indic digits")
    {
      auto locale = Locale::Create(u8"ar-EG");
      auto localized = locale->ConvertToLocalizedNumber(u8"-12.5");
      // An Arabic letter mark before the minus sign.
      CHECK(localized == u8"\u061c-\u0661\u0662\u066b\u0665");
      CHECK(locale->ConvertFromLocalizedNumber(localized) == u8"-12.5");
      CHECK(locale->ConvertToLocalizedNumber(u8"09") == u8"\u0660\u0669");
    }
  }

  TEST_CASE("LocaleICU: formatting dates", "[Core][LocaleICU]")
  {
    auto english = Locale::Create(u8"en-US");
    auto date = DateComponents::FromParsingDate(u8"2026-03-07");
    REQUIRE(date);
    CHECK(english->FormatDateTime(*date) == u8"03/07/2026");

    auto month = DateComponents::FromParsingMonth(u8"2026-09");
    REQUIRE(month);
    CHECK(english->FormatDateTime(*month) == u8"September 2026");
    CHECK(english->FormatDateTime(*month, Locale::FormatType::Short) == u8"Sep 2026");

    auto time = DateComponents::FromParsingTime(u8"00:05:09.250");
    REQUIRE(time);
    CHECK(english->FormatDateTime(*time) == u8"12:05:09.250\u202fAM");
    CHECK(english->FormatDateTime(*time, Locale::FormatType::Short) == u8"12:05\u202fAM");

    auto dateTime = DateComponents::FromParsingDateTimeLocal(u8"2026-12-31T23:59");
    REQUIRE(dateTime);
    CHECK(Locale::Create(u8"en-GB")->FormatDateTime(*dateTime, Locale::FormatType::Short)
          == u8"31/12/2026, 23:59");

    // Weeks aren't formatted.
    auto week = DateComponents::FromParsingWeek(u8"2026-W10");
    REQUIRE(week);
    CHECK(english->FormatDateTime(*week).empty());
    CHECK(english->FormatDateTime(DateComponents()).empty());
  }

  TEST_CASE("LocaleICU: the default language", "[Core][LocaleICU]")
  {
    auto language = DefaultLanguage();
    CHECK(!language.empty());
    CHECK(Locale::CreateDefault()->MonthLabels().size() == 12);
  }
}
