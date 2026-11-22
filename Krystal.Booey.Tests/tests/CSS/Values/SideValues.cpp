#include "Krystal.Booey/CSS/Values/SideValues.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("SideValues: serialising the fewest sides", "[CSS][Values][SideValues]")
  {
    SECTION("Four equal sides are one value")
    {
      CHECK(SerialiseSides(u8"1px", u8"1px", u8"1px", u8"1px") == u8"1px");
    }

    SECTION("Equal top and bottom, and equal right and left, are two values")
    {
      CHECK(SerialiseSides(u8"1px", u8"2px", u8"1px", u8"2px") == u8"1px 2px");
    }

    SECTION("Equal right and left are three values")
    {
      CHECK(SerialiseSides(u8"1px", u8"2px", u8"3px", u8"2px") == u8"1px 2px 3px");
      // The right side is kept even when it's the top's, as the bottom differs.
      CHECK(SerialiseSides(u8"1px", u8"1px", u8"2px", u8"1px") == u8"1px 1px 2px");
    }

    SECTION("A left side that isn't the right side's keeps all four")
    {
      CHECK(SerialiseSides(u8"1px", u8"2px", u8"3px", u8"4px") == u8"1px 2px 3px 4px");
      CHECK(SerialiseSides(u8"1px", u8"1px", u8"1px", u8"2px") == u8"1px 1px 1px 2px");
      CHECK(SerialiseSides(u8"1px", u8"2px", u8"1px", u8"3px") == u8"1px 2px 1px 3px");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "SideValues: values", "[CSS][Values][SideValues]")
  {
    SideValues sides {Px(1.0), Px(2.0), Px(1.0), Px(2.0)};
    auto &context = DefaultSerialisationContext();

    CHECK(SerialiseSides(sides, context) == u8"1px 2px");
    CHECK(SerialiseRect(sides, context) == u8"rect(1px, 2px, 1px, 2px)");

    CHECK(sides == SideValues {Px(1.0), Px(2.0), Px(1.0), Px(2.0)});
    CHECK_FALSE(sides == SideValues {Px(1.0), Px(2.0), Px(1.0), Px(3.0)});
    CHECK_FALSE(sides.AreAllEqual());

    auto uniform = SideValues::Uniform(Px(5.0));
    CHECK(uniform.AreAllEqual());
    CHECK(uniform.Top.get() == uniform.Left.get());
    CHECK(SerialiseSides(uniform, context) == u8"5px");
    // rect() always lists all four sides.
    CHECK(SerialiseRect(uniform, context) == u8"rect(5px, 5px, 5px, 5px)");
  }
}
