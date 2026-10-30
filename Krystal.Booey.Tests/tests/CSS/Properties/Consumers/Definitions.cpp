#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AnglePercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/FlexDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/FrequencyDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/TimeDefinitions.hpp"

namespace krys::boo::css::tests
{
  namespace
  {

    /// @brief Consumes a single `T` from `css`, requiring the whole input to be consumed on success.
    template <typename T>
    Maybe<T> ConsumeAll(utf8_stringview css, ParserMode mode = ParserMode::HTMLStandard,
                        PropertyId property = PropertyId::Invalid, PropertyParserOptions options = {})
    {
      ParserContext context {mode};
      PropertyParserState state {.Context = context, .CurrentProperty = property};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      auto result = MetaConsumer<T>::Consume(range, state, options);
      if (result)
      {
        CHECK(range.IsAtEnd());
      }
      return result;
    }
  }

  TEST_CASE("ConsumerDefinition(Integer)", "[CSS][Properties][Consumers]")
  {
    auto integer = ConsumeAll<Integer<>>(u8"7");
    REQUIRE(integer);
    CHECK(integer->GetRaw()->Value == 7);

    CHECK_FALSE(ConsumeAll<Integer<>>(u8"7.5"));
    CHECK_FALSE(ConsumeAll<Integer<>>(u8"7px"));
    CHECK_FALSE(ConsumeAll<Integer<NonNegative>>(u8"-1"));

    auto calc = ConsumeAll<Integer<>>(u8"calc(2 + 3)");
    REQUIRE(calc);
    CHECK(calc->IsCalc());
  }

  TEST_CASE("ConsumerDefinition(Length)", "[CSS][Properties][Consumers]")
  {
    auto length = ConsumeAll<Length<>>(u8"5px");
    REQUIRE(length);
    CHECK(length->GetRaw()->Unit == LengthUnit::Px);
    CHECK(length->GetRaw()->Value == 5.0);

    CHECK(ConsumeAll<Length<>>(u8"2em")->GetRaw()->Unit == LengthUnit::Em);
    CHECK_FALSE(ConsumeAll<Length<>>(u8"5deg"));
    CHECK_FALSE(ConsumeAll<Length<>>(u8"5%"));
    CHECK_FALSE(ConsumeAll<Length<NonNegative>>(u8"-5px"));
    CHECK(ConsumeAll<Length<>>(u8"calc(1px + 1em)")->IsCalc());

    // Unitless values: zero (unless disallowed), SVG attributes, and quirky properties in quirks mode.
    CHECK(ConsumeAll<Length<>>(u8"0"));
    CHECK_FALSE(ConsumeAll<Length<>>(u8"0", ParserMode::HTMLStandard, PropertyId::Invalid,
                                     {.UnitlessZeroLength = AllowUnitlessZero {false}}));
    CHECK_FALSE(ConsumeAll<Length<>>(u8"5"));
    CHECK(ConsumeAll<Length<>>(u8"5", ParserMode::SVGAttribute)->GetRaw()->Unit == LengthUnit::Px);
    CHECK(ConsumeAll<Length<>>(u8"5", ParserMode::HTMLQuirks, PropertyId::FontSize));
    CHECK_FALSE(ConsumeAll<Length<>>(u8"5", ParserMode::HTMLQuirks, PropertyId::Invalid));
    CHECK_FALSE(ConsumeAll<Length<>>(u8"5", ParserMode::HTMLStandard, PropertyId::FontSize));

    // The quirky em unit is only allowed in UA sheets.
    CHECK_FALSE(ConsumeAll<Length<>>(u8"1__qem"));
    CHECK(ConsumeAll<Length<>>(u8"1__qem", ParserMode::UASheet)->GetRaw()->Unit == LengthUnit::QuirkyEm);
    CHECK(ConsumeAll<Length<>>(u8"1__qem", ParserMode::HTMLStandard, PropertyId::Invalid,
                               {.OverrideParserMode = ParserMode::UASheet}));
  }

