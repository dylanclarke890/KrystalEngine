#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"

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
  }

  TEST_CASE("MetaConsumer(Number token)", "[CSS][Properties][Consumers]")
  {
    ParserContext context;
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"5  0.5"};
    auto range = tokens.Tokenizer.Tokens();
    auto number = MetaConsumer<Number<All>>::Consume(range, state);
    REQUIRE(number);
    REQUIRE(number->GetRaw());
    CHECK(number->GetRaw()->Value == 5.0);

    // The whitespace after the token is consumed.
    CHECK(range.Peek().Type() == TokenType::Number);
    auto unit = MetaConsumer<Number<ClosedUnitRange>>::Consume(range, state);
    REQUIRE(unit);
    CHECK(unit->GetRaw()->Value == 0.5);
    CHECK(range.IsAtEnd());
  }

  TEST_CASE("MetaConsumer(range validation)", "[CSS][Properties][Consumers]")
  {
    ParserContext context;
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"2"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK_FALSE(MetaConsumer<Number<ClosedUnitRange>>::Consume(range, state));
    CHECK(range.Peek().Type() == TokenType::Number);
    CHECK(MetaConsumer<Number<NonNegative>>::Consume(range, state));
  }

  TEST_CASE("MetaConsumer(unsupported tokens)", "[CSS][Properties][Consumers]")
  {
    ParserContext context;
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"auto 5px 5% \"5\""};
    auto range = tokens.Tokenizer.Tokens();
    CHECK_FALSE(MetaConsumer<Number<All>>::Consume(range, state));
    range.Discard();
    range.DiscardWhitespace();
    CHECK_FALSE(MetaConsumer<Number<All>>::Consume(range, state));
    range.Discard();
    range.DiscardWhitespace();
    CHECK_FALSE(MetaConsumer<Number<All>>::Consume(range, state));
    range.Discard();
    range.DiscardWhitespace();
    CHECK_FALSE(MetaConsumer<Number<All>>::Consume(range, state));
  }

  TEST_CASE("MetaConsumer(calc)", "[CSS][Properties][Consumers]")
  {
    ParserContext context;
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"calc(2 * 3) calc(1px) 7"};
    auto range = tokens.Tokenizer.Tokens();
    auto number = MetaConsumer<Number<All>>::Consume(range, state);
    REQUIRE(number);
    REQUIRE(number->IsCalc());
    CHECK(number->GetCalc()->ProtectedCalc()->DoubleValue(NoConversionDataRequiredTag {}) == 6.0);

    // A failed calc() leaves the range untouched.
    CHECK_FALSE(MetaConsumer<Number<All>>::Consume(range, state));
    CHECK(range.Peek().Type() == TokenType::Function);
    (void)range.ConsumeBlock();
    range.DiscardWhitespace();

    auto plain = MetaConsumer<Number<All>>::Consume(range, state);
    REQUIRE(plain);
    CHECK(plain->GetRaw()->Value == 7.0);
  }

  TEST_CASE("MetaConsumer(functor)", "[CSS][Properties][Consumers]")
  {
    ParserContext context;
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"4"};
    auto range = tokens.Tokenizer.Tokens();
    auto doubled = MetaConsumer<Number<All>>::Consume(range, state, [](const Number<All> &number)
                                                      { return number.GetRaw()->Value * 2; });
    REQUIRE(doubled);
    CHECK(*doubled == 8.0);
  }
}
