#include "Krystal.Http/HTTPParsers.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::http::tests
{
  namespace
  {
    using namespace std::chrono;

    /// @brief 1994-11-06 08:49:37 UTC, the RFC's example.
    constexpr sys_seconds RFCExample = sys_days(1'994y / November / 6) + 8h + 49min + 37s;
  }

  TEST_CASE("ParseHTTPDate: the three HTTP-date forms", "[Http][HTTPParsers]")
  {
    CHECK(ParseHTTPDate(u8"Sun, 06 Nov 1994 08:49:37 GMT") == RFCExample);
    CHECK(ParseHTTPDate(u8"Sunday, 06-Nov-94 08:49:37 GMT") == RFCExample);
    CHECK(ParseHTTPDate(u8"Sun Nov  6 08:49:37 1994") == RFCExample);
    CHECK(ParseHTTPDate(u8"Sun Nov 16 08:49:37 1994") == RFCExample + days(10));
    // The day name isn't checked against the date.
    CHECK(ParseHTTPDate(u8"Mon, 06 Nov 1994 08:49:37 GMT") == RFCExample);
    CHECK(ParseHTTPDate(u8"Thu, 29 Feb 2024 23:59:59 GMT")
          == sys_days(2'024y / February / 29) + 23h + 59min + 59s);
  }

  TEST_CASE("ParseHTTPDate: anything else is no date", "[Http][HTTPParsers]")
  {
    for (auto value : {u8"", u8"Sun, 06 Nov 1994 08:49:37", u8"Sun, 06 Nov 1994 08:49:37 UTC",
                       u8"Sun, 6 Nov 1994 08:49:37 GMT", u8"Sun, 06 Foo 1994 08:49:37 GMT",
                       u8"Sun, 31 Nov 1994 08:49:37 GMT", u8"Sun, 06 Nov 1994 24:00:00 GMT",
                       u8"Sun, 06 Nov 1994 08:49:37 GMT trailing", u8"1994-11-06T08:49:37Z",
                       u8"Fri, 29 Feb 2023 00:00:00 GMT"})
    {
      INFO(reinterpret_cast<const char *>(value));
      CHECK_FALSE(ParseHTTPDate(value));
    }
  }

  TEST_CASE("ParseHTTPDate: an RFC 850 year more than 50 years ahead is in the past century",
            "[Http][HTTPParsers]")
  {
    auto year = static_cast<int32>(year_month_day(floor<days>(system_clock::now())).year());
    auto parsed = ParseHTTPDate(u8"Sunday, 06-Nov-94 08:49:37 GMT");
    REQUIRE(parsed);
    auto parsedYear = static_cast<int32>(year_month_day(floor<days>(*parsed)).year());
    CHECK(parsedYear % 100 == 94);
    CHECK(parsedYear <= year + 50);
    CHECK(parsedYear > year - 50);
  }

  TEST_CASE("IsNosniffContentTypeOptions: the first value is nosniff", "[Http][HTTPParsers]")
  {
    CHECK(IsNosniffContentTypeOptions(u8"nosniff"));
    CHECK(IsNosniffContentTypeOptions(u8" NoSniff "));
    CHECK(IsNosniffContentTypeOptions(u8"nosniff, other"));
    CHECK_FALSE(IsNosniffContentTypeOptions(u8"other, nosniff"));
    CHECK_FALSE(IsNosniffContentTypeOptions(u8""));
    CHECK_FALSE(IsNosniffContentTypeOptions(u8"nosniffing"));
  }
}