  TEST_CASE("ConsumerDefinition(LengthPercentage)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAll<LengthPercentage<>>(u8"5px")->GetRaw()->Unit == LengthPercentageUnit::Px);

    auto percentage = ConsumeAll<LengthPercentage<>>(u8"50%");
    REQUIRE(percentage);
    CHECK(percentage->GetRaw()->Unit == LengthPercentageUnit::Percentage);
    CHECK(percentage->GetRaw()->Value == 50.0);

    CHECK(ConsumeAll<LengthPercentage<>>(u8"0")->GetRaw()->Unit == LengthPercentageUnit::Px);
    CHECK(ConsumeAll<LengthPercentage<>>(u8"5", ParserMode::HTMLQuirks, PropertyId::FontSize));
    CHECK_FALSE(ConsumeAll<LengthPercentage<>>(u8"1__qem"));
    CHECK_FALSE(ConsumeAll<LengthPercentage<NonNegative>>(u8"-1%"));
    CHECK(ConsumeAll<LengthPercentage<>>(u8"calc(50% - 1px)")->IsCalc());
  }

  TEST_CASE("ConsumerDefinition(AnglePercentage)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAll<AnglePercentage<>>(u8"1turn")->GetRaw()->Unit == AnglePercentageUnit::Turn);
    CHECK(ConsumeAll<AnglePercentage<>>(u8"25%")->GetRaw()->Unit == AnglePercentageUnit::Percentage);
    CHECK_FALSE(ConsumeAll<AnglePercentage<>>(u8"5px"));

    CHECK(ConsumeAll<AnglePercentage<>>(u8"0", ParserMode::HTMLStandard, PropertyId::Invalid,
                                        {.UnitlessZeroAngle = AllowUnitlessZero {true}})
            ->GetRaw()
            ->Unit
          == AnglePercentageUnit::Deg);
    CHECK_FALSE(ConsumeAll<AnglePercentage<>>(u8"0"));
  }

  TEST_CASE("ConsumerDefinition(Time)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAll<Time<>>(u8"2s")->GetRaw()->Unit == TimeUnit::S);
    CHECK(ConsumeAll<Time<>>(u8"200ms")->GetRaw()->Unit == TimeUnit::Ms);
    CHECK_FALSE(ConsumeAll<Time<>>(u8"0"));
    CHECK_FALSE(ConsumeAll<Time<NonNegative>>(u8"-1s"));
    CHECK(ConsumeAll<Time<>>(u8"calc(1s + 5ms)")->IsCalc());
  }

  TEST_CASE("ConsumerDefinition(Resolution)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAll<Resolution<>>(u8"2dppx")->GetRaw()->Unit == ResolutionUnit::Dppx);
    CHECK(ConsumeAll<Resolution<>>(u8"96dpi")->GetRaw()->Unit == ResolutionUnit::Dpi);
    CHECK(ConsumeAll<Resolution<>>(u8"1x")->GetRaw()->Unit == ResolutionUnit::X);
    CHECK_FALSE(ConsumeAll<Resolution<>>(u8"1"));
  }

  TEST_CASE("ConsumerDefinition(Flex)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAll<Flex<>>(u8"1fr")->GetRaw()->Value == 1.0);
    CHECK_FALSE(ConsumeAll<Flex<NonNegative>>(u8"-1fr"));
    CHECK_FALSE(ConsumeAll<Flex<>>(u8"1px"));
  }

  TEST_CASE("ConsumerDefinition(Frequency)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAll<Frequency<>>(u8"440hz")->GetRaw()->Unit == FrequencyUnit::Hz);
    CHECK(ConsumeAll<Frequency<>>(u8"1khz")->GetRaw()->Unit == FrequencyUnit::Khz);
    CHECK_FALSE(ConsumeAll<Frequency<>>(u8"1s"));
  }
}
