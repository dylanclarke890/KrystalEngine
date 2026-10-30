#include "Krystal.Booey/Rendering/LineClampUpdater.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineClampUpdater: line-clamp", "[Rendering][LineClampUpdater]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"div { width: 30px } #c { line-clamp: 2 } #n { line-clamp: 3 } #n div { width: auto } #i { "
      u8"line-clamp: "
      u8"1 } #l { contain: layout }",
      u8"<div id=c>aa bb cc dd</div><div id=n><div>aa</div><div>bb cc</div><div>dd</div></div><div id=i><div "
      u8"id=l>aa bb</div>cc</div>")};

    // The line-clamp shorthand discards the content after the clamp. The clamp carries on through nested
    // blocks: the third line ends it and the last block has no lines left. The lines of a block that
    // establishes an independent formatting context aren't counted.
    CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x80
  RenderBlock {HTML} at (0,0) size 800x80
    RenderBody {BODY} at (0,0) size 800x80
      RenderBlock {DIV} at (0,0) size 30x20 id="c"
        RenderText {#text} at (0,0) size 20x20
          text run at (0,0) width 20: "aa"
          text run at (0,10) width 20: "bb"
      RenderBlock {DIV} at (0,20) size 30x30 id="n"
        RenderBlock {DIV} at (0,0) size 30x10
          RenderText {#text} at (0,0) size 20x10
            text run at (0,0) width 20: "aa"
        RenderBlock {DIV} at (0,10) size 30x20
          RenderText {#text} at (0,0) size 20x20
            text run at (0,0) width 20: "bb"
            text run at (0,10) width 20: "cc"
        RenderBlock {DIV} at (0,30) size 30x0
          RenderText {#text} at (0,0) size 0x0
      RenderBlock {DIV} at (0,50) size 30x30 id="i"
        RenderBlock (anonymous) at (0,20) size 30x10
          RenderText {#text} at (0,0) size 20x10
            text run at (0,0) width 20: "cc"
layer at (0,50) size 30x20
  RenderBlock {DIV} at (0,0) size 30x20 id="l"
    RenderText {#text} at (0,0) size 20x20
      text run at (0,0) width 20: "aa"
      text run at (0,10) width 20: "bb"
)");
  }
}
