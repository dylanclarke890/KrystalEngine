#include "Krystal.Booey/CSS/Properties/Consumers/Attr.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    // `ConsumeAttr` takes the function's arguments by value and consumes all of them.
    constexpr auto Consume = [](TokenRange &tokens, PropertyParserState &state)
    {
      auto value = ConsumeAttr(tokens, state);
      std::ignore = tokens.ConsumeAll();
      return value;
    };
  }

  TEST_CASE("PropertyParserHelpers(ConsumeAttr)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"data-x", Consume) == "attr(data-x)");
    CHECK(ConsumeToText(u8"Data-X", Consume) == "attr(Data-X)");
    CHECK(ConsumeToText(u8"title, \"none\"", Consume) == "attr(title, \"none\")");
    CHECK(ConsumeToText(u8"title,", Consume) == "attr(title)");

    CHECK(ConsumeToText(u8"5", Consume) == "<null>");
    CHECK(ConsumeToText(u8"a b", Consume) == "<null>");
    CHECK(ConsumeToText(u8"a, b", Consume) == "<null>");
    CHECK(ConsumeToText(u8"a, \"b\" c", Consume) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeAttr HTML document)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    context.IsHTMLDocument = true;
    CHECK(ConsumeToText(u8"Data-X", Consume, context) == "attr(data-x)");
  }
}
