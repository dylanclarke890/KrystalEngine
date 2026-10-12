#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
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

  TEST_CASE("PropertyParserHelpers(ConsumeIdentRaw)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"auto  none 5"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeIdentRaw(range) == ValueId::Auto);
    CHECK(ConsumeIdentRaw<ValueId::Auto>(range) == null);
    CHECK(ConsumeIdentRaw<ValueId::Auto, ValueId::None>(range) == ValueId::None);
    CHECK(ConsumeIdentRaw(range) == null);
    CHECK(range.Peek().Type() == TokenType::Number);
  }

  TEST_CASE("PropertyParserHelpers(ConsumeIdentRaw predicate)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"top left"};
    auto range = tokens.Tokenizer.Tokens();
    auto isTop = [](ValueId id)
    {
      return id == ValueId::Top;
    };
    CHECK(ConsumeIdentRaw(range, isTop) == ValueId::Top);
    CHECK(ConsumeIdentRaw(range, isTop) == null);
    CHECK(ConsumeIdentRangeRaw(range, ValueId::Left, ValueId::Left) == ValueId::Left);
    CHECK(range.IsAtEnd());
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCustomIdentRaw)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"Foo inherit default Bar 5"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeCustomIdentRaw(range) == CSSOMString(u8"Foo"));

    // CSS-wide keywords and "default" aren't valid custom identifiers.
    CHECK(ConsumeCustomIdentRaw(range) == null);
    DiscardIdent(range);
    CHECK(ConsumeCustomIdentRaw(range) == null);
    DiscardIdent(range);

    CHECK(ConsumeCustomIdentRaw(range, true) == CSSOMString(u8"bar"));
    CHECK(ConsumeCustomIdentRaw(range) == null);
  }

  TEST_CASE("PropertyParserHelpers(ConsumeDashedIdentRaw)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"--a foo 5"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeDashedIdentRaw(range) == CSSOMString(u8"--a"));

    // Not dashed: the range is left where it was.
    CHECK(ConsumeDashedIdentRaw(range) == null);
    CHECK(range.Peek().IdentCodePoints() == u8"foo");
    DiscardIdent(range);

    // Not an identifier at all.
    CHECK(ConsumeDashedIdentRaw(range) == null);
    CHECK(range.Peek().Type() == TokenType::Number);
  }
}
