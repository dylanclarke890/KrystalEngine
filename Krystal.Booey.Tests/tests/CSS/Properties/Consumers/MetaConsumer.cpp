#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/SymbolDefinitions.hpp"
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
    ParserContext context {ParserMode::HTMLStandard};
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
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"2"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK_FALSE(MetaConsumer<Number<ClosedUnitRange>>::Consume(range, state));
    CHECK(range.Peek().Type() == TokenType::Number);
    CHECK(MetaConsumer<Number<NonNegative>>::Consume(range, state));
  }

  TEST_CASE("MetaConsumer(unsupported tokens)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
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
    ParserContext context {ParserMode::HTMLStandard};
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
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"4"};
    auto range = tokens.Tokenizer.Tokens();
    auto doubled = MetaConsumer<Number<All>>::Consume(range, state, [](const Number<All> &number)
                                                      { return number.GetRaw()->Value * 2; });
    REQUIRE(doubled);
    CHECK(*doubled == 8.0);
  }

  TEST_CASE("MetaConsumer(unitless angles)", "[CSS][Properties][Consumers]")
  {
    auto consume = [](utf8_stringview css, ParserMode mode, PropertyId property)
    {
      ParserContext context {mode};
      PropertyParserState state {.Context = context, .CurrentProperty = property};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      return MetaConsumer<Angle<All>>::Consume(range, state).has_value();
    };

    CHECK(consume(u8"90deg", ParserMode::HTMLStandard, PropertyId::Width));
    CHECK_FALSE(consume(u8"0", ParserMode::HTMLStandard, PropertyId::Width));
    CHECK_FALSE(consume(u8"90", ParserMode::HTMLStandard, PropertyId::GlyphOrientationVertical));
    CHECK_FALSE(consume(u8"90", ParserMode::HTMLQuirks, PropertyId::Width));
    CHECK(consume(u8"90", ParserMode::HTMLQuirks, PropertyId::GlyphOrientationVertical));
    CHECK(consume(u8"90", ParserMode::HTMLQuirks, PropertyId::GlyphOrientationHorizontal));
    CHECK(consume(u8"90", ParserMode::SVGAttribute, PropertyId::Width));
  }

  TEST_CASE("MetaConsumer(symbols)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};
    AllowedSymbols symbols {{{ValueId::R, UnitType::Number}}};

    Tokens tokens {u8"r g"};
    auto range = tokens.Tokenizer.Tokens();
    auto symbol = MetaConsumer<Number<All>, Symbol>::Consume(range, state, symbols);
    REQUIRE(symbol);
    REQUIRE(std::holds_alternative<Symbol>(*symbol));
    CHECK(std::get<Symbol>(*symbol).Value == ValueId::R);

    // Not in the table.
    CHECK_FALSE(MetaConsumer<Number<All>, Symbol>::Consume(range, state, symbols));
    CHECK(range.Peek().ValueId() == ValueId::G);
    CHECK(ValidatedRange(SymbolRaw {ValueId::G}, {}) == SymbolRaw {ValueId::G});
  }
}
