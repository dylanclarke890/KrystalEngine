#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentConstrainer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineContentConstrainer: text-wrap: balance",
                   "[Layout][InlineContentConstrainer]")
  {
    ResolverFixture fixture {AhemDocument(
      u8"div { width: 100px } .b { text-wrap: balance } #f { float: left; width: 10px; height: 10px }",
      u8"<div>aaa bb cc dd</div><div class=b id=b>aaa bb cc dd</div><div class=b id=c>aaa bb cc dd<br>aa bb "
      u8"cc "
      u8"dd e</div><div class=b id=n><div id=f></div>aaa bb cc dd</div><div class=b id=w "
      u8"style=\"text-wrap-mode: "
      u8"nowrap\">aaa bb cc dd</div>")};

    // Balancing evens out the lines' widths and keeps the number of lines. Each chunk between forced breaks
    // is balanced on its own. Content beside floats and content that doesn't wrap isn't balanced.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x110
  RenderBlock {HTML} at (0,0) size 800x110
    RenderBody {BODY} at (0,0) size 800x110
      RenderBlock {DIV} at (0,0) size 100x20
        RenderText {#text} at (0,0) size 90x20
          text run at (0,0) width 90: "aaa bb cc"
          text run at (0,10) width 20: "dd"
      RenderBlock {DIV} at (0,20) size 100x20 id="b" class="b"
        RenderText {#text} at (0,0) size 60x20
          text run at (0,0) width 60: "aaa bb"
          text run at (0,10) width 50: "cc dd"
      RenderBlock {DIV} at (0,40) size 100x40 id="c" class="b"
        RenderText {#text} at (0,0) size 60x20
          text run at (0,0) width 60: "aaa bb"
          text run at (0,10) width 50: "cc dd"
        RenderBR {BR} at (50,10) size 0x10
        RenderText {#text} at (0,20) size 70x20
          text run at (0,20) width 50: "aa bb"
          text run at (0,30) width 70: "cc dd e"
      RenderBlock {DIV} at (0,80) size 100x20 id="n" class="b"
        RenderBlock (floating) {DIV} at (0,0) size 10x10 id="f"
        RenderText {#text} at (0,0) size 100x20
          text run at (10,0) width 90: "aaa bb cc"
          text run at (0,10) width 20: "dd"
      RenderBlock {DIV} at (0,100) size 100x10 id="w" class="b"
        RenderText {#text} at (0,0) size 120x10
          text run at (0,0) width 120: "aaa bb cc dd"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineContentConstrainer: text-wrap: pretty",
                   "[Layout][InlineContentConstrainer]")
  {
    ResolverFixture fixture {
      AhemDocument(u8"div { width: 100px } #p { text-wrap: pretty }",
                   u8"<div>aa bb cc dd ee ff g</div><div id=p>aa bb cc dd ee ff g</div>")};

    // Pretty avoids the short last line ("g") by breaking the earlier lines sooner.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x50
  RenderBlock {HTML} at (0,0) size 800x50
    RenderBody {BODY} at (0,0) size 800x50
      RenderBlock {DIV} at (0,0) size 100x20
        RenderText {#text} at (0,0) size 100x20
          text run at (0,0) width 80: "aa bb cc"
          text run at (0,10) width 100: "dd ee ff g"
      RenderBlock {DIV} at (0,20) size 100x30 id="p"
        RenderText {#text} at (0,0) size 70x30
          text run at (0,0) width 50: "aa bb"
          text run at (0,10) width 50: "cc dd"
          text run at (0,20) width 70: "ee ff g"
)");
  }
}
