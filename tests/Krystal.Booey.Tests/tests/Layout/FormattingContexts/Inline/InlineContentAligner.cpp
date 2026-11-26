#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentAligner.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineContentAligner: text-align: justify",
                   "[Layout][InlineContentAligner]")
  {
    ResolverFixture fixture {AhemDocument(
      u8"div { width: 100px; text-align: justify } #l { text-align-last: justify }",
      u8"<div id=j>aa bb cc dd e f</div><div id=l>aa bb cc dd e f</div><div id=b>aa bb<br>cc dd ee ff gg "
      u8"hh</div>")};

    // Each line but the last (and the one before a forced break) fills the line: the 20px left over on "aa bb
    // cc" go to its two word separators. text-align-last: justify justifies the last line too.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x70
  RenderBlock {HTML} at (0,0) size 800x70
    RenderBody {BODY} at (0,0) size 800x70
      RenderBlock {DIV} at (0,0) size 100x20 id="j"
        RenderText {#text} at (0,0) size 100x20
          text run at (0,0) width 100: "aa bb cc"
          text run at (0,10) width 60: "dd e f"
      RenderBlock {DIV} at (0,20) size 100x20 id="l"
        RenderText {#text} at (0,0) size 100x20
          text run at (0,0) width 100: "aa bb cc"
          text run at (0,10) width 100: "dd e f"
      RenderBlock {DIV} at (0,40) size 100x30 id="b"
        RenderText {#text} at (0,0) size 50x10
          text run at (0,0) width 50: "aa bb"
        RenderBR {BR} at (50,0) size 0x10
        RenderText {#text} at (0,10) size 100x20
          text run at (0,10) width 100: "cc dd ee"
          text run at (0,20) width 80: "ff gg hh"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineContentAligner: ruby-align",
                   "[Layout][InlineContentAligner]")
  {
    ResolverFixture fixture {AhemDocument(
      u8"rt { font-size: 5px } #s { ruby-align: start } #b { ruby-align: space-between } #a { ruby-align: "
      u8"space-around }",
      u8"<div><ruby id=s>a<rt>xxxxxxxx</rt></ruby></div><div><ruby>a<rt>xxxxxxxx</rt></ruby></div><div><ruby "
      u8"id=b>a a<rt>xxxxxxxxxxxxxxxxxxxx</rt></ruby></div><div><ruby id=a>a "
      u8"a<rt>xxxxxxxxxxxxxxxxxxxx</rt></ruby>"
      u8"</div><div><ruby>aaaa<rt>xx</rt></ruby></div>")};

    // Under a wider annotation, start keeps the base content (and the base box) at the start, while center
    // (the initial value) centres the content in a base as wide as the annotation, space-between expands its
    // word separator and space-around adds half of an extra expansion opportunity at each end. An annotation
    // narrower than its base is centred in it.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x75
  RenderBlock {HTML} at (0,0) size 800x75
    RenderBody {BODY} at (0,0) size 800x75
      RenderBlock {DIV} at (0,0) size 800x15
        RenderInline {RUBY} at (0,5) size 10x10 id="s"
          RenderInline (generated) at (0,5) size 10x10
            RenderText {#text} at (0,5) size 10x10
              text run at (0,5) width 10: "a"
          RenderBlock {RT} at (0,0) size 40x5
            RenderText {#text} at (0,0) size 40x5
              text run at (0,0) width 40: "xxxxxxxx"
      RenderBlock {DIV} at (0,15) size 800x15
        RenderInline {RUBY} at (0,5) size 40x10
          RenderInline (generated) at (0,5) size 40x10
            RenderText {#text} at (15,5) size 10x10
              text run at (15,5) width 10: "a"
          RenderBlock {RT} at (0,0) size 40x5
            RenderText {#text} at (0,0) size 40x5
              text run at (0,0) width 40: "xxxxxxxx"
      RenderBlock {DIV} at (0,30) size 800x15
        RenderInline {RUBY} at (0,5) size 100x10 id="b"
          RenderInline (generated) at (0,5) size 100x10
            RenderText {#text} at (0,5) size 100x10
              text run at (0,5) width 100: "a a"
          RenderBlock {RT} at (0,0) size 100x5
            RenderText {#text} at (0,0) size 100x5
              text run at (0,0) width 100: "xxxxxxxxxxxxxxxxxxxx"
      RenderBlock {DIV} at (0,45) size 800x15
        RenderInline {RUBY} at (0,5) size 100x10 id="a"
          RenderInline (generated) at (0,5) size 100x10
            RenderText {#text} at (17,5) size 66x10
              text run at (17,5) width 66: "a a"
          RenderBlock {RT} at (0,0) size 100x5
            RenderText {#text} at (0,0) size 100x5
              text run at (0,0) width 100: "xxxxxxxxxxxxxxxxxxxx"
      RenderBlock {DIV} at (0,60) size 800x15
        RenderInline {RUBY} at (0,5) size 40x10
          RenderInline (generated) at (0,5) size 40x10
            RenderText {#text} at (0,5) size 40x10
              text run at (0,5) width 40: "aaaa"
          RenderBlock {RT} at (0,0) size 40x5
            RenderText {#text} at (15,0) size 10x5
              text run at (15,0) width 10: "xx"
)");
  }
}
