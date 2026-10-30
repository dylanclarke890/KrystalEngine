#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<GridLineNamesValue> Names(List<CSSOMString> names)
    {
      return GridLineNamesValue::Create(names);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue::Create", "[CSS][Values][GridLineNamesValue]")
  {
    auto value = Names({u8"header-start", u8"main-start"});
    CHECK(value->IsGridLineNamesValue());
    REQUIRE(value->Names().size() == 2uz);
    CHECK(value->Names()[0uz] == u8"header-start");
    CHECK(value->Names()[1uz] == u8"main-start");
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue serialisation", "[CSS][Values][GridLineNamesValue]")
  {
    CHECK(CssText(*Names({})) == "[]");
    CHECK(CssText(*Names({u8"a"})) == "[a]");
    CHECK(CssText(*Names({u8"header-start", u8"main-start"})) == "[header-start main-start]");

    SECTION("Names are serialised as identifiers")
    {
      CHECK(CssText(*Names({u8"1a", u8"a b"})) == "[\\31 a a\\ b]");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue equality", "[CSS][Values][GridLineNamesValue]")
  {
    CHECK(*Names({u8"a", u8"b"}) == *Names({u8"a", u8"b"}));
    CHECK(*Names({}) == *Names({}));
    CHECK_FALSE(*Names({u8"a", u8"b"}) == *Names({u8"b", u8"a"}));
    CHECK_FALSE(*Names({u8"a", u8"b"}) == *Names({u8"a"}));
    CHECK_FALSE(*Names({u8"a"}) == *Names({}));
  }

  TEST_CASE_METHOD(ValueFixture, "GridLineNamesValue has no value children",
                   "[CSS][Values][GridLineNamesValue]")
  {
    CHECK(Children(*Names({u8"a"})).empty());
  }
}
