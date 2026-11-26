#include "Krystal.Booey/Css/Properties/Consumers/UnicodeRange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Css/ConsumerTestUtils.hpp"

namespace krys::boo::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    constexpr auto UnicodeRangeConsumer = [](TokenRange &tokens, PropertyParserState &)
    {
      return ConsumeUnicodeRangeToken(tokens);
    };
  }

  TEST_CASE("parse_property(ConsumeUnicodeRangeToken)", "[CSS][Properties][Consumers]")
  {
    // Ranges written with only letters and question marks.
    CHECK(ConsumeToText(u8"U+A", UnicodeRangeConsumer) == "U+a");
    CHECK(ConsumeToText(u8"u+abc", UnicodeRangeConsumer) == "U+abc");
    CHECK(ConsumeToText(u8"U+a-f ", UnicodeRangeConsumer) == "U+a-f");
    CHECK(ConsumeToText(u8"U+A?", UnicodeRangeConsumer) == "U+a0-af");
    CHECK(ConsumeToText(u8"U+??", UnicodeRangeConsumer) == "U+0-ff");
    CHECK(ConsumeToText(u8"U+ABCDEF", UnicodeRangeConsumer) == "<null>");

    CHECK(ConsumeToText(u8"U+abcdefa", UnicodeRangeConsumer) == "<null>");
    CHECK(ConsumeToText(u8"U+???????", UnicodeRangeConsumer) == "<null>");
    CHECK(ConsumeToText(u8"U+G", UnicodeRangeConsumer) == "<null>");
    CHECK(ConsumeToText(u8"U+f-a", UnicodeRangeConsumer) == "<null>");
    CHECK(ConsumeToText(u8"U+A?-B", UnicodeRangeConsumer) == "<partial>");
    CHECK(ConsumeToText(u8"V+A", UnicodeRangeConsumer) == "<null>");
  }

  TEST_CASE("parse_property(ConsumeUnicodeRangeToken leaves the range on failure)",
            "[CSS][Properties][Consumers]")
  {
    css::tests::Tokens tokens {u8"U+G"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK_FALSE(ConsumeUnicodeRangeToken(range));
    CHECK(range.Peek().Type() == TokenType::Ident);
  }

  TEST_CASE("parse_property(ConsumeUnicodeRangeToken numeric)", "[CSS][Properties][Consumers]")
  {
    // Ranges containing number and dimension tokens, which are read from their original text.
    CHECK(ConsumeToText(u8"U+0025-00FF", UnicodeRangeConsumer) == "U+25-ff");
    CHECK(ConsumeToText(u8"U+4??", UnicodeRangeConsumer) == "U+400-4ff");
    CHECK(ConsumeToText(u8"U+1e3", UnicodeRangeConsumer) == "U+1e3");
    CHECK(ConsumeToText(u8"U+1F600", UnicodeRangeConsumer) == "U+1f600");
    CHECK(ConsumeToText(u8"U+0-7F", UnicodeRangeConsumer) == "U+0-7f");
    CHECK(ConsumeToText(u8"U+10-20", UnicodeRangeConsumer) == "U+10-20");
    CHECK(ConsumeToText(u8"U+1e-3", UnicodeRangeConsumer) == "<null>");
    CHECK(ConsumeToText(u8"U+0.5", UnicodeRangeConsumer) == "<null>");
    // An escaped unit isn't a hex digit.
    CHECK(ConsumeToText(u8"U+0\\66", UnicodeRangeConsumer) == "<null>");
  }
}
