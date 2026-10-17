#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderRuby.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTreeBuilder::Ruby: anonymous ruby boxes",
                   "[Rendering][RenderTreeBuilder][Ruby]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"rt { font-size: 5px } #k { float: left }",
      u8"<div><ruby id=a>a<rt>x</rt>b<rt>y</rt></ruby></div><div><ruby id=e><rt>x</rt></ruby></div><ruby "
      u8"id=k>c<rt>z</rt></ruby>")};

    // Content of a ruby container that isn't in a base is wrapped in an anonymous base, an annotation without
    // a base gets an empty anonymous one, and a blockified ruby gets an anonymous inline ruby container.
    CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x45
  RenderBlock {HTML} at (0,0) size 800x45
    RenderBody {BODY} at (0,0) size 800x30
      RenderBlock {DIV} at (0,0) size 800x15
        RenderInline {RUBY} at (0,5) size 20x10 id="a"
          RenderInline (generated) at (0,5) size 10x10
            RenderText {#text} at (0,5) size 10x10
              text run at (0,5) width 10: "a"
          RenderBlock {RT} at (0,0) size 10x5
            RenderText {#text} at (2,0) size 6x5
              text run at (2,0) width 6: "x"
          RenderInline (generated) at (10,5) size 10x10
            RenderText {#text} at (10,5) size 10x10
              text run at (10,5) width 10: "b"
          RenderBlock {RT} at (10,0) size 10x5
            RenderText {#text} at (2,0) size 6x5
              text run at (2,0) width 6: "y"
      RenderBlock {DIV} at (0,15) size 800x15
        RenderInline {RUBY} at (0,5) size 5x10 id="e"
          RenderInline (generated) at (0,5) size 5x10
          RenderBlock {RT} at (0,0) size 5x5
            RenderText {#text} at (0,0) size 5x5
              text run at (0,0) width 5: "x"
      RenderBlock (floating) {RUBY} at (0,30) size 10x15 id="k"
        RenderInline (generated) at (0,5) size 10x10
          RenderInline (generated) at (0,5) size 10x10
            RenderText {#text} at (0,5) size 10x10
              text run at (0,5) width 10: "c"
          RenderBlock {RT} at (0,0) size 10x5
            RenderText {#text} at (2,0) size 6x5
              text run at (2,0) width 6: "z"
)");

    // Anonymous ruby boxes are bidi-isolated, and bases don't wrap.
    auto &ruby = *fixture.ById(u8"a").Renderer();
    auto &base = *ruby.FirstChild();
    CHECK(base.IsAnonymous());
    CHECK(base.Style().Display() == DisplayType::RubyBase);
    CHECK(base.Style().GetUnicodeBidi() == UnicodeBidi::Isolate);
    CHECK(base.Style().GetTextWrapMode() == TextWrapMode::NoWrap);
    auto &container = *fixture.ById(u8"k").Renderer()->FirstChild();
    CHECK(container.IsAnonymous());
    CHECK(container.Style().Display() == DisplayType::Ruby);
    CHECK(container.Style().GetUnicodeBidi() == UnicodeBidi::Isolate);
    CHECK(container.Style().GetTextWrapMode() == TextWrapMode::Wrap);

    SECTION("Text added to a ruby container goes in a base")
    {
      REQUIRE_FALSE(fixture.ById(u8"a").AppendChild(*fixture.Document->CreateTextNode(u8"d")).HasException());
      CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x45
  RenderBlock {HTML} at (0,0) size 800x45
    RenderBody {BODY} at (0,0) size 800x30
      RenderBlock {DIV} at (0,0) size 800x15
        RenderInline {RUBY} at (0,5) size 30x10 id="a"
          RenderInline (generated) at (0,5) size 10x10
            RenderText {#text} at (0,5) size 10x10
              text run at (0,5) width 10: "a"
          RenderBlock {RT} at (0,0) size 10x5
            RenderText {#text} at (2,0) size 6x5
              text run at (2,0) width 6: "x"
          RenderInline (generated) at (10,5) size 10x10
            RenderText {#text} at (10,5) size 10x10
              text run at (10,5) width 10: "b"
          RenderBlock {RT} at (10,0) size 10x5
            RenderText {#text} at (2,0) size 6x5
              text run at (2,0) width 6: "y"
          RenderInline (generated) at (20,5) size 10x10
            RenderText {#text} at (20,5) size 10x10
              text run at (20,5) width 10: "d"
      RenderBlock {DIV} at (0,15) size 800x15
        RenderInline {RUBY} at (0,5) size 5x10 id="e"
          RenderInline (generated) at (0,5) size 5x10
          RenderBlock {RT} at (0,0) size 5x5
            RenderText {#text} at (0,0) size 5x5
              text run at (0,0) width 5: "x"
      RenderBlock (floating) {RUBY} at (0,30) size 10x15 id="k"
        RenderInline (generated) at (0,5) size 10x10
          RenderInline (generated) at (0,5) size 10x10
            RenderText {#text} at (0,5) size 10x10
              text run at (0,5) width 10: "c"
          RenderBlock {RT} at (0,0) size 10x5
            RenderText {#text} at (2,0) size 6x5
              text run at (2,0) width 6: "z"
)");
    }
  }
}
