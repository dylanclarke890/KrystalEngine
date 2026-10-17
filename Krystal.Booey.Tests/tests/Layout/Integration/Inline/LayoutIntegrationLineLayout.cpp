#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo::layout_integration::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    // Ahem's glyphs are 1em squares, so at 10px every character is 10px wide and a line is 10px tall.
    constexpr auto AhemStyle = u8"body { margin: 0; font: 10px Ahem }";

    KRYS_NODISCARD utf8_string Dump(ResolverFixture &fixture) noexcept
    {
      return ExternalRepresentation(fixture.Frame.get(), {RenderAsTextFlag::ShowIDAndClass});
    }

    KRYS_NODISCARD utf8_string Document(utf8_stringview style, utf8_stringview body) noexcept
    {
      layout::tests::RegisterAhem();
      auto html = utf8_string {u8"<!DOCTYPE html><style>"};
      html += AhemStyle;
      html += style;
      html += u8"</style><body>";
      html += body;
      html += u8"</body>";
      return html;
    }

    KRYS_NODISCARD RenderBlockFlow &BlockFlowOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBlockFlow>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderText &FirstTextIn(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderText>(*fixture.ById(id).Renderer()->FirstChildSlow());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineLayout: wrapping and line breaks", "[Layout][LineLayout]")
  {
    ResolverFixture fixture {
      Document(u8"#w { width: 50px }", u8"<div id=w>aa bb cc dd</div><div id=br>a<br>bb</div>")};

    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x40
  RenderBlock {HTML} at (0,0) size 800x40
    RenderBody {BODY} at (0,0) size 800x40
      RenderBlock {DIV} at (0,0) size 50x20 id="w"
        RenderText {#text} at (0,0) size 50x20
          text run at (0,0) width 50: "aa bb"
          text run at (0,10) width 50: "cc dd"
      RenderBlock {DIV} at (0,20) size 800x20 id="br"
        RenderText {#text} at (0,0) size 10x10
          text run at (0,0) width 10: "a"
        RenderBR {BR} at (10,0) size 0x10
        RenderText {#text} at (0,10) size 20x10
          text run at (0,10) width 20: "bb"
)");

    CHECK(BlockFlowOf(fixture, u8"w").LineCount() == 2);
    CHECK(BlockFlowOf(fixture, u8"br").LineCount() == 2);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineLayout: inline boxes and inline blocks",
                   "[Layout][LineLayout]")
  {
    ResolverFixture fixture {Document(
      u8"#s { padding: 0 5px; border: 1px solid } #ib { display: inline-block; width: 30px; height: 30px }",
      u8"<div id=d>a <span id=s>bb</span> c <div id=ib>x</div> d</div>")};

    // The inline box's padding and border overflow the line, and the inline block's baseline is its last
    // line's.
    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x30
  RenderBlock {HTML} at (0,0) size 800x30
    RenderBody {BODY} at (0,0) size 800x30
      RenderBlock {DIV} at (0,0) size 800x30 id="d"
        RenderText {#text} at (0,0) size 20x10
          text run at (0,0) width 20: "a "
        RenderInline {SPAN} at (20,-1) size 32x12 [border: (1px solid #000000)] id="s"
          RenderText {#text} at (26,0) size 20x10
            text run at (26,0) width 20: "bb"
        RenderText {#text} at (52,0) size 30x10
          text run at (52,0) width 30: " c "
        RenderBlock {DIV} at (82,0) size 30x30 id="ib"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "x"
        RenderText {#text} at (112,0) size 20x10
          text run at (112,0) width 20: " d"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineLayout: floats and right-to-left text",
                   "[Layout][LineLayout]")
  {
    ResolverFixture fixture {Document(
      u8"#f { float: left; width: 20px; height: 25px } #d { width: 60px } #r { direction: rtl; width: 60px }",
      u8"<div id=d><div id=f></div>aa bb cc</div><div id=r>aa bb</div>")};

    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x40
  RenderBlock {HTML} at (0,0) size 800x40
    RenderBody {BODY} at (0,0) size 800x40
      RenderBlock {DIV} at (0,0) size 60x30 id="d"
        RenderBlock (floating) {DIV} at (0,0) size 20x25 id="f"
        RenderText {#text} at (20,0) size 20x30
          text run at (20,0) width 20: "aa"
          text run at (20,10) width 20: "bb"
          text run at (20,20) width 20: "cc"
      RenderBlock {DIV} at (0,30) size 60x10 id="r"
        RenderText {#text} at (10,0) size 50x10
          text run at (10,0) width 50: "aa bb"
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineLayout: preferred widths", "[Layout][LineLayout]")
  {
    ResolverFixture fixture {
      Document(u8"#f { float: left } #min { width: min-content } #max { width: max-content } #ib { display: "
               u8"inline-block }",
               u8"<div id=f>aa bbb</div><div id=min>aa bbb <span>cccc</span></div><div id=max>aa bbb</div>"
               u8"<div>x <span id=ib>aa bbbb</span></div>")};

    // The floats and blocks shrink to fit their text. #min's lines don't fit beside the float, so they start
    // below it.
    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x60
  RenderBlock {HTML} at (0,0) size 800x60
    RenderBody {BODY} at (0,0) size 800x60
      RenderBlock (floating) {DIV} at (0,0) size 60x10 id="f"
        RenderText {#text} at (0,0) size 60x10
          text run at (0,0) width 60: "aa bbb"
      RenderBlock {DIV} at (0,0) size 40x40 id="min"
        RenderText {#text} at (0,10) size 30x20
          text run at (0,10) width 20: "aa"
          text run at (0,20) width 30: "bbb"
        RenderInline {SPAN} at (0,30) size 40x10
          RenderText {#text} at (0,30) size 40x10
            text run at (0,30) width 40: "cccc"
      RenderBlock {DIV} at (0,40) size 60x10 id="max"
        RenderText {#text} at (0,0) size 60x10
          text run at (0,0) width 60: "aa bbb"
      RenderBlock {DIV} at (0,50) size 800x10
        RenderText {#text} at (0,0) size 20x10
          text run at (0,0) width 20: "x "
        RenderBlock {SPAN} at (20,0) size 70x10 id="ib"
          RenderText {#text} at (0,0) size 70x10
            text run at (0,0) width 70: "aa bbbb"
)");

    auto &text = FirstTextIn(fixture, u8"max");
    CHECK(text.MinLogicalWidth() == 30.0f);
    CHECK(text.MaxLogicalWidth() == 60.0f);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineLayout: iterators and bounding boxes",
                   "[Layout][LineLayout]")
  {
    ResolverFixture fixture {Document(u8"#d { width: 50px; padding: 5px }",
                                      u8"<div id=d>aa <span id=s>bb cc</span><br id=br>dd</div>")};
    (void)fixture.Document->UpdateLayout();

    auto &block = BlockFlowOf(fixture, u8"d");
    REQUIRE(block.InlineLayout());
    CHECK(block.LineCount() == 3);

    auto line = inline_iterator::FirstLineBoxFor(block);
    REQUIRE(line);
    CHECK(line->LogicalTop() == 5.0f);
    CHECK(line->LogicalBottom() == 15.0f);
    line.TraverseNext();
    REQUIRE(line);
    CHECK(line->LogicalTop() == 15.0f);
    line.TraverseNext();
    REQUIRE(line);
    line.TraverseNext();
    CHECK_FALSE(line);

    auto &text = FirstTextIn(fixture, u8"d");
    CHECK(text.LinesBoundingBox() == IntRect(5, 5, 30, 10));
    CHECK(text.FirstRunLocation() == IntPoint(5, 5));
    auto textBoxCount = 0;
    for (auto &textBox : inline_iterator::TextBoxesFor(text))
    {
      CHECK(textBox.OriginalText() == u8"aa ");
      ++textBoxCount;
    }
    CHECK(textBoxCount == 1);

    // The inline box's bounding box spans both of its lines.
    auto &span = Downcast<RenderInline>(*fixture.ById(u8"s").Renderer());
    CHECK(span.LinesBoundingBox() == IntRect(5, 5, 50, 20));
    auto &lineBreak = Downcast<RenderLineBreak>(*fixture.ById(u8"br").Renderer());
    CHECK(lineBreak.LinesBoundingBox() == IntRect(25, 15, 0, 10));
  }
}
