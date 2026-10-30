#include "Krystal.Booey/CSS/Properties/Consumers/UnicodeRange.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    constexpr auto Consume = [](TokenRange &tokens, PropertyParserState &)
    {
      return ConsumeUnicodeRangeToken(tokens);
    };
  }

  TEST_CASE("PropertyParserHelpers(ConsumeUnicodeRangeToken)", "[CSS][Properties][Consumers]")
  {
    // Ranges written with only letters and question marks.
    CHECK(ConsumeToText(u8"U+A", Consume) == "U+a");
    CHECK(ConsumeToText(u8"u+abc", Consume) == "U+abc");
    CHECK(ConsumeToText(u8"U+a-f ", Consume) == "U+a-f");
    CHECK(ConsumeToText(u8"U+A?", Consume) == "U+a0-af");
    CHECK(ConsumeToText(u8"U+??", Consume) == "U+0-ff");
    CHECK(ConsumeToText(u8"U+ABCDEF", Consume) == "<null>");

    CHECK(ConsumeToText(u8"U+abcdefa", Consume) == "<null>");
    CHECK(ConsumeToText(u8"U+???????", Consume) == "<null>");
    CHECK(ConsumeToText(u8"U+G", Consume) == "<null>");
    CHECK(ConsumeToText(u8"U+f-a", Consume) == "<null>");
    CHECK(ConsumeToText(u8"U+A?-B", Consume) == "<partial>");
    CHECK(ConsumeToText(u8"V+A", Consume) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeUnicodeRangeToken leaves the range on failure)",
            "[CSS][Properties][Consumers]")
  {
    css::tests::Tokens tokens {u8"U+G"};
    auto range = tokens.Tokenizer.Tokens();
    CHECK_FALSE(ConsumeUnicodeRangeToken(range));
    CHECK(range.Peek().Type() == TokenType::Ident);
  }

  TEST_CASE("PropertyParserHelpers(ConsumeUnicodeRangeToken numeric)", "[CSS][Properties][Consumers]")
  {
    // Ranges containing number and dimension tokens, which are read from their original text.
    CHECK(ConsumeToText(u8"U+0025-00FF", Consume) == "U+25-ff");
    CHECK(ConsumeToText(u8"U+4??", Consume) == "U+400-4ff");
    CHECK(ConsumeToText(u8"U+1e3", Consume) == "U+1e3");
    CHECK(ConsumeToText(u8"U+1F600", Consume) == "U+1f600");
    CHECK(ConsumeToText(u8"U+0-7F", Consume) == "U+0-7f");
    CHECK(ConsumeToText(u8"U+10-20", Consume) == "U+10-20");
    CHECK(ConsumeToText(u8"U+1e-3", Consume) == "<null>");
    CHECK(ConsumeToText(u8"U+0.5", Consume) == "<null>");
    // An escaped unit isn't a hex digit.
    CHECK(ConsumeToText(u8"U+0\\66", Consume) == "<null>");
  }
}
