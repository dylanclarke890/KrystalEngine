#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE_METHOD(ValueFixture, "QuadValue::Create", "[CSS][Values][QuadValue]")
  {
    SECTION("One value for all four sides")
    {
      auto quad = QuadValue::Create(Px(1.0));
      CHECK(Is<QuadValue>(*quad));
      CHECK(CssText(*quad->Sides().Top) == "1px");
      CHECK(CssText(*quad->Sides().Right) == "1px");
      CHECK(CssText(*quad->Sides().Bottom) == "1px");
      CHECK(CssText(*quad->Sides().Left) == "1px");
      CHECK(CssText(*quad) == "1px");
      CHECK(quad->CanBeCoalesced());
    }

    SECTION("Vertical and horizontal")
    {
      auto quad = QuadValue::Create(Px(1.0), Px(2.0));
      CHECK(CssText(*quad->Sides().Top) == "1px");
      CHECK(CssText(*quad->Sides().Right) == "2px");
      CHECK(CssText(*quad->Sides().Bottom) == "1px");
      CHECK(CssText(*quad->Sides().Left) == "2px");
      CHECK(CssText(*quad) == "1px 2px");
      CHECK_FALSE(quad->CanBeCoalesced());
    }

    SECTION("Top, horizontal and bottom")
    {
      auto quad = QuadValue::Create(Px(1.0), Px(2.0), Px(3.0));
      CHECK(CssText(*quad->Sides().Left) == "2px");
      CHECK(CssText(*quad) == "1px 2px 3px");
    }

    SECTION("Four sides")
    {
      auto quad = QuadValue::Create(Px(1.0), Px(2.0), Px(3.0), Px(4.0));
      CHECK(CssText(*quad->Sides().Top) == "1px");
      CHECK(CssText(*quad->Sides().Right) == "2px");
      CHECK(CssText(*quad->Sides().Bottom) == "3px");
      CHECK(CssText(*quad->Sides().Left) == "4px");
      CHECK(CssText(*quad) == "1px 2px 3px 4px");
    }

    SECTION("From side values")
    {
      auto quad = QuadValue::Create(SideValues {Px(1.0), Px(2.0), Px(1.0), Px(2.0)});
      CHECK(CssText(*quad) == "1px 2px");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "QuadValue serialisation omits repeated sides", "[CSS][Values][QuadValue]")
  {
    CHECK(CssText(*QuadValue::Create(Px(1.0), Px(1.0), Px(1.0), Px(1.0))) == "1px");
    CHECK(CssText(*QuadValue::Create(Px(1.0), Px(2.0), Px(1.0), Px(2.0))) == "1px 2px");
    CHECK(CssText(*QuadValue::Create(Px(1.0), Px(2.0), Px(3.0), Px(2.0))) == "1px 2px 3px");
    CHECK(CssText(*QuadValue::Create(Px(1.0), Px(1.0), Px(3.0), Px(1.0))) == "1px 1px 3px");
    CHECK(CssText(*QuadValue::Create(Px(1.0), Px(1.0), Px(1.0), Px(2.0))) == "1px 1px 1px 2px");
  }

  TEST_CASE_METHOD(ValueFixture, "QuadValue::CanBeCoalesced", "[CSS][Values][QuadValue]")
  {
    CHECK(QuadValue::Create(Px(1.0), Px(1.0), Px(1.0), Px(1.0))->CanBeCoalesced());
    CHECK_FALSE(QuadValue::Create(Px(2.0), Px(1.0), Px(1.0), Px(1.0))->CanBeCoalesced());
    CHECK_FALSE(QuadValue::Create(Px(1.0), Px(2.0), Px(1.0), Px(1.0))->CanBeCoalesced());
    CHECK_FALSE(QuadValue::Create(Px(1.0), Px(1.0), Px(2.0), Px(1.0))->CanBeCoalesced());
    CHECK_FALSE(QuadValue::Create(Px(1.0), Px(1.0), Px(1.0), Px(2.0))->CanBeCoalesced());
  }

  TEST_CASE_METHOD(ValueFixture, "QuadValue equality", "[CSS][Values][QuadValue]")
  {
    auto a = QuadValue::Create(Px(1.0), Px(2.0), Px(3.0), Px(4.0));
    CHECK(*a == *QuadValue::Create(Px(1.0), Px(2.0), Px(3.0), Px(4.0)));
    CHECK_FALSE(*a == *QuadValue::Create(Px(9.0), Px(2.0), Px(3.0), Px(4.0)));
    CHECK_FALSE(*a == *QuadValue::Create(Px(1.0), Px(9.0), Px(3.0), Px(4.0)));
    CHECK_FALSE(*a == *QuadValue::Create(Px(1.0), Px(2.0), Px(9.0), Px(4.0)));
    CHECK_FALSE(*a == *QuadValue::Create(Px(1.0), Px(2.0), Px(3.0), Px(9.0)));
  }
}
