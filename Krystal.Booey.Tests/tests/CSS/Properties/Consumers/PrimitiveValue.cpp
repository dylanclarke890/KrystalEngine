#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    struct Tokens
    {
      InputStream Input;
      Tokenizer Tokenizer;

      explicit Tokens(utf8_stringview css) : Input(CSSOMString(css)), Tokenizer(Input)
      {
        REQUIRE(Tokenizer.PumpTokenizer());
      }
    };

    template <typename... Ts>
    std::string ConsumeAndResolve(utf8_stringview css)
    {
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      auto value = PrimitiveValueResolver<Ts...>::ConsumeAndResolve(range, state);
      if (!value)
      {
        return "<null>";
      }

      CHECK(range.IsAtEnd());
      return CssText(*value);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValueResolver(raw)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAndResolve<Length<>>(u8"5px") == "5px");
    CHECK(ConsumeAndResolve<Number<>>(u8"1.5") == "1.5");
    CHECK(ConsumeAndResolve<LengthPercentage<>>(u8"50%") == "50%");
    CHECK(ConsumeAndResolve<Length<>>(u8"5deg") == "<null>");

    auto integer = PrimitiveValueResolverBase::Resolve(IntegerRaw<All, int> {IntegerUnit::Integer, 7}, {});
    REQUIRE(integer);
    CHECK(integer->IsInteger());
    CHECK(CssText(*integer) == "7");
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValueResolver(calc)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};
    Tokens tokens {u8"calc(1px + 1em)"};
    auto range = tokens.Tokenizer.Tokens();
    auto value = PrimitiveValueResolver<Length<>>::ConsumeAndResolve(range, state);
    REQUIRE(value);
    CHECK(value->IsCalculated());
    CHECK(CssText(*value) == "calc(1em + 1px)");
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValueResolver(alternatives)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeAndResolve<Integer<>, Length<>>(u8"3") == "3");
    CHECK(ConsumeAndResolve<Integer<>, Length<>>(u8"3px") == "3px");
  }

  TEST_CASE_METHOD(ValueFixture, "PrimitiveValueResolver(NumberOrPercentageResolvedToNumber)",
                   "[CSS][Properties][Consumers]")
  {
    using Resolved = NumberOrPercentageResolvedToNumber<All, All, double>;

    auto number = PrimitiveValueResolverBase::Resolve(Resolved {NumberRaw<All> {NumberUnit::Number, 2.0}});
    REQUIRE(number);
    CHECK(CssText(*number) == "2");

    auto percentage =
      PrimitiveValueResolverBase::Resolve(Resolved {PercentageRaw<All> {PercentageUnit::Percentage, 50.0}});
    REQUIRE(percentage);
    CHECK(percentage->IsNumber());
    CHECK(CssText(*percentage) == "0.5");
  }
}
