#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<PrimitiveValue> Degrees(double value) noexcept
    {
      return PrimitiveValue::Create(value, UnitType::deg);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "OffsetRotateValue::Create", "[CSS][Values][OffsetRotateValue]")
  {
    SECTION("Modifier and angle")
    {
      auto modifier = PrimitiveValue::Create(ValueId::Auto);
      auto angle = Degrees(45.0);
      auto value = OffsetRotateValue::Create(modifier, angle);
      CHECK(value->IsOffsetRotateValue());
      CHECK(value->Modifier() == modifier.get());
      CHECK(value->GetAngle() == angle.get());
    }

    SECTION("Modifier only")
    {
      auto value = OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Reverse), nullptr);
      REQUIRE(value->Modifier());
      CHECK(value->Modifier()->ValueId() == ValueId::Reverse);
      CHECK_FALSE(value->GetAngle());
    }

    SECTION("Angle only")
    {
      auto value = OffsetRotateValue::Create(nullptr, Degrees(90.0));
      CHECK_FALSE(value->Modifier());
      REQUIRE(value->GetAngle());
      CHECK(CssText(*value->GetAngle()) == "90deg");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "OffsetRotateValue serialisation", "[CSS][Values][OffsetRotateValue]")
  {
    CHECK(CssText(*OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), nullptr)) == "auto");
    CHECK(CssText(*OffsetRotateValue::Create(nullptr, Degrees(45.0))) == "45deg");
    CHECK(CssText(*OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Reverse), Degrees(45.0)))
          == "reverse 45deg");
  }

  TEST_CASE_METHOD(ValueFixture, "OffsetRotateValue::IsInitialValue", "[CSS][Values][OffsetRotateValue]")
  {
    CHECK(OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), nullptr)->IsInitialValue());
    CHECK(OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), Degrees(0.0))->IsInitialValue());
    CHECK_FALSE(
      OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), Degrees(45.0))->IsInitialValue());
    CHECK_FALSE(
      OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Reverse), nullptr)->IsInitialValue());
    CHECK_FALSE(OffsetRotateValue::Create(nullptr, Degrees(0.0))->IsInitialValue());
  }

  TEST_CASE_METHOD(ValueFixture, "OffsetRotateValue equality", "[CSS][Values][OffsetRotateValue]")
  {
    auto a = OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), Degrees(45.0));
    CHECK(*a == *OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), Degrees(45.0)));
    CHECK_FALSE(*a == *OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Reverse), Degrees(45.0)));
    CHECK_FALSE(*a == *OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), Degrees(90.0)));
    CHECK_FALSE(*a == *OffsetRotateValue::Create(PrimitiveValue::Create(ValueId::Auto), nullptr));
    CHECK_FALSE(*a == *OffsetRotateValue::Create(nullptr, Degrees(45.0)));
  }

  TEST_CASE_METHOD(ValueFixture, "OffsetRotateValue visits its modifier and angle",
                   "[CSS][Values][OffsetRotateValue]")
  {
    auto modifier = PrimitiveValue::Create(ValueId::Auto);
    auto angle = Degrees(45.0);

    auto both = Children(*OffsetRotateValue::Create(modifier, angle));
    REQUIRE(both.size() == 2uz);
    CHECK(both[0uz] == modifier.get());
    CHECK(both[1uz] == angle.get());

    auto angleOnly = Children(*OffsetRotateValue::Create(nullptr, angle));
    REQUIRE(angleOnly.size() == 1uz);
    CHECK(angleOnly[0uz] == angle.get());
  }
}
