#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::CssText;
  using css::tests::ToNarrow;
  using css::tests::ValueFixture;

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

  TEST_CASE("PropertyParserHelpers(ConsumeStringRaw)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"\"foo\"  'bar' baz"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ToNarrow(ConsumeStringRaw(range)) == "foo");
    CHECK(ToNarrow(ConsumeStringRaw(range)) == "bar");
    CHECK(ConsumeStringRaw(range).empty());
    CHECK(range.Peek().Type() == TokenType::Ident);
  }

  TEST_CASE_METHOD(ValueFixture, "PropertyParserHelpers(ConsumeString)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"\"a b\" 5"};
    auto range = tokens.Tokenizer.Tokens();
    auto value = ConsumeString(range);
    REQUIRE(value);
    CHECK(value->IsString());
    CHECK(CssText(*value) == "\"a b\"");
    CHECK_FALSE(ConsumeString(range));
    CHECK(range.Peek().Type() == TokenType::Number);
  }
}
