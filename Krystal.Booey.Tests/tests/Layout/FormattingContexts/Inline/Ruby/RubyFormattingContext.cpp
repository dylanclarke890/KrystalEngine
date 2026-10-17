#include "Krystal.Booey/Layout/FormattingContexts/Inline/Ruby/RubyFormattingContext.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RubyFormattingContext: annotations over and under the base",
                   "[Layout][Ruby]")
  {
    ResolverFixture fixture {AhemDocument(
      u8"rt { font-size: 5px } #u { ruby-position: under } #w { width: 40px }",
      u8"<div>a<ruby>bb<rt>xx</rt></ruby>c</div><div>a<ruby id=u>bb<rt>xx</rt></ruby>c</div><div id=w>aa "
      u8"<ruby>bb<rt>xx</rt></ruby> cc <ruby>dd<rt>yy</rt></ruby></div>")};

    // The annotation sits over (or under) its base and makes the line taller. On later lines it may use the
    // gap between the lines instead.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x80
  RenderBlock {HTML} at (0,0) size 800x80
    RenderBody {BODY} at (0,0) size 800x80
      RenderBlock {DIV} at (0,0) size 800x15
        RenderText {#text} at (0,5) size 10x10
          text run at (0,5) width 10: "a"
        RenderInline {RUBY} at (10,5) size 20x10
          RenderInline (generated) at (10,5) size 20x10
            RenderText {#text} at (10,5) size 20x10
              text run at (10,5) width 20: "bb"
          RenderBlock {RT} at (10,0) size 20x5
            RenderText {#text} at (5,0) size 10x5
              text run at (5,0) width 10: "xx"
        RenderText {#text} at (30,5) size 10x10
          text run at (30,5) width 10: "c"
      RenderBlock {DIV} at (0,15) size 800x15
        RenderText {#text} at (0,0) size 10x10
          text run at (0,0) width 10: "a"
        RenderInline {RUBY} at (10,0) size 20x10 id="u"
          RenderInline (generated) at (10,0) size 20x10
            RenderText {#text} at (10,0) size 20x10
              text run at (10,0) width 20: "bb"
          RenderBlock {RT} at (10,10) size 20x5
            RenderText {#text} at (5,0) size 10x5
              text run at (5,0) width 10: "xx"
        RenderText {#text} at (30,0) size 10x10
          text run at (30,0) width 10: "c"
      RenderBlock {DIV} at (0,30) size 40x50 id="w"
        RenderText {#text} at (0,0) size 20x10
          text run at (0,0) width 20: "aa"
        RenderInline {RUBY} at (0,15) size 20x10
          RenderInline (generated) at (0,15) size 20x10
            RenderText {#text} at (0,15) size 20x10
              text run at (0,15) width 20: "bb"
          RenderBlock {RT} at (0,10) size 20x5
            RenderText {#text} at (5,0) size 10x5
              text run at (5,0) width 10: "xx"
        RenderText {#text} at (0,25) size 20x10
          text run at (0,25) width 20: "cc"
        RenderInline {RUBY} at (0,40) size 20x10
          RenderInline (generated) at (0,40) size 20x10
            RenderText {#text} at (0,40) size 20x10
              text run at (0,40) width 20: "dd"
          RenderBlock {RT} at (0,35) size 20x5
            RenderText {#text} at (5,0) size 10x5
              text run at (5,0) width 10: "yy"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RubyFormattingContext: wide annotations and overhang",
                   "[Layout][Ruby]")
  {
    ResolverFixture fixture {
      AhemDocument(u8"rt { font-size: 5px } #n ruby { ruby-overhang: none }",
                   u8"<div>aa<ruby>b<rt>xxxxxx</rt></ruby>cc</div><div id=n>aa<ruby>b<rt>"
                   u8"xxxxxx</rt></ruby>cc</div>")};

    // A base is at least as wide as its annotation. The annotation may overhang the adjacent text by up to
    // half of its font size, unless ruby-overhang is none.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x30
  RenderBlock {HTML} at (0,0) size 800x30
    RenderBody {BODY} at (0,0) size 800x30
      RenderBlock {DIV} at (0,0) size 800x15
        RenderText {#text} at (0,5) size 20x10
          text run at (0,5) width 20: "aa"
        RenderInline {RUBY} at (20,5) size 30x10
          RenderInline (generated) at (17,5) size 31x10
            RenderText {#text} at (27,5) size 11x10
              text run at (27,5) width 11: "b"
          RenderBlock {RT} at (17,0) size 31x5
            RenderText {#text} at (0,0) size 30x5
              text run at (0,0) width 30: "xxxxxx"
        RenderText {#text} at (45,5) size 20x10
          text run at (45,5) width 20: "cc"
      RenderBlock {DIV} at (0,15) size 800x15 id="n"
        RenderText {#text} at (0,5) size 20x10
          text run at (0,5) width 20: "aa"
        RenderInline {RUBY} at (20,5) size 30x10
          RenderInline (generated) at (20,5) size 30x10
            RenderText {#text} at (30,5) size 10x10
              text run at (30,5) width 10: "b"
          RenderBlock {RT} at (20,0) size 30x5
            RenderText {#text} at (0,0) size 30x5
              text run at (0,0) width 30: "xxxxxx"
        RenderText {#text} at (50,5) size 20x10
          text run at (50,5) width 20: "cc"
)");
  }
}
