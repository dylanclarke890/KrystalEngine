#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue::Create", "[CSS][Values][GridLineNamesValue]")
  {
    auto value = LineNames({u8"header-start", u8"main-start"});
    CHECK(value->IsGridLineNamesValue());
    REQUIRE(value->Names().size() == 2uz);
    CHECK(value->Names()[0uz] == u8"header-start");
    CHECK(value->Names()[1uz] == u8"main-start");
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue serialisation", "[CSS][Values][GridLineNamesValue]")
  {
    CHECK(CssText(*LineNames({})) == "[]");
    CHECK(CssText(*LineNames({u8"a"})) == "[a]");
    CHECK(CssText(*LineNames({u8"header-start", u8"main-start"})) == "[header-start main-start]");

    SECTION("Names are serialised as identifiers")
    {
      CHECK(CssText(*LineNames({u8"1a", u8"a b"})) == "[\\31 a a\\ b]");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue equality", "[CSS][Values][GridLineNamesValue]")
  {
    CHECK(*LineNames({u8"a", u8"b"}) == *LineNames({u8"a", u8"b"}));
    CHECK(*LineNames({}) == *LineNames({}));
    CHECK_FALSE(*LineNames({u8"a", u8"b"}) == *LineNames({u8"b", u8"a"}));
    CHECK_FALSE(*LineNames({u8"a", u8"b"}) == *LineNames({u8"a"}));
    CHECK_FALSE(*LineNames({u8"a"}) == *LineNames({}));
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue has no value children",
                   "[CSS][Values][GridLineNamesValue]")
  {
    CHECK(Children(*LineNames({u8"a"})).empty());
  }
}
