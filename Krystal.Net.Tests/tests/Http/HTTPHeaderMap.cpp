#include "Krystal.Net/Http/HTTPHeaderMap.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::net::http::tests
{
  TEST_CASE("HTTPHeaderMap: names ignore case, and values given twice are joined", "[Http][HTTPHeaderMap]")
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
}
