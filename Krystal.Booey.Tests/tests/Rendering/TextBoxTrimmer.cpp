#include "Krystal.Booey/Rendering/TextBoxTrimmer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "TextBoxTrimmer: trimming the first and last formatted lines",
                   "[Rendering][TextBoxTrimmer]")
  {
    // A 20px line height gives each line 5px of half-leading over and under Ahem's 8px ascent and 2px
    // descent.
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"div { line-height: 20px } #s { text-box-trim: trim-start } #e { text-box-trim: trim-end } #b { "
      u8"text-box-trim: trim-both } #c { text-box: trim-both cap alphabetic }",
      u8"<div id=s>aa<br>bb</div><div id=e>aa<br>bb</div><div id=b>aa<br>bb</div><div id=c>aa<br>bb</div>")};

    // Trimming the start removes the first line's half-leading (and with cap, the space over the cap height);
    // trimming the end removes the last line's half-leading (and with alphabetic, the descent).
    CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x128
  RenderBlock {HTML} at (0,0) size 800x128
    RenderBody {BODY} at (0,0) size 800x128
      RenderBlock {DIV} at (0,0) size 800x35 id="s"
        RenderText {#text} at (0,0) size 20x10
          text run at (0,0) width 20: "aa"
        RenderBR {BR} at (20,0) size 0x10
        RenderText {#text} at (0,20) size 20x10
          text run at (0,20) width 20: "bb"
      RenderBlock {DIV} at (0,35) size 800x35 id="e"
        RenderText {#text} at (0,5) size 20x10
          text run at (0,5) width 20: "aa"
        RenderBR {BR} at (20,5) size 0x10
        RenderText {#text} at (0,25) size 20x10
          text run at (0,25) width 20: "bb"
      RenderBlock {DIV} at (0,70) size 800x30 id="b"
        RenderText {#text} at (0,0) size 20x10
          text run at (0,0) width 20: "aa"
        RenderBR {BR} at (20,0) size 0x10
        RenderText {#text} at (0,20) size 20x10
          text run at (0,20) width 20: "bb"
      RenderBlock {DIV} at (0,100) size 800x28 id="c"
        RenderText {#text} at (0,0) size 20x10
          text run at (0,0) width 20: "aa"
        RenderBR {BR} at (20,0) size 0x10
        RenderText {#text} at (0,20) size 20x10
          text run at (0,20) width 20: "bb"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TextBoxTrimmer: nested block containers",
                   "[Rendering][TextBoxTrimmer]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"div { line-height: 20px } .t { text-box-trim: trim-both } #p { padding-top: 1px } #i { display: "
      u8"inline-block }",
      u8"<div class=t id=a><div>aa</div><div>bb</div></div><div class=t id=b><div id=p>aa</div><div>bb</div>"
      u8"</div><div class=t id=c><div id=i>aa</div></div>")};

    // The trim reaches the first and last formatted lines of nested blocks. Padding prevents trimming the
    // start, and the trim doesn't reach into an inline-block (a new formatting context), though the line it
    // sits on is trimmed.
    CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x76
  RenderBlock {HTML} at (0,0) size 800x76
    RenderBody {BODY} at (0,0) size 800x76
      RenderBlock {DIV} at (0,0) size 800x30 id="a" class="t"
        RenderBlock {DIV} at (0,0) size 800x15
          RenderText {#text} at (0,0) size 20x10
            text run at (0,0) width 20: "aa"
        RenderBlock {DIV} at (0,15) size 800x15
          RenderText {#text} at (0,5) size 20x10
            text run at (0,5) width 20: "bb"
      RenderBlock {DIV} at (0,30) size 800x36 id="b" class="t"
        RenderBlock {DIV} at (0,0) size 800x21 id="p"
          RenderText {#text} at (0,6) size 20x10
            text run at (0,6) width 20: "aa"
        RenderBlock {DIV} at (0,21) size 800x15
          RenderText {#text} at (0,5) size 20x10
            text run at (0,5) width 20: "bb"
      RenderBlock {DIV} at (0,66) size 800x10 id="c" class="t"
        RenderBlock {DIV} at (0,-5) size 20x20 id="i"
          RenderText {#text} at (0,5) size 20x10
            text run at (0,5) width 20: "aa"
)");
  }
}
