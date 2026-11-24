#include "Krystal.Booey/CSS/Values/GridIntegerRepeatValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "GridIntegerRepeatValue::Create", "[CSS][Values][GridIntegerRepeatValue]")
  {
    auto value = GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(3.0), {Px(100.0), Px(200.0)});
    CHECK(Is<GridIntegerRepeatValue>(*value));
    CHECK(value->ContainsValueList());
    CHECK(CssText(value->Repetitions()) == "3");
    REQUIRE(value->size() == 2u);
    CHECK(CssText((*value)[0u]) == "100px");
    CHECK(CssText((*value)[1u]) == "200px");
  }

  TEST_CASE_METHOD(ValueFixture, "GridIntegerRepeatValue serialisation",
                   "[CSS][Values][GridIntegerRepeatValue]")
  {
    CHECK(CssText(*GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(2.0), {Px(100.0)}))
          == "repeat(2, 100px)");
    CHECK(
      CssText(*GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(4.0),
                                              {LineNames({u8"a"}), Px(100.0), LineNames({u8"b"}), Px(200.0)}))
      == "repeat(4, [a] 100px [b] 200px)");
  }

  TEST_CASE_METHOD(ValueFixture, "GridIntegerRepeatValue equality", "[CSS][Values][GridIntegerRepeatValue]")
  {
    auto a =
      GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(2.0), {LineNames({u8"a"}), Px(100.0)});
    auto b =
      GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(2.0), {LineNames({u8"a"}), Px(100.0)});
    auto otherCount =
      GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(3.0), {LineNames({u8"a"}), Px(100.0)});
    auto otherItems =
      GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(2.0), {LineNames({u8"b"}), Px(100.0)});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *otherCount);
    CHECK_FALSE(*a == *otherItems);
  }

  TEST_CASE_METHOD(ValueFixture, "GridIntegerRepeatValue visits its items, then the repetitions",
                   "[CSS][Values][GridIntegerRepeatValue]")
  {
    auto value =
      GridIntegerRepeatValue::Create(PrimitiveValue::CreateInteger(2.0), {LineNames({u8"a"}), Px(100.0)});
    auto children = Children(*value);
    REQUIRE(children.size() == 3uz);
    CHECK(children[0uz] == &(*value)[0u]);
    CHECK(children[1uz] == &(*value)[1u]);
    CHECK(children[2uz] == &value->Repetitions());
  }
}
