#include "Krystal.Web/Css/Properties/Consumers/Scrollbars.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ConsumerTestUtils.hpp"

namespace krys::web::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("parse_property(ConsumeScrollbarColor)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeScrollbarColor) == "auto");
    CHECK(ConsumeToText(u8"red blue", ConsumeScrollbarColor) == "red blue");

    // The pair doesn't coalesce, even when the colours match.
    CHECK(ConsumeToText(u8"red red", ConsumeScrollbarColor) == "red red");

    CHECK(ConsumeToText(u8"red", ConsumeScrollbarColor) == "<null>");
    CHECK(ConsumeToText(u8"auto red", ConsumeScrollbarColor) == "<partial>");
  }
}
