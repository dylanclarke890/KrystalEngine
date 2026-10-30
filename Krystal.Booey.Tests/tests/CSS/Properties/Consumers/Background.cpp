#include "Krystal.Booey/CSS/Properties/Consumers/Background.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeBorderImageSlice)", "[CSS][Properties][Consumers]")
  {
    auto slice = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeBorderImageSlice(tokens, state);
    };

    CHECK(ConsumeToText(u8"10", slice) == "10");
    CHECK(ConsumeToText(u8"10% 20", slice) == "10% 20");
    CHECK(ConsumeToText(u8"1 2 3 4 fill", slice) == "1 2 3 4 fill");
    CHECK(ConsumeToText(u8"fill 5", slice) == "5 fill");

    CHECK(ConsumeToText(u8"-1", slice) == "<null>");
    CHECK(ConsumeToText(u8"1 2 3 4 5", slice) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeBorderImageWidth)", "[CSS][Properties][Consumers]")
  {
    auto width = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeBorderImageWidth(tokens, state);
    };

    CHECK(ConsumeToText(u8"1", width) == "1");
    CHECK(ConsumeToText(u8"auto", width) == "auto");
    CHECK(ConsumeToText(u8"10px 20%", width) == "10px 20%");
    CHECK(ConsumeToText(u8"1 2 3 4", width) == "1 2 3 4");
    CHECK(ConsumeToText(u8"-1px", width) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeBorderImageComponents)", "[CSS][Properties][Consumers]")
  {
    auto components = [](TokenRange &tokens, PropertyParserState &state) -> RefPtr<Value>
    {
      auto result = ConsumeBorderImageComponents(tokens, state);
      if (!result)
      {
        return nullptr;
      }

      return CreateBorderImageValue(krys::move(*result));
    };

    CHECK(ConsumeToText(u8"url(a.png) 10 round", components) == "url(\"a.png\") 10 round");
    CHECK(ConsumeToText(u8"url(a.png) 10 / 2px / 1px stretch", components)
          == "url(\"a.png\") 10 / 2px / 1px stretch");
    CHECK(ConsumeToText(u8"repeat space", components) == "repeat space");
    CHECK(ConsumeToText(u8"10 10", components) == "10");

    CHECK(ConsumeToText(u8"url(a.png) url(b.png)", components) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeSingleBackgroundSize)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"cover", ConsumeSingleBackgroundSize) == "cover");
    CHECK(ConsumeToText(u8"contain", ConsumeSingleBackgroundSize) == "contain");
    CHECK(ConsumeToText(u8"auto", ConsumeSingleBackgroundSize) == "auto");
    CHECK(ConsumeToText(u8"auto auto", ConsumeSingleBackgroundSize) == "auto");
    CHECK(ConsumeToText(u8"50% 10px", ConsumeSingleBackgroundSize) == "50% 10px");

    // A single length gets an `auto` height, which isn't coalesced away.
    CHECK(ConsumeToText(u8"10px", ConsumeSingleBackgroundSize) == "10px auto");
    CHECK(ConsumeToText(u8"10px auto", ConsumeSingleBackgroundSize) == "10px auto");

    CHECK(ConsumeToText(u8"-1px", ConsumeSingleBackgroundSize) == "<null>");
    CHECK(ConsumeToText(u8"cover 10px", ConsumeSingleBackgroundSize) == "<partial>");

    // -webkit-background-size repeats a single length; mask-size leaves it alone.
    CHECK(ConsumeToText(u8"10px", ConsumeSingleWebkitBackgroundSize) == "10px 10px");
    CHECK(ConsumeToText(u8"10px 20px", ConsumeSingleWebkitBackgroundSize) == "10px 20px");
    CHECK(ConsumeToText(u8"10px", ConsumeSingleMaskSize) == "10px");
    CHECK(ConsumeToText(u8"10px 20px", ConsumeSingleMaskSize) == "10px 20px");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeRepeatStyle)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"repeat-x", ConsumeRepeatStyle) == "repeat-x");
    CHECK(ConsumeToText(u8"repeat-y", ConsumeRepeatStyle) == "repeat-y");
    CHECK(ConsumeToText(u8"repeat", ConsumeRepeatStyle) == "repeat");
    CHECK(ConsumeToText(u8"space round", ConsumeRepeatStyle) == "space round");
    CHECK(ConsumeToText(u8"no-repeat no-repeat", ConsumeRepeatStyle) == "no-repeat");
    CHECK(ConsumeToText(u8"repeat-x repeat", ConsumeRepeatStyle) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeBoxShadow)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeBoxShadow) == "none");
    CHECK(ConsumeToText(u8"1px 2px", ConsumeBoxShadow) == "1px 2px");
    CHECK(ConsumeToText(u8"1px 2px 3px 4px red inset", ConsumeBoxShadow) == "red 1px 2px 3px 4px inset");
    CHECK(ConsumeToText(u8"inset red 1px 2px", ConsumeBoxShadow) == "red 1px 2px inset");
    CHECK(ConsumeToText(u8"1px 2px, 3px 4px blue", ConsumeBoxShadow) == "1px 2px, blue 3px 4px");

    CHECK(ConsumeToText(u8"1px", ConsumeBoxShadow) == "<null>");
    CHECK(ConsumeToText(u8"1px 2px -3px", ConsumeBoxShadow) == "<null>");
    CHECK(ConsumeToText(u8"red red 1px 2px", ConsumeBoxShadow) == "<null>");
  }
}
