#include "Krystal.Booey/CSS/Properties/Consumers/Attr.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::parse_property::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    // `ConsumeAttr` takes the function's arguments by value and consumes all of them.
    constexpr auto AttrConsumer = [](TokenRange &tokens, PropertyParserState &state)
    {
      auto value = ConsumeAttr(tokens, state);
      std::ignore = tokens.ConsumeAll();
      return value;
    };
  }

  TEST_CASE("parse_property(ConsumeAttr)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"data-x", AttrConsumer) == "attr(data-x)");
    CHECK(ConsumeToText(u8"Data-X", AttrConsumer) == "attr(Data-X)");
    CHECK(ConsumeToText(u8"title, \"none\"", AttrConsumer) == "attr(title, \"none\")");
    CHECK(ConsumeToText(u8"title,", AttrConsumer) == "attr(title)");

    CHECK(ConsumeToText(u8"5", AttrConsumer) == "<null>");
    CHECK(ConsumeToText(u8"a b", AttrConsumer) == "<null>");
    CHECK(ConsumeToText(u8"a, b", AttrConsumer) == "<null>");
    CHECK(ConsumeToText(u8"a, \"b\" c", AttrConsumer) == "<null>");
  }

  TEST_CASE("parse_property(ConsumeAttr HTML document)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    context.IsHTMLDocument = true;
    CHECK(ConsumeToText(u8"Data-X", AttrConsumer, context) == "attr(data-x)");
  }
}
