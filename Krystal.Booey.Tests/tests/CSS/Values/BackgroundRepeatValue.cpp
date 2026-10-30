#include "Krystal.Booey/CSS/Values/BackgroundRepeatValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "BackgroundRepeatValue::Create", "[CSS][Values][BackgroundRepeatValue]")
  {
    auto value = BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::Space);
    CHECK(value->IsBackgroundRepeatValue());
    CHECK(value->XValue() == ValueId::Repeat);
    CHECK(value->YValue() == ValueId::Space);
  }

  TEST_CASE_METHOD(ValueFixture, "BackgroundRepeatValue serialisation",
                   "[CSS][Values][BackgroundRepeatValue]")
  {
    SECTION("Identical values serialise once")
    {
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::Repeat)) == "repeat");
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::NoRepeat, ValueId::NoRepeat)) == "no-repeat");
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::Space, ValueId::Space)) == "space");
    }

    SECTION("repeat no-repeat is repeat-x, no-repeat repeat is repeat-y")
    {
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::NoRepeat)) == "repeat-x");
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::NoRepeat, ValueId::Repeat)) == "repeat-y");
    }

    SECTION("Other pairs serialise both values")
    {
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::Space)) == "repeat space");
      CHECK(CssText(*BackgroundRepeatValue::Create(ValueId::Round, ValueId::NoRepeat)) == "round no-repeat");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "BackgroundRepeatValue equality", "[CSS][Values][BackgroundRepeatValue]")
  {
    auto a = BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::Space);
    CHECK(*a == *BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::Space));
    CHECK_FALSE(*a == *BackgroundRepeatValue::Create(ValueId::Round, ValueId::Space));
    CHECK_FALSE(*a == *BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::Round));
  }
}
