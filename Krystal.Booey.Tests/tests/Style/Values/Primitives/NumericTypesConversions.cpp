#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/AllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserOptions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCalc.hpp"

namespace krys::boo::style::tests
{
  using namespace css::Literals;

  namespace
  {
    constexpr css::NoConversionDataRequiredTag Token {};

    /// @brief Parses `text` as a calc value of `category`.
    template <typename CalcType>
    CalcType ParseCalc(utf8_stringview text, css::Category category)
    {
      css::InputStream input {css::CSSOMString(text)};
      css::Tokenizer tokenizer {input};
      REQUIRE(tokenizer.PumpTokenizer());

      css::ParserContext context;
      css::PropertyParserState state {.Context = context};
      auto tokens = tokenizer.Tokens();
      auto value = css::calc::Value::Parse(tokens, state, category, css::All, css::AllowedSymbols {},
                                           css::PropertyParserOptions {});
      REQUIRE(value);
      return CalcType {*value};
    }

    double EvaluateChild(calc::Child &&child, double percentResolutionLength = 0)
    {
      return calc::Evaluate(calc::Tree {.Root = krys::move(child)}, percentResolutionLength,
                            ZoomFactor {1.0f, 1.0f});
    }
  }

  TEST_CASE("Style conversions: raw canonicalization", "[Style][NumericTypesConversions]")
  {
    CHECK(Canonicalize(css::IntegerRaw<css::All, int> {2.996}, Token) == Integer<> {3});
    CHECK(Canonicalize(css::NumberRaw<> {2.5}, Token) == Number<> {2.5});
    CHECK(Canonicalize(css::PercentageRaw<> {50}, Token) == Percentage<> {50});
    CHECK(Canonicalize(css::AngleRaw<> {css::AngleUnit::Turn, 0.5}, Token) == Angle<> {180});
    CHECK(Canonicalize(css::TimeRaw<> {css::TimeUnit::Ms, 1'500}, Token) == Time<> {1.5});
    CHECK(Canonicalize(css::FrequencyRaw<> {css::FrequencyUnit::Khz, 2}, Token) == Frequency<> {2'000});
    CHECK(Canonicalize(css::ResolutionRaw<> {css::ResolutionUnit::Dpi, 192}, Token) == Resolution<> {2});
    CHECK(Canonicalize(css::FlexRaw<> {2}, Token) == Flex<> {2});
    CHECK(Canonicalize(css::LengthRaw<> {css::LengthUnit::In, 1}, Token) == Length<> {96.0f});
    CHECK(Canonicalize(css::LengthRaw<> {css::LengthUnit::Px, 1e20}, Token)
          == Length<> {css::MaxValueForCssLength});
    CHECK(Canonicalize(css::LengthRaw<> {css::LengthUnit::Px, -1e20}, Token)
          == Length<> {css::MinValueForCssLength});

    const css::ToLengthConversionData conversionData;
    CHECK(Canonicalize(css::LengthRaw<> {css::LengthUnit::Cm, 2.54}, conversionData).UnresolvedValue()
          == 96.0f);
    CHECK(Canonicalize(css::NumberRaw<> {2.5}, conversionData) == Number<> {2.5});

    auto lengthPercentage = Canonicalize(css::LengthPercentageRaw<> {css::LengthUnit::Px, 4}, Token);
    CHECK(lengthPercentage.HoldsAlternative<Length<>>());
    auto percentage =
      Canonicalize(css::LengthPercentageRaw<> {css::PercentageUnit::Percentage, 25}, conversionData);
    CHECK(percentage.HoldsAlternative<Percentage<css::All, float>>());
    auto angle = Canonicalize(css::AnglePercentageRaw<> {css::AngleUnit::Rad, 0}, Token);
    CHECK(angle.HoldsAlternative<Angle<css::All, float>>());
  }

  TEST_CASE("Style conversions: CSS to Style", "[Style][NumericTypesConversions]")
  {
    STATIC_CHECK((SameType<StyleType<css::Number<>>, Number<>>));
    STATIC_CHECK((SameType<StyleType<css::LengthPercentage<>>, LengthPercentage<>>));
    STATIC_CHECK(
      (SameType<StyleType<css::NumberOrPercentageResolvedToNumber<>>, NumberOrPercentageResolvedToNumber<>>));

    CHECK(ConvertToStyle(css::Number<> {2.5}, Token) == Number<> {2.5});
    CHECK(ConvertToStyle(css::Length<> {1_css_in}, Token) == Length<> {96.0f});
    CHECK(ConvertToStyleNoConversionDataRequired(css::Angle<> {0.5_css_turn}) == Angle<> {180});

    auto length = ConvertToStyle(css::LengthPercentage<> {4_css_px}, css::ToLengthConversionData {});
    CHECK(length == LengthPercentage<> {Length<> {4.0f}});

    auto resolved = ConvertToStyle(css::NumberOrPercentageResolvedToNumber<> {css::Percentage<> {50}}, Token);
    CHECK(resolved == 0.5);
  }

  TEST_CASE("Style conversions: calc to Style", "[Style][NumericTypesConversions]")
  {
    using CSSCalc = css::LengthPercentage<>::Calc;

    // Calc that simplifies to a length becomes a length.
    auto length =
      ConvertToStyle(ParseCalc<CSSCalc>(u8"calc(10px + 5px)", css::Category::LengthPercentage), Token);
    REQUIRE(length.HoldsAlternative<Length<>>());
    CHECK(get<Length<>>(length).UnresolvedValue() == 15.0f);

    // A lone percentage becomes a percentage.
    auto percentage =
      ConvertToStyle(ParseCalc<CSSCalc>(u8"calc(25% * 2)", css::Category::LengthPercentage), Token);
    REQUIRE(percentage.HoldsAlternative<Percentage<css::All, float>>());
    CHECK(get<Percentage<css::All, float>>(percentage).Value == 50.0f);

    // Mixed lengths and percentages stay calc. Building the Style calc for a length needs conversion data.
    auto mixed = ConvertToStyle(ParseCalc<CSSCalc>(u8"calc(10px + 50%)", css::Category::LengthPercentage),
                                css::ToLengthConversionData {});
    REQUIRE(mixed.HoldsAlternative<LengthPercentage<>::Calc>());
    CHECK(EvaluateChild(CopyCalculation(mixed), 100) == 60.0);

    // Non-composite calc evaluates directly.
    auto number = ConvertToStyle(
      css::Number<> {ParseCalc<css::Number<>::Calc>(u8"calc(1 + 2)", css::Category::Number)}, Token);
    CHECK(number == Number<> {3});
  }

  TEST_CASE("Style CopyCalculation", "[Style][NumericTypesCalc]")
  {
    CHECK(EvaluateChild(CopyCalculation(Number<> {2.5})) == 2.5);
    CHECK(EvaluateChild(CopyCalculation(Percentage<> {50}), 200) == 100.0);
    CHECK(EvaluateChild(CopyCalculation(Length<> {12.0f})) == 12.0);
    CHECK(EvaluateChild(CopyCalculation(Angle<> {90})) == 90.0);
    CHECK(EvaluateChild(CopyCalculation(LengthPercentage<> {Percentage<css::All, float> {25.0f}}), 40)
          == 10.0);
    CHECK(EvaluateChild(CopyCalculation(LengthPercentage<> {Length<> {7.0f}})) == 7.0);
  }
}
