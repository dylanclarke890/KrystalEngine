#include "Krystal.Booey/Css/Properties/Consumers/WillChange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ConsumerTestUtils.hpp"

namespace krys::boo::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("parse_property(ConsumeWillChange)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeWillChange) == "auto");
    CHECK(ConsumeToText(u8"contents", ConsumeWillChange) == "contents");
    CHECK(ConsumeToText(u8"scroll-position, opacity", ConsumeWillChange) == "scroll-position, opacity");
    CHECK(ConsumeToText(u8"Transform", ConsumeWillChange) == "transform");

    // Unknown identifiers are kept.
    CHECK(ConsumeToText(u8"opacity, not-a-property", ConsumeWillChange) == "opacity, not-a-property");

    CHECK(ConsumeToText(u8"will-change", ConsumeWillChange) == "<null>");
    CHECK(ConsumeToText(u8"opacity, auto", ConsumeWillChange) == "<null>");
    CHECK(ConsumeToText(u8"none", ConsumeWillChange) == "<null>");
    CHECK(ConsumeToText(u8"all", ConsumeWillChange) == "<null>");
    CHECK(ConsumeToText(u8"opacity transform", ConsumeWillChange) == "<null>");
    CHECK(ConsumeToText(u8"opacity, 5", ConsumeWillChange) == "<null>");
    CHECK(ConsumeToText(u8"inherit", ConsumeWillChange) == "<null>");
  }
}
