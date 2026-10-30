#include "Krystal.Booey/CSS/Values/GridAutoRepeatValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Ref<Value> Names(List<CSSOMString> names)
    {
      return GridLineNamesValue::Create(names);
    }

    Ref<Value> Px(double value)
    {
      return PrimitiveValue::Create(value, UnitType::px);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue::Create", "[CSS][Values][GridAutoRepeatValue]")
  {
    auto fill = GridAutoRepeatValue::Create(ValueId::AutoFill, {Px(100.0)});
    CHECK(fill->IsGridAutoRepeatValue());
    CHECK(fill->ContainsValueList());
    CHECK(fill->AutoRepeatID() == ValueId::AutoFill);
    REQUIRE(fill->Length() == 1u);
    CHECK(CssText(*fill->Item(0u)) == "100px");

    auto fit = GridAutoRepeatValue::Create(ValueId::AutoFit, {Px(100.0)});
    CHECK(fit->AutoRepeatID() == ValueId::AutoFit);
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue serialisation", "[CSS][Values][GridAutoRepeatValue]")
  {
    CHECK(CssText(*GridAutoRepeatValue::Create(ValueId::AutoFill, {Px(100.0)}))
          == "repeat(auto-fill, 100px)");
    CHECK(CssText(*GridAutoRepeatValue::Create(ValueId::AutoFit, {Names({u8"a"}), Px(100.0), Names({u8"b"})}))
          == "repeat(auto-fit, [a] 100px [b])");
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue equality", "[CSS][Values][GridAutoRepeatValue]")
  {
    auto a = GridAutoRepeatValue::Create(ValueId::AutoFill, {Names({u8"a"}), Px(100.0)});
    auto b = GridAutoRepeatValue::Create(ValueId::AutoFill, {Names({u8"a"}), Px(100.0)});
    auto fit = GridAutoRepeatValue::Create(ValueId::AutoFit, {Names({u8"a"}), Px(100.0)});
    auto otherItems = GridAutoRepeatValue::Create(ValueId::AutoFill, {Names({u8"a"}), Px(200.0)});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *fit);
    CHECK_FALSE(*a == *otherItems);
  }

  TEST_CASE_METHOD(ValueFixture, "GridAutoRepeatValue visits its items", "[CSS][Values][GridAutoRepeatValue]")
  {
    auto value = GridAutoRepeatValue::Create(ValueId::AutoFill, {Names({u8"a"}), Px(100.0)});
    auto children = Children(*value);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == value->Item(0u));
    CHECK(children[1uz] == value->Item(1u));
  }
}
