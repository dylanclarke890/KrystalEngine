#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRangeGuard.hpp"
#include <string>

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

    std::string Serialise(const TokenRange &range)
    {
      auto text = range.Serialise(TokenSerialisationMode::Normal);
      return std::string(text.begin(), text.end());
    }
  }

  TEST_CASE("PropertyParserHelpers(ConsumeComma)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8",  a"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeComma(range));
    CHECK(range.Peek().Type() == TokenType::Ident);
    CHECK_FALSE(ConsumeComma(range));
    CHECK(range.Peek().Type() == TokenType::Ident);
  }

  TEST_CASE("PropertyParserHelpers(ConsumeSlash)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"/ a * b"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeSlash(range));
    CHECK(range.Peek().Type() == TokenType::Ident);
    CHECK_FALSE(ConsumeSlash(range));

    range.Discard();
    range.DiscardWhitespace();
    CHECK_FALSE(ConsumeSlash(range));
    CHECK(range.Peek().Type() == TokenType::Delim);
  }

  TEST_CASE("PropertyParserHelpers(ConsumeFunction)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"calc( 1px + f(2) )  next"};
    auto range = tokens.Tokenizer.Tokens();
    auto contents = ConsumeFunction(range);
    CHECK(Serialise(contents) == "1px + f(2) ");
    CHECK(Serialise(range) == "next");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeArgument)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"a b, f(c, d), e"};
    auto range = tokens.Tokenizer.Tokens();

    auto first = ConsumeArgument(range, 0);
    REQUIRE(first);
    CHECK(Serialise(*first) == "a b");

    auto second = ConsumeArgument(range, 1);
    REQUIRE(second);
    CHECK(Serialise(*second) == "f(c, d)");

    auto third = ConsumeArgument(range, 2);
    REQUIRE(third);
    CHECK(Serialise(*third) == "e");

    CHECK_FALSE(ConsumeArgument(range, 3));
  }

  TEST_CASE("TokenRangeGuard", "[CSS][Parser]")
  {
    Tokens tokens {u8"a b c"};
    auto range = tokens.Tokenizer.Tokens();

    {
      TokenRangeGuard guard {range};
      range.Discard();
    }
    CHECK(range.Peek().IdentCodePoints() == u8"a");

    {
      TokenRangeGuard guard {range};
      range.Discard();
      guard.Commit();
    }
    CHECK(range.Peek().Type() == TokenType::Whitespace);
  }

  TEST_CASE("TokenRange(Tokens)", "[CSS][Parser]")
  {
    Tokens tokens {u8"a b"};
    auto range = tokens.Tokenizer.Tokens();
    auto all = range.Tokens();
    range.Discard();
    CHECK(range.Tokens().size() == all.size() - 1);
    CHECK(range.Tokens().data() == all.data() + 1);
  }
}
