#include "Krystal.Booey/Platform/Network/HTTPParsers.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Platform/Network/HTTPHeaderMap.hpp"
#include "Krystal.Booey/Platform/Network/ResourceResponse.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono;

    /// @brief 1994-11-06 08:49:37 UTC, the RFC's example.
    constexpr sys_seconds RFCExample = sys_days(1'994y / November / 6) + 8h + 49min + 37s;
  }

  TEST_CASE("ParseHTTPDate: the three HTTP-date forms", "[Platform][Network][HTTPParsers]")
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

  TEST_CASE("ParseHTTPDate: anything else is no date", "[Platform][Network][HTTPParsers]")
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
            "[Platform][Network][HTTPParsers]")
  {
    auto year = static_cast<int32>(year_month_day(floor<days>(system_clock::now())).year());
    auto parsed = ParseHTTPDate(u8"Sunday, 06-Nov-94 08:49:37 GMT");
    REQUIRE(parsed);
    auto parsedYear = static_cast<int32>(year_month_day(floor<days>(*parsed)).year());
    CHECK(parsedYear % 100 == 94);
    CHECK(parsedYear <= year + 50);
    CHECK(parsedYear > year - 50);
  }

  TEST_CASE("IsNosniffContentTypeOptions: the first value is nosniff", "[Platform][Network][HTTPParsers]")
  {
    CHECK(IsNosniffContentTypeOptions(u8"nosniff"));
    CHECK(IsNosniffContentTypeOptions(u8" NoSniff "));
    CHECK(IsNosniffContentTypeOptions(u8"nosniff, other"));
    CHECK_FALSE(IsNosniffContentTypeOptions(u8"other, nosniff"));
    CHECK_FALSE(IsNosniffContentTypeOptions(u8""));
    CHECK_FALSE(IsNosniffContentTypeOptions(u8"nosniffing"));
  }

  TEST_CASE("HTTPHeaderMap: names ignore case, and values given twice are joined",
            "[Platform][Network][HTTPHeaderMap]")
  {
    HTTPHeaderMap headers;
    headers.Add(u8"Content-Language", u8"en");
    headers.Add(u8"content-language", u8"fr");
    CHECK(headers.Get(u8"CONTENT-LANGUAGE") == utf8_stringview(u8"en, fr"));
    CHECK(headers.Size() == 1);
    CHECK(headers.begin()->first == u8"Content-Language");

    headers.Set(u8"CONTENT-language", u8"de");
    CHECK(headers.Get(u8"Content-Language") == utf8_stringview(u8"de"));
    CHECK_FALSE(headers.Contains(u8"Last-Modified"));
    headers.Remove(u8"content-LANGUAGE");
    CHECK(headers.IsEmpty());
  }

  TEST_CASE("ResourceResponse: a status, headers and the Last-Modified date",
            "[Platform][Network][ResourceResponse]")
  {
    ResourceResponse response(url::URL(u8"https://example.com/"), u8"text/html", 0);
    CHECK(response.HTTPStatusCode() == 0);
    CHECK_FALSE(response.IsSuccessful());
    CHECK_FALSE(response.LastModified());
    CHECK(response.HTTPHeaderField(u8"Last-Modified").empty());

    response.SetHTTPStatusCode(204);
    CHECK(response.IsSuccessful());
    response.SetHTTPStatusCode(404);
    CHECK_FALSE(response.IsSuccessful());

    response.SetHTTPHeaderField(u8"last-modified", u8"Sun, 06 Nov 1994 08:49:37 GMT");
    CHECK(response.LastModified() == RFCExample);
    response.SetHTTPHeaderField(u8"Last-Modified", u8"yesterday");
    CHECK_FALSE(response.LastModified());
  }
}
