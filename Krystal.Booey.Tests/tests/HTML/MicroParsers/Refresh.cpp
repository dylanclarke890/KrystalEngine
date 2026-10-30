#include "Krystal.Booey/HTML/MicroParsers/Refresh.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("ParseRefresh", "[HTML][MicroParsers]")
  {
    auto parse = [](dom::DOMStringView input)
    {
      return ParseRefresh(input);
    };

    auto delayOnly = parse(u8" 5");
    REQUIRE(delayOnly);
    CHECK(delayOnly->Delay == 5);
    CHECK(delayOnly->URL.empty());

    // The time's fraction is skipped.
    auto fraction = parse(u8"2.75");
    REQUIRE(fraction);
    CHECK(fraction->Delay == 2);
    REQUIRE(parse(u8".5"));
    CHECK(parse(u8".5")->Delay == 0);

    auto withURL = parse(u8"3; url=next.html");
    REQUIRE(withURL);
    CHECK(withURL->Delay == 3);
    CHECK(withURL->URL == u8"next.html");

    CHECK(parse(u8"0,URL = 'a b.html' trailing")->URL == u8"a b.html");
    CHECK(parse(u8"0;\"quoted.html\"")->URL == u8"quoted.html");
    CHECK(parse(u8"1 next.html")->URL == u8"next.html");
    // A URL that starts with u but isn't "url=" is the URL.
    CHECK(parse(u8"1; urgent.html")->URL == u8"urgent.html");

    CHECK_FALSE(parse(u8""));
    CHECK_FALSE(parse(u8"soon"));
    CHECK_FALSE(parse(u8"5x; url=next.html"));
  }
}
