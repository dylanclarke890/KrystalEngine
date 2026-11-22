#include "Krystal.Booey/CSS/Properties/Consumers/Url.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    constexpr auto UrlConsumer(AllowedUrlModifiers allowed) noexcept
    {
      return [allowed](TokenRange &tokens, PropertyParserState &state)
      {
        return ConsumeUrl(tokens, state, allowed);
      };
    }

    constexpr auto AllModifiers = AllowedUrlModifiers {
      AllowedUrlModifier::CrossOrigin, AllowedUrlModifier::Integrity, AllowedUrlModifier::ReferrerPolicy};
  }

  TEST_CASE("parse_property(ConsumeURL)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, net::Url(u8"https://example.com/dir/")};
    auto consume = UrlConsumer({});

    CHECK(ConsumeToText(u8"url(a.png)", consume, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"url(\"a.png\")", consume, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"URL('b.png')  ", consume, context) == "url(\"b.png\")");

    // An empty string is a valid URL.
    CHECK(ConsumeToText(u8"url(\"\")", consume, context) == "url(\"\")");

    CHECK(ConsumeToText(u8"url()", consume, context) == "url(\"\")");
    CHECK(ConsumeToText(u8"url(a b)", consume, context) == "<null>");
    CHECK(ConsumeToText(u8"\"a.png\"", consume, context) == "<null>");
    CHECK(ConsumeToText(u8"src(\"a.png\")", consume, context) == "<null>");
  }

  TEST_CASE("parse_property(ConsumeURL modifiers)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, net::Url(u8"https://example.com/")};

    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(anonymous))", UrlConsumer(AllModifiers), context)
          == "url(\"a\" cross-origin(anonymous))");
    CHECK(ConsumeToText(u8"url(\"a\" referrer-policy(no-referrer) cross-origin(use-credentials))",
                        UrlConsumer(AllModifiers), context)
          == "url(\"a\" cross-origin(use-credentials) referrer-policy(no-referrer))");

    // Duplicates, bad arguments and modifiers that aren't allowed here.
    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(anonymous) cross-origin(anonymous))",
                        UrlConsumer(AllModifiers), context)
          == "<null>");
    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(foo))", UrlConsumer(AllModifiers), context) == "<null>");
    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(anonymous))",
                        UrlConsumer(AllowedUrlModifier::ReferrerPolicy), context)
          == "<null>");
    CHECK(ConsumeToText(u8"url(\"a\" foo(bar))", UrlConsumer(AllModifiers), context) == "<null>");

    CHECK(ConsumeToText(u8"url(\"a\" integrity(\"sha384-x\"))", UrlConsumer(AllModifiers), context)
          == "url(\"a\" integrity(\"sha384-x\"))");
    CHECK(ConsumeToText(u8"url(\"a\" integrity(\"\"))", UrlConsumer(AllModifiers), context)
          == "url(\"a\" integrity(\"\"))");
    CHECK(ConsumeToText(u8"url(\"a\" integrity(x))", UrlConsumer(AllModifiers), context) == "<null>");
  }
}
