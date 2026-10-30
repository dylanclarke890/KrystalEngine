#include "Krystal.Booey/CSS/Properties/Consumers/UI.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    constexpr auto Cursor = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeCursor(tokens, state);
    };

    ParserContext Context(ParserMode mode = ParserMode::HTMLStandard)
    {
      return ParserContext {mode, url::URL(u8"https://example.com/dir/")};
    }
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCursor keywords)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", Cursor) == "auto");
    CHECK(ConsumeToText(u8"pointer", Cursor) == "pointer");
    CHECK(ConsumeToText(u8"none", Cursor) == "none");
    CHECK(ConsumeToText(u8"copy", Cursor) == "copy");
    CHECK(ConsumeToText(u8"zoom-in", Cursor) == "zoom-in");
    CHECK(ConsumeToText(u8"-webkit-zoom-out", Cursor) == "-webkit-zoom-out");
    CHECK(ConsumeToText(u8"inherit", Cursor) == "<null>");
    CHECK(ConsumeToText(u8"10px", Cursor) == "<null>");

    // `hand` is a quirks-mode alias for `pointer`.
    CHECK(ConsumeToText(u8"hand", Cursor) == "<null>");
    CHECK(ConsumeToText(u8"hand", Cursor, Context(ParserMode::HTMLQuirks)) == "pointer");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeCursor images)", "[CSS][Properties][Consumers]")
  {
    auto context = Context();

    CHECK(ConsumeToText(u8"url(a.png), auto", Cursor, context) == "url(\"a.png\"), auto");
    CHECK(ConsumeToText(u8"url(a.png) 1 2, pointer", Cursor, context) == "url(\"a.png\") 1 2, pointer");
    CHECK(ConsumeToText(u8"url(a.png), image-set(url(b.png) 2x) 3 4, wait", Cursor, context)
          == "url(\"a.png\"), image-set(url(\"b.png\") 2x) 3 4, wait");

    // The keyword is required, and comes last.
    CHECK(ConsumeToText(u8"url(a.png)", Cursor, context) == "<null>");
    CHECK(ConsumeToText(u8"url(a.png) auto", Cursor, context) == "<null>");
    CHECK(ConsumeToText(u8"auto, url(a.png)", Cursor, context) == "<partial>");

    // The hot spot needs both coordinates.
    CHECK(ConsumeToText(u8"url(a.png) 1, auto", Cursor, context) == "<null>");

    // Only URLs and image-set() are allowed.
    CHECK(ConsumeToText(u8"linear-gradient(red, blue), auto", Cursor, context) == "<null>");
  }
}
