#include "Krystal.Booey/CSS/Properties/Consumers/Content.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  TEST_CASE("PropertyParserHelpers(ConsumeQuotes)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeQuotes) == "none");
    CHECK(ConsumeToText(u8"auto", ConsumeQuotes) == "auto");
    CHECK(ConsumeToText(u8"\"a\" \"b\"", ConsumeQuotes) == "\"a\" \"b\"");
    CHECK(ConsumeToText(u8"\"a\" \"b\" \"c\" \"d\"", ConsumeQuotes) == "\"a\" \"b\" \"c\" \"d\"");

    // Quotes come in pairs.
    CHECK(ConsumeToText(u8"\"a\"", ConsumeQuotes) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeContent)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};

    CHECK(ConsumeToText(u8"normal", ConsumeContent) == "normal");
    CHECK(ConsumeToText(u8"none", ConsumeContent) == "none");
    CHECK(ConsumeToText(u8"\"a\"", ConsumeContent) == "\"a\"");
    CHECK(ConsumeToText(u8"counter(x)", ConsumeContent) == "counter(x)");
    CHECK(ConsumeToText(u8"counters(x, \".\", upper-roman)", ConsumeContent)
          == "counters(x, \".\", upper-roman)");
    CHECK(ConsumeToText(u8"attr(title)", ConsumeContent) == "attr(title)");
    CHECK(ConsumeToText(u8"open-quote \"a\"", ConsumeContent) == "open-quote \"a\"");
    CHECK(ConsumeToText(u8"url(a.png)", ConsumeContent, context) == "url(\"a.png\")");

    // Alternative text after `/`.
    CHECK(ConsumeToText(u8"\"a\" / \"alt\"", ConsumeContent) == "\"a\" / \"alt\"");
    CHECK(ConsumeToText(u8"\"a\" / ", ConsumeContent) == "<null>");

    CHECK(ConsumeToText(u8"none \"a\"", ConsumeContent) == "<partial>");
    CHECK(ConsumeToText(u8"counter(x, none)", ConsumeContent) == "<null>");
  }
}
