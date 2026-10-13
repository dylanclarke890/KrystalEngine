#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Position TwoComponent(TwoComponentPositionHorizontal x,
                                         TwoComponentPositionVertical y) noexcept
    {
      return Position {TwoComponentPositionHorizontalVertical {krys::move(x), krys::move(y)}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PositionValue", "[CSS][Values][PositionValue]")
  {
    const auto position = TwoComponent({keywords::Left {}}, {LengthPercentage<> {10_css_px}});
    auto value = PositionValue::Create(position);
    CHECK(value->IsPositionValue());
    CHECK_FALSE(value->IsPositionXValue());
    CHECK_FALSE(value->IsPositionYValue());
    CHECK(value->GetPosition() == position);
    CHECK(CssText(*value) == "left 10px");

    CHECK(*value == *PositionValue::Create(position));
    CHECK_FALSE(
      *value == *PositionValue::Create(TwoComponent({keywords::Right {}}, {LengthPercentage<> {10_css_px}})));

    CHECK(Children(*value).empty());
  }

  TEST_CASE_METHOD(ValueFixture, "PositionXValue", "[CSS][Values][PositionValue]")
  {
    const PositionX position {
      FourComponentPositionHorizontal {{keywords::Right {}, LengthPercentage<> {5_css_px}}}};
    auto value = PositionXValue::Create(position);
    CHECK(value->IsPositionXValue());
    CHECK_FALSE(value->IsPositionValue());
    CHECK(value->GetPosition() == position);
    CHECK(CssText(*value) == "right 5px");
    CHECK(CssText(*PositionXValue::Create(PositionX {TwoComponentPositionHorizontal {keywords::XStart {}}}))
          == "x-start");

    CHECK(*value == *PositionXValue::Create(position));
    CHECK_FALSE(*value
                == *PositionXValue::Create(PositionX {TwoComponentPositionHorizontal {keywords::Center {}}}));

    CHECK(Children(*value).empty());
  }

  TEST_CASE_METHOD(ValueFixture, "PositionYValue", "[CSS][Values][PositionValue]")
  {
    const PositionY position {
      FourComponentPositionVertical {{keywords::Top {}, LengthPercentage<> {1_css_em}}}};
    auto value = PositionYValue::Create(position);
    CHECK(value->IsPositionYValue());
    CHECK_FALSE(value->IsPositionValue());
    CHECK(value->GetPosition() == position);
    CHECK(CssText(*value) == "top 1em");
    CHECK(CssText(*PositionYValue::Create(PositionY {TwoComponentPositionVertical {keywords::Center {}}}))
          == "center");

    CHECK(*value == *PositionYValue::Create(position));
    CHECK_FALSE(*value
                == *PositionYValue::Create(PositionY {TwoComponentPositionVertical {keywords::Bottom {}}}));

    CHECK(Children(*value).empty());
  }

  TEST_CASE_METHOD(ValueFixture, "PositionValue types don't compare equal to each other",
                   "[CSS][Values][PositionValue]")
  {
    auto x = PositionXValue::Create(PositionX {TwoComponentPositionHorizontal {keywords::Center {}}});
    auto y = PositionYValue::Create(PositionY {TwoComponentPositionVertical {keywords::Center {}}});
    CHECK_FALSE(*x == *y);
  }
}
