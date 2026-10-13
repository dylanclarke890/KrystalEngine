#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    template <AllowedURLModifiers Allowed>
    constexpr auto Consume = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeURL(tokens, state, Allowed);
    };

    constexpr auto AllModifiers =
      AllowedURLModifiers::CrossOrigin | AllowedURLModifiers::Integrity | AllowedURLModifiers::ReferrerPolicy;
  }

  TEST_CASE("PropertyParserHelpers(ConsumeURL)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};
    auto consume = Consume<AllowedURLModifiers::None>;

    CHECK(ConsumeToText(u8"url(a.png)", consume, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"url(\"a.png\")", consume, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"URL('b.png')  ", consume, context) == "url(\"b.png\")");

    // An empty string is a valid URL.
    CHECK(ConsumeToText(u8"url(\"\")", consume, context) == "url(\"\")");

    CHECK(ConsumeToText(u8"url()", consume, context) == "url(\"\")");
    CHECK(ConsumeToText(u8"url(a b)", consume, context) == "<null>");
    CHECK(ConsumeToText(u8"\"a.png\"", consume, context) == "<null>");
    CHECK(ConsumeToText(u8"src(\"a.png\")", consume, context) == "<null>");

    // Modifiers are rejected unless the setting is on.
    CHECK(ConsumeToText(u8"url(\"a.png\" cross-origin(anonymous))", Consume<AllModifiers>, context)
          == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeURL modifiers)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/")};
    context.cssURLModifiersEnabled = true;

    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(anonymous))", Consume<AllModifiers>, context)
          == "url(\"a\" cross-origin(anonymous))");
    CHECK(ConsumeToText(u8"url(\"a\" referrer-policy(no-referrer) cross-origin(use-credentials))",
                        Consume<AllModifiers>, context)
          == "url(\"a\" cross-origin(use-credentials) referrer-policy(no-referrer))");

    // Duplicates, bad arguments and modifiers that aren't allowed here.
    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(anonymous) cross-origin(anonymous))", Consume<AllModifiers>,
                        context)
          == "<null>");
    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(foo))", Consume<AllModifiers>, context) == "<null>");
    CHECK(ConsumeToText(u8"url(\"a\" cross-origin(anonymous))", Consume<AllowedURLModifiers::ReferrerPolicy>,
                        context)
          == "<null>");
    CHECK(ConsumeToText(u8"url(\"a\" foo(bar))", Consume<AllModifiers>, context) == "<null>");

    // integrity() has its own setting.
    CHECK(ConsumeToText(u8"url(\"a\" integrity(\"sha384-x\"))", Consume<AllModifiers>, context) == "<null>");
    context.cssURLIntegrityModifierEnabled = true;
    CHECK(ConsumeToText(u8"url(\"a\" integrity(\"sha384-x\"))", Consume<AllModifiers>, context)
          == "url(\"a\" integrity(\"sha384-x\"))");
    CHECK(ConsumeToText(u8"url(\"a\" integrity(\"\"))", Consume<AllModifiers>, context)
          == "url(\"a\" integrity(\"\"))");
    CHECK(ConsumeToText(u8"url(\"a\" integrity(x))", Consume<AllModifiers>, context) == "<null>");
  }
}
