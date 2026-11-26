#include "Krystal.Web/Css/Values/BorderImageSliceValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<PrimitiveValue> Num(double value) noexcept
    {
      return PrimitiveValue::Create(value, UnitType::Number);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "BorderImageSliceValue::Create", "[CSS][Values][BorderImageSliceValue]")
  {
    auto value = BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), true);
    CHECK(Is<BorderImageSliceValue>(*value));
    CHECK(value->Fill());
    CHECK(CssText(*value->Slices().Top) == "10");
    CHECK(CssText(*value->Slices().Left) == "10");
    CHECK_FALSE(BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), false)->Fill());
  }

  TEST_CASE_METHOD(ValueFixture, "BorderImageSliceValue serialisation",
                   "[CSS][Values][BorderImageSliceValue]")
  {
    CHECK(CssText(*BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), false)) == "10");
    CHECK(CssText(*BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), true)) == "10 fill");
    CHECK(CssText(*BorderImageSliceValue::Create(
            SideValues {Num(1.0), PrimitiveValue::Create(20.0, UnitType::Percentage), Num(3.0),
                        PrimitiveValue::Create(20.0, UnitType::Percentage)},
            true))
          == "1 20% 3 fill");
  }

  TEST_CASE_METHOD(ValueFixture, "BorderImageSliceValue equality", "[CSS][Values][BorderImageSliceValue]")
  {
    auto a = BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), true);
    CHECK(*a == *BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), true));
    CHECK_FALSE(*a == *BorderImageSliceValue::Create(SideValues::Uniform(Num(10.0)), false));
    CHECK_FALSE(*a == *BorderImageSliceValue::Create(SideValues::Uniform(Num(20.0)), true));
  }
}
