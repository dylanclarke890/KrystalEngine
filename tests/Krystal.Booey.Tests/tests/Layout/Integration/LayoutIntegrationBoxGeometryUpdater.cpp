#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxGeometryUpdater.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "BoxGeometryUpdater: form controls' baselines",
                   "[Layout][BoxGeometryUpdater]")
  {
    // Each control shares a line with an empty inline block, whose bottom (here its top) is on the baseline.
    ResolverFixture fixture {AhemDocument(
      u8"input,select,button{margin:0}span{display:inline-block;width:1px}",
      u8"<div><span id=sliderProbe></span><input type=range id=slider></div>"
      u8"<div><span id=listProbe></span><select size=2 id=list><option>a</option></select></div>"
      u8"<div><span id=menuProbe></span><select id=menu><option>a</option></select></div>"
      u8"<div><span id=fileProbe></span><input type=file id=file></div>"
      u8"<div><span id=buttonProbe></span><button id=button style='padding:5px 0 9px'>a</button></div>")};
    // A slider's baseline is its bottom; a list box's is 7 pixels above it; a menu list's and a file upload
    // control's are their text's; a button's is its text's, not its content box's bottom.
    CHECK(DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x114
  RenderBlock {HTML} at (0,0) size 800x114
    RenderBody {BODY} at (0,0) size 800x114
      RenderBlock {DIV} at (0,0) size 800x22
        RenderBlock {SPAN} at (0,20) size 1x0 id="sliderProbe"
        RenderSlider {INPUT} at (1,0) size 129x20 [color=#909090] [bgcolor=#FFFFFF] id="slider"
          RenderFlexibleBox {DIV} at (0,0) size 129x20
            RenderBlock {DIV} at (0,0) size 129x20
              RenderBlock {DIV} at (54,0) size 21x20
      RenderBlock {DIV} at (0,22) size 800x23
        RenderBlock {SPAN} at (0,16) size 1x0 id="listProbe"
        RenderListBox {SELECT} at (1,0) size 37x23 [bgcolor=#FFFFFF] [border: (1px inset #808080)] id="list"
      RenderBlock {DIV} at (0,45) size 800x22
        RenderBlock {SPAN} at (0,14) size 1x0 id="menuProbe"
        RenderMenuList {SELECT} at (1,0) size 38x22 [bgcolor=#FFFFFF] [border: (1px solid #000000)] id="menu"
          RenderBlock (anonymous) at (1,1) size 36x20
            RenderText at (5,5) size 10x10
              text run at (5,5) width 10: "a"
      RenderBlock {DIV} at (0,67) size 800x19
        RenderBlock {SPAN} at (0,12) size 1x0 id="fileProbe"
        RenderFileUploadControl {INPUT} at (1,0) size 340x19 "no file selected" id="file"
          RenderButton {INPUT} at (0,0) size 126x19 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)]
            RenderBlock (anonymous) at (8,4) size 110x10
              RenderTextFragment at (0,0) size 110x10
                text run at (0,0) width 110: "Choose File"
      RenderBlock {DIV} at (0,86) size 800x28
        RenderBlock {SPAN} at (0,15) size 1x0 id="buttonProbe"
        RenderButton {BUTTON} at (1,0) size 14x28 [color=#000000CC] [bgcolor=#C0C0C0] [border: (2px outset #C0C0C0)] id="button"
          RenderBlock (anonymous) at (2,7) size 10x10
            RenderText {#text} at (0,0) size 10x10
              text run at (0,0) width 10: "a"
)");
  }
}
