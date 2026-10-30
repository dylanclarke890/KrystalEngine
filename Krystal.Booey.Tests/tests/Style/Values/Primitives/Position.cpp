#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  TEST_CASE_METHOD(BuilderFixture, "Style Position", "[Style][Values][Position]")
  {
    SECTION("Two components")
    {
      auto position = Convert<Position>(PropertyId::ObjectPosition, u8"left 10px top 20%");
      CHECK(Serialised(position) == "10px 20%");
      CHECK(position == Position {PositionX {10_css_px}, PositionY {20_css_percentage}});
    }

    SECTION("Keywords resolve to percentages")
    {
      CHECK(Serialised(Convert<Position>(PropertyId::ObjectPosition, u8"center")) == "50% 50%");
      CHECK(Serialised(Convert<Position>(PropertyId::ObjectPosition, u8"right bottom")) == "100% 100%");
      CHECK(Serialised(Convert<Position>(PropertyId::ObjectPosition, u8"left top")) == "0% 0%");
    }

    SECTION("Offsets from the right and bottom are reflected")
    {
      CHECK(Serialised(Convert<Position>(PropertyId::ObjectPosition, u8"right 10px bottom 20%"))
            == "calc(100% - 10px) 80%");
    }

    SECTION("Logical keywords follow the writing mode")
    {
      auto horizontal =
        ConvertToStyle(css::Position {css::FourComponentPositionHorizontalVertical {
                         css::FourComponentPositionHorizontal {{css::keywords::XStart {}, 10_css_px}},
                         css::FourComponentPositionVertical {{css::keywords::YEnd {}, 5_css_px}}}},
                       *State);
      CHECK(Serialised(horizontal) == "10px calc(100% - 5px)");

      Style.SetDirection(TextDirection::RTL);
      auto rtl =
        ConvertToStyle(css::Position {css::ThreeComponentPositionHorizontalVerticalLengthFirst {
                         css::FourComponentPositionHorizontal {{css::keywords::XStart {}, 10_css_px}},
                         css::ThreeComponentPositionVertical {css::keywords::YStart {}}}},
                       *State);
      CHECK(Serialised(rtl) == "calc(100% - 10px) 0%");
    }

    SECTION("Single axes")
    {
      CHECK(Serialised(Convert<PositionX>(PropertyId::BackgroundPositionX, u8"right 20px"))
            == "calc(100% - 20px)");
      CHECK(Serialised(Convert<PositionY>(PropertyId::BackgroundPositionY, u8"30%")) == "30%");
      CHECK(Serialised(Convert<PositionY>(PropertyId::BackgroundPositionY, u8"bottom")) == "100%");
    }

    SECTION("A non-position value is invalid at computed-value time")
    {
      CHECK(ConvertCSSValueToStyle<Position>(*State, *css::PrimitiveValue::Create(css::ValueId::Auto))
            == Position {0_css_px, 0_css_px});
      CHECK(IsInvalid());
    }

    SECTION("Creation")
    {
      CHECK(Created(Position {PositionX {10_css_px}, PositionY {20_css_percentage}}) == "10px 20%");
    }

    SECTION("Evaluation")
    {
      auto point = Evaluate<FloatPoint>(Position {PositionX {10_css_px}, PositionY {50_css_percentage}},
                                        FloatSize {200, 100}, ZoomNeeded {});
      CHECK(point == FloatPoint {10, 50});
    }

    SECTION("Construction from a point")
    {
      CHECK(Position {FloatPoint {3, 4}} == Position {3_css_px, 4_css_px});
    }
  }
}
