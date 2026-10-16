#include "Krystal.Booey/CSS/Properties/Consumers/Position.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PositionValue.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::CssText;
  using css::tests::Tokens;

  namespace
  {
    // `<bg-position>` has no value-returning consumer, so wrap the unresolved one.
    constexpr auto ConsumeBackgroundPosition = [](TokenRange &tokens,
                                                  PropertyParserState &state) -> RefPtr<Value>
    {
      if (auto position = ConsumeBackgroundPositionUnresolved(tokens, state))
      {
        return PositionValue::Create(krys::move(*position));
      }
      return nullptr;
    };

    constexpr auto ConsumeOneOrTwoComponentPosition = [](TokenRange &tokens,
                                                         PropertyParserState &state) -> RefPtr<Value>
    {
      if (auto position = ConsumeOneOrTwoComponentPositionUnresolved(tokens, state))
      {
        return PositionValue::Create(krys::move(*position));
      }
      return nullptr;
    };
  }

  TEST_CASE("PropertyParserHelpers(ConsumePosition)", "[CSS][Properties][Consumers]")
  {
    // One component: the other axis is center.
    CHECK(ConsumeToText(u8"left", ConsumePosition) == "left center");
    CHECK(ConsumeToText(u8"top", ConsumePosition) == "center top");
    CHECK(ConsumeToText(u8"center", ConsumePosition) == "center center");
    CHECK(ConsumeToText(u8"10px", ConsumePosition) == "10px center");

    // Two components: keywords may come in either order; lengths are horizontal then vertical.
    CHECK(ConsumeToText(u8"left top", ConsumePosition) == "left top");
    CHECK(ConsumeToText(u8"top left", ConsumePosition) == "left top");
    CHECK(ConsumeToText(u8"bottom center", ConsumePosition) == "center bottom");
    CHECK(ConsumeToText(u8"10px 20%", ConsumePosition) == "10px 20%");
    CHECK(ConsumeToText(u8"left 20%", ConsumePosition) == "left 20%");
    CHECK(ConsumeToText(u8"center 5px", ConsumePosition) == "center 5px");

    // Four components.
    CHECK(ConsumeToText(u8"left 10px top 20px", ConsumePosition) == "left 10px top 20px");
    CHECK(ConsumeToText(u8"bottom 5% right 1em", ConsumePosition) == "right 1em bottom 5%");

    CHECK(ConsumeToText(u8"left right", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"top 10px", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"10px left", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"left 10px top", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"center 10px top 20px", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"left 10px right 20px", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"auto", ConsumePosition) == "<null>");
    CHECK(ConsumeToText(u8"left top 5px 5px", ConsumePosition) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeBackgroundPositionUnresolved)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"right top", ConsumeBackgroundPosition) == "right top");

    // Three components are allowed for <bg-position>.
    CHECK(ConsumeToText(u8"left 10px top", ConsumeBackgroundPosition) == "left 10px top");
    CHECK(ConsumeToText(u8"left 10px center", ConsumeBackgroundPosition) == "left 10px center");
    CHECK(ConsumeToText(u8"top left 10px", ConsumeBackgroundPosition) == "left 10px top");
    CHECK(ConsumeToText(u8"center bottom 10px", ConsumeBackgroundPosition) == "center bottom 10px");
    CHECK(ConsumeToText(u8"center right 10px", ConsumeBackgroundPosition) == "right 10px center");
    CHECK(ConsumeToText(u8"bottom 10px center", ConsumeBackgroundPosition) == "center bottom 10px");
    CHECK(ConsumeToText(u8"left 10px top 5px", ConsumeBackgroundPosition) == "left 10px top 5px");

    CHECK(ConsumeToText(u8"10px left top", ConsumeBackgroundPosition) == "<null>");
    CHECK(ConsumeToText(u8"left top 10px 10px", ConsumeBackgroundPosition) == "<null>");
    CHECK(ConsumeToText(u8"center center 10px", ConsumeBackgroundPosition) == "<null>");
    CHECK(ConsumeToText(u8"left 10px 20px", ConsumeBackgroundPosition) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeOneOrTwoComponentPositionUnresolved)",
            "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"right", ConsumeOneOrTwoComponentPosition) == "right center");
    CHECK(ConsumeToText(u8"top right", ConsumeOneOrTwoComponentPosition) == "right top");

    // Only up to two components are consumed.
    CHECK(ConsumeToText(u8"left 10px top 20px", ConsumeOneOrTwoComponentPosition) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumePosition axis-relative keywords)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};

    CHECK(ConsumeToText(u8"x-start", ConsumePosition, context) == "x-start center");
    CHECK(ConsumeToText(u8"y-end x-end", ConsumePosition, context) == "x-end y-end");
    CHECK(ConsumeToText(u8"x-start 5px y-start 5px", ConsumePosition, context) == "x-start 5px y-start 5px");
    CHECK(ConsumeToText(u8"x-end 1px", ConsumePositionX, context) == "x-end 1px");
    CHECK(ConsumeToText(u8"y-start", ConsumePositionY, context) == "y-start");
  }

  TEST_CASE("PropertyParserHelpers(ConsumePositionX and ConsumePositionY)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"left", ConsumePositionX) == "left");
    CHECK(ConsumeToText(u8"right 10px", ConsumePositionX) == "right 10px");
    CHECK(ConsumeToText(u8"center", ConsumePositionX) == "center");
    CHECK(ConsumeToText(u8"25%", ConsumePositionX) == "25%");
    CHECK(ConsumeToText(u8"top", ConsumePositionX) == "<null>");
    CHECK(ConsumeToText(u8"center 10px", ConsumePositionX) == "<partial>");

    CHECK(ConsumeToText(u8"bottom 5px", ConsumePositionY) == "bottom 5px");
    CHECK(ConsumeToText(u8"top", ConsumePositionY) == "top");
    CHECK(ConsumeToText(u8"0", ConsumePositionY) == "0px");
    CHECK(ConsumeToText(u8"left", ConsumePositionY) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeTwoComponentPosition*Unresolved)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"left 5px top bottom"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeTwoComponentPositionHorizontalUnresolved(range, state)
          == TwoComponentPositionHorizontal {keywords::Left {}});
    CHECK(ConsumeTwoComponentPositionHorizontalUnresolved(range, state)
          == TwoComponentPositionHorizontal {
            LengthPercentage<> {LengthPercentageRaw<> {LengthPercentageUnit::Px, 5}}});
    CHECK_FALSE(ConsumeTwoComponentPositionHorizontalUnresolved(range, state));

    CHECK(ConsumeTwoComponentPositionVerticalUnresolved(range, state)
          == TwoComponentPositionVertical {keywords::Top {}});
    CHECK(ConsumeTwoComponentPositionVerticalUnresolved(range, state)
          == TwoComponentPositionVertical {keywords::Bottom {}});
    CHECK(range.IsAtEnd());
  }
}
