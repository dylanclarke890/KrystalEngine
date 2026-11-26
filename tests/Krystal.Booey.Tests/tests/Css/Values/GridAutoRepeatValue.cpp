#include "Krystal.Booey/Css/Values/GridAutoRepeatValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Booey/Css/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue::Create", "[CSS][Values][GridAutoRepeatValue]")
  {
    auto fill = GridAutoRepeatValue::Create(ValueId::AutoFill, {Px(100.0)});
    CHECK(Is<GridAutoRepeatValue>(*fill));
    CHECK(fill->ContainsValueList());
    CHECK(fill->AutoRepeatID() == ValueId::AutoFill);
    REQUIRE(fill->size() == 1u);
    CHECK(CssText((*fill)[0u]) == "100px");

    auto fit = GridAutoRepeatValue::Create(ValueId::AutoFit, {Px(100.0)});
    CHECK(fit->AutoRepeatID() == ValueId::AutoFit);
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue serialisation", "[CSS][Values][GridAutoRepeatValue]")
  {
    CHECK(CssText(*GridAutoRepeatValue::Create(ValueId::AutoFill, {Px(100.0)}))
          == "repeat(auto-fill, 100px)");
    CHECK(CssText(*GridAutoRepeatValue::Create(ValueId::AutoFit,
                                               {LineNames({u8"a"}), Px(100.0), LineNames({u8"b"})}))
          == "repeat(auto-fit, [a] 100px [b])");
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue equality", "[CSS][Values][GridAutoRepeatValue]")
  {
    auto a = GridAutoRepeatValue::Create(ValueId::AutoFill, {LineNames({u8"a"}), Px(100.0)});
    auto b = GridAutoRepeatValue::Create(ValueId::AutoFill, {LineNames({u8"a"}), Px(100.0)});
    auto fit = GridAutoRepeatValue::Create(ValueId::AutoFit, {LineNames({u8"a"}), Px(100.0)});
    auto otherItems = GridAutoRepeatValue::Create(ValueId::AutoFill, {LineNames({u8"a"}), Px(200.0)});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *fit);
    CHECK_FALSE(*a == *otherItems);
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue visits its items", "[CSS][Values][GridAutoRepeatValue]")
  {
    auto value = GridAutoRepeatValue::Create(ValueId::AutoFill, {LineNames({u8"a"}), Px(100.0)});
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == &(*value)[0u]);
    CHECK(children[1uz] == &(*value)[1u]);
  }
}
