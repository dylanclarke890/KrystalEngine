#include "Krystal.Booey/CSS/Properties/Consumers/Grid.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::Tokens;

  namespace
  {
    template <TrackListType Type>
    constexpr auto ConsumeTrackList = [](TokenRange &tokens, PropertyParserState &state)
    {
      return ConsumeGridTrackList(tokens, state, Type);
    };

    template <AllowEmpty Allow>
    constexpr auto ConsumeLineNames = [](TokenRange &tokens, PropertyParserState &state) -> RefPtr<Value>
    {
      return ConsumeGridLineNames(tokens, state, Allow);
    };

    Maybe<GridNamedAreaMapRow> ConsumeRow(utf8_stringview css)
    {
      ParserContext context {ParserMode::HTMLStandard};
      PropertyParserState state {.Context = context};
      Tokens tokens {css};
      auto range = tokens.Tokenizer.Tokens();
      return ConsumeUnresolvedGridTemplateAreasRow(range, state);
    }
  }

  TEST_CASE("PropertyParserHelpers(IsGridBreadthIdent)", "[CSS][Properties][Consumers]")
  {
    CHECK(IsGridBreadthIdent(ValueId::MinContent));
    CHECK(IsGridBreadthIdent(ValueId::WebkitMaxContent));
    CHECK(IsGridBreadthIdent(ValueId::Auto));
    CHECK_FALSE(IsGridBreadthIdent(ValueId::FitContent));
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridLine)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeGridLine) == "auto");
    CHECK(ConsumeToText(u8"foo", ConsumeGridLine) == "foo");
    CHECK(ConsumeToText(u8"3", ConsumeGridLine) == "3");
    CHECK(ConsumeToText(u8"-2 foo", ConsumeGridLine) == "-2 foo");
    CHECK(ConsumeToText(u8"foo -2", ConsumeGridLine) == "-2 foo");
    CHECK(ConsumeToText(u8"span 2", ConsumeGridLine) == "span 2");
    CHECK(ConsumeToText(u8"span foo", ConsumeGridLine) == "span foo");
    CHECK(ConsumeToText(u8"2 span", ConsumeGridLine) == "span 2");
    CHECK(ConsumeToText(u8"span foo 2", ConsumeGridLine) == "span 2 foo");

    CHECK(ConsumeToText(u8"0", ConsumeGridLine) == "<null>");
    CHECK(ConsumeToText(u8"span", ConsumeGridLine) == "<null>");
    CHECK(ConsumeToText(u8"span -1", ConsumeGridLine) == "<null>");
    CHECK(ConsumeToText(u8"span 0", ConsumeGridLine) == "<null>");
    CHECK(ConsumeToText(u8"1.5", ConsumeGridLine) == "<null>");
    CHECK(ConsumeToText(u8"span auto", ConsumeGridLine) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridTrackSize)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"auto", ConsumeGridTrackSize) == "auto");
    CHECK(ConsumeToText(u8"min-content", ConsumeGridTrackSize) == "min-content");
    CHECK(ConsumeToText(u8"10px", ConsumeGridTrackSize) == "10px");
    CHECK(ConsumeToText(u8"25%", ConsumeGridTrackSize) == "25%");
    CHECK(ConsumeToText(u8"1fr", ConsumeGridTrackSize) == "1fr");
    CHECK(ConsumeToText(u8"2FR", ConsumeGridTrackSize) == "2fr");
    CHECK(ConsumeToText(u8"minmax(10px, 1fr)", ConsumeGridTrackSize) == "minmax(10px, 1fr)");
    CHECK(ConsumeToText(u8"minmax(auto, max-content)", ConsumeGridTrackSize) == "minmax(auto, max-content)");
    CHECK(ConsumeToText(u8"fit-content(100px)", ConsumeGridTrackSize) == "fit-content(100px)");

    CHECK(ConsumeToText(u8"-1fr", ConsumeGridTrackSize) == "<null>");
    CHECK(ConsumeToText(u8"-1px", ConsumeGridTrackSize) == "<null>");
    CHECK(ConsumeToText(u8"minmax(1fr, 10px)", ConsumeGridTrackSize) == "<null>");
    CHECK(ConsumeToText(u8"minmax(10px)", ConsumeGridTrackSize) == "<null>");
    CHECK(ConsumeToText(u8"fit-content(auto)", ConsumeGridTrackSize) == "<null>");
    CHECK(ConsumeToText(u8"none", ConsumeGridTrackSize) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridLineNames)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"[a b]", ConsumeLineNames<AllowEmpty::No>) == "[a b]");
    CHECK(ConsumeToText(u8"[]", ConsumeLineNames<AllowEmpty::No>) == "<null>");
    CHECK(ConsumeToText(u8"[]", ConsumeLineNames<AllowEmpty::Yes>) == "[]");
    CHECK(ConsumeToText(u8"[a 5]", ConsumeLineNames<AllowEmpty::No>) == "<null>");
    CHECK(ConsumeToText(u8"[span]", ConsumeLineNames<AllowEmpty::No>) == "<null>");
    CHECK(ConsumeToText(u8"a", ConsumeLineNames<AllowEmpty::No>) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridTrackList)", "[CSS][Properties][Consumers]")
  {
    auto consume = ConsumeTrackList<TrackListType::GridTemplate>;
    CHECK(ConsumeToText(u8"10px 1fr", consume) == "10px 1fr");
    CHECK(ConsumeToText(u8"[a] 10px [b c] 1fr [d]", consume) == "[a] 10px [b c] 1fr [d]");
    CHECK(ConsumeToText(u8"repeat(2, 10px [a])", consume) == "repeat(2, 10px [a])");
    CHECK(ConsumeToText(u8"repeat(auto-fill, [a] 10px)", consume) == "repeat(auto-fill, [a] 10px)");
    CHECK(ConsumeToText(u8"repeat(auto-fit, 10px) 20px", consume) == "repeat(auto-fit, 10px) 20px");

    // Auto repeat needs fixed sizes everywhere, and only one auto repeat is allowed.
    CHECK(ConsumeToText(u8"repeat(auto-fill, 1fr)", consume) == "<null>");
    CHECK(ConsumeToText(u8"repeat(auto-fill, 10px) 1fr", consume) == "<null>");
    CHECK(ConsumeToText(u8"repeat(auto-fill, minmax(10px, 1fr))", consume)
          == "repeat(auto-fill, minmax(10px, 1fr))");
    CHECK(ConsumeToText(u8"repeat(auto-fill, 10px) repeat(auto-fit, 10px)", consume) == "<null>");

    CHECK(ConsumeToText(u8"repeat(0, 10px)", consume) == "<null>");
    CHECK(ConsumeToText(u8"repeat(2)", consume) == "<null>");
    CHECK(ConsumeToText(u8"repeat(2, [a])", consume) == "<null>");
    CHECK(ConsumeToText(u8"[a]", consume) == "<null>");

    // Other track list types.
    CHECK(ConsumeToText(u8"10px auto", ConsumeTrackList<TrackListType::GridAuto>) == "10px auto");
    CHECK(ConsumeToText(u8"[a] 10px", ConsumeTrackList<TrackListType::GridAuto>) == "<null>");
    CHECK(ConsumeToText(u8"10px [a]", ConsumeTrackList<TrackListType::GridAuto>) == "<null>");
    CHECK(ConsumeToText(u8"[a] 10px", ConsumeTrackList<TrackListType::GridTemplateNoRepeat>) == "[a] 10px");
    CHECK(ConsumeToText(u8"repeat(2, 10px)", ConsumeTrackList<TrackListType::GridTemplateNoRepeat>)
          == "<null>");
    CHECK(ConsumeToText(u8"subgrid", ConsumeTrackList<TrackListType::GridAuto>) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridTrackList repeat clamping)", "[CSS][Properties][Consumers]")
  {
    // Integer repetitions are clamped so the total number of tracks stays within the grid's limit.
    CHECK(ConsumeToText(u8"repeat(99999999, 10px 20px)", ConsumeTrackList<TrackListType::GridTemplate>)
          == "repeat(500000, 10px 20px)");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridTrackList subgrid)", "[CSS][Properties][Consumers]")
  {
    auto consume = ConsumeTrackList<TrackListType::GridTemplate>;
    CHECK(ConsumeToText(u8"subgrid", consume) == "subgrid");
    CHECK(ConsumeToText(u8"subgrid [a] [] [b c]", consume) == "subgrid [a] [] [b c]");
    CHECK(ConsumeToText(u8"subgrid repeat(2, [a] [b])", consume) == "subgrid repeat(2, [a] [b])");
    CHECK(ConsumeToText(u8"subgrid repeat(auto-fill, [a])", consume) == "subgrid repeat(auto-fill, [a])");

    CHECK(ConsumeToText(u8"subgrid repeat(auto-fill, [a]) repeat(auto-fill, [b])", consume) == "<null>");
    CHECK(ConsumeToText(u8"subgrid repeat(auto-fit, [a])", consume) == "<null>");
    CHECK(ConsumeToText(u8"subgrid 10px", consume) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridTemplatesRowsOrColumns)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeGridTemplatesRowsOrColumns) == "none");
    CHECK(ConsumeToText(u8"100px repeat(2, 1fr)", ConsumeGridTemplatesRowsOrColumns)
          == "100px repeat(2, 1fr)");
    CHECK(ConsumeToText(u8"none 10px", ConsumeGridTemplatesRowsOrColumns) == "<partial>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeUnresolvedGridTemplateAreasRow)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeRow(u8"\"a b\"") == GridNamedAreaMapRow {u8"a", u8"b"});
    CHECK(ConsumeRow(u8"\"  head   head \"") == GridNamedAreaMapRow {u8"head", u8"head"});

    // Runs of dots are one null cell token.
    CHECK(ConsumeRow(u8"\"a...b\"") == GridNamedAreaMapRow {u8"a", u8".", u8"b"});
    CHECK(ConsumeRow(u8"\". .\"") == GridNamedAreaMapRow {u8".", u8"."});

    CHECK_FALSE(ConsumeRow(u8"\"a $\""));
    CHECK_FALSE(ConsumeRow(u8"\"   \""));
    CHECK_FALSE(ConsumeRow(u8"a"));
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridTemplateAreas)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"none", ConsumeGridTemplateAreas) == "none");
    CHECK(ConsumeToText(u8"\"a b\" \"c d\"", ConsumeGridTemplateAreas) == "\"a b\" \"c d\"");
    CHECK(ConsumeToText(u8"\"a a\" \"b .\"", ConsumeGridTemplateAreas) == "\"a a\" \"b .\"");
    CHECK(ConsumeToText(u8"\"a...b\"", ConsumeGridTemplateAreas) == "\"a . b\"");

    // Rows must be the same width and areas rectangular.
    CHECK(ConsumeToText(u8"\"a b\" \"c\"", ConsumeGridTemplateAreas) == "<null>");
    CHECK(ConsumeToText(u8"\"a b\" \"b a\"", ConsumeGridTemplateAreas) == "<null>");
    CHECK(ConsumeToText(u8"\"a b a\"", ConsumeGridTemplateAreas) == "<null>");
    CHECK(ConsumeToText(u8"\"\"", ConsumeGridTemplateAreas) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeGridAutoFlow)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"row", ConsumeGridAutoFlow) == "row");
    CHECK(ConsumeToText(u8"column", ConsumeGridAutoFlow) == "column");
    CHECK(ConsumeToText(u8"dense", ConsumeGridAutoFlow) == "dense");
    CHECK(ConsumeToText(u8"column dense", ConsumeGridAutoFlow) == "column dense");
    CHECK(ConsumeToText(u8"dense row", ConsumeGridAutoFlow) == "row dense");
    CHECK(ConsumeToText(u8"normal", ConsumeGridAutoFlow) == "normal");

    CHECK(ConsumeToText(u8"auto", ConsumeGridAutoFlow) == "<null>");
    CHECK(ConsumeToText(u8"row column", ConsumeGridAutoFlow) == "<partial>");
  }
}
