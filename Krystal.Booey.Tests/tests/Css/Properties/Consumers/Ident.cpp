#include "Krystal.Booey/Css/Properties/Consumers/Ident.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ConsumerTestUtils.hpp"
#include "Krystal.Booey/Css/Parser/InputStream.hpp"
#include "Krystal.Booey/Css/Parser/Tokenizer.hpp"

namespace krys::boo::css::parse_property::tests
{
  using css::tests::Tokens;

  TEST_CASE("parse_property(ConsumeIdentRaw)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"auto  none 5"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK(ConsumeIdentRaw(range) == ValueId::Auto);
    CHECK(ConsumeIdentRaw<ValueId::Auto>(range) == null);
    CHECK(ConsumeIdentRaw<ValueId::Auto, ValueId::None>(range) == ValueId::None);
    CHECK(ConsumeIdentRaw(range) == null);
    CHECK(range.Peek().Type() == TokenType::Number);
  }

  TEST_CASE("parse_property(ConsumeIdentRaw predicate)", "[CSS][Properties][Consumers]")
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

  TEST_CASE("parse_property(ConsumeIdentUsingMapping)", "[CSS][Properties][Consumers]")
  {
    Tokens tokens {u8"top left"};
    auto range = tokens.Tokenizer.Tokens();
    auto mapping = [](ValueId id) -> Maybe<int32>
    {
      if (id == ValueId::Top)
      {
        return 1;
      }
      return null;
    };
    CHECK(PeekIdentUsingMapping(range, mapping) == 1);
    CHECK(ConsumeIdentUsingMapping(range, mapping) == 1);
    CHECK(PeekIdentUsingMapping(range, mapping) == null);
    CHECK(ConsumeIdentUsingMapping(range, mapping) == null);
    CHECK(range.Peek().ValueId() == ValueId::Left);
  }

  TEST_CASE("parse_property(ConsumeCustomIdentRaw)", "[CSS][Properties][Consumers]")
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

  TEST_CASE("parse_property(ConsumeDashedIdentRaw)", "[CSS][Properties][Consumers]")
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
