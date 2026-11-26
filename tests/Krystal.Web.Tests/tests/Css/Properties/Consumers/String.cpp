#include "Krystal.Web/Css/Properties/Consumers/String.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ConsumerTestUtils.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Parser/InputStream.hpp"
#include "Krystal.Web/Css/Parser/Tokenizer.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"

namespace krys::web::css::parse_property::tests
{
  using css::tests::CssText;
  using css::tests::Tokens;
  using css::tests::ToNarrow;
  using css::tests::ValueFixture;

  TEST_CASE("parse_property(ConsumeStringRaw)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"\"foo\"  'bar' baz"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ToNarrow(ConsumeStringRaw(range)) == "foo");
    CHECK(ToNarrow(ConsumeStringRaw(range)) == "bar");
    CHECK(ConsumeStringRaw(range).empty());
    CHECK(range.Peek().Type() == TokenType::Ident);
  }

  TEST_CASE_METHOD(ValueFixture, "parse_property(ConsumeString)", "[CSS][Properties][Consumers]")
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
