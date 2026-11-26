#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Css/Geometry/DOMPointReadOnly.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Svg/Elements/SVGGraphicsElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGTextContentElement.hpp"
#include "Krystal.Booey/Svg/SVGRect.hpp"
#include <string>

namespace krys::boo::layout_integration::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD utf8_string DumpInlineLayout(ResolverFixture &fixture) noexcept
    {
      return layout::tests::DumpWithIds(fixture);
    }

    KRYS_NODISCARD utf8_string Document(utf8_stringview style, utf8_stringview body) noexcept
    {
      return layout::tests::AhemDocument(style, body);
    }

    KRYS_NODISCARD RenderBlockFlow &BlockFlowOf(ResolverFixture &fixture, DOMStringView id) noexcept
    {
      return Downcast<RenderBlockFlow>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderText &FirstTextIn(ResolverFixture &fixture, DOMStringView id) noexcept
    {
      return Downcast<RenderText>(*fixture.ById(id).Renderer()->FirstChildSlow());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LineLayout: wrapping and line breaks", "[Layout][LineLayout]")
  {
    ResolverFixture fixture {
      Document(u8"#w { width: 50px }", u8"<div id=w>aa bb cc dd</div><div id=br>a<br>bb</div>")};

    CHECK(DumpInlineLayout(fixture) == u8R"(layer at (0,0) size 800x600
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
    CHECK(DumpInlineLayout(fixture) == u8R"(layer at (0,0) size 800x600
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

    CHECK(DumpInlineLayout(fixture) == u8R"(layer at (0,0) size 800x600
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
    CHECK(DumpInlineLayout(fixture) == u8R"(layer at (0,0) size 800x600
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
    CHECK(text.LinesBoundingBox() == RectI(5, 5, 30, 10));
    CHECK(text.FirstRunLocation() == PointI(5, 5));
    auto textBoxCount = 0;
    for (auto &textBox : inline_iterator::TextBoxesFor(text))
    {
      CHECK(textBox.OriginalText() == u8"aa ");
      ++textBoxCount;
    }
    CHECK(textBoxCount == 1);

    // The inline box's bounding box spans both of its lines.
    auto &span = Downcast<RenderInline>(*fixture.ById(u8"s").Renderer());
    CHECK(span.LinesBoundingBox() == RectI(5, 5, 50, 20));
    auto &lineBreak = Downcast<RenderLineBreak>(*fixture.ById(u8"br").Renderer());
    CHECK(lineBreak.LinesBoundingBox() == RectI(25, 15, 0, 10));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "LineLayout: lays out SVG text with the inline formatting context",
                   "[Layout][LineLayout][SVG]")
  {
    // The text queries' answers, which matched the legacy line layout's before it was removed.
    struct Page
    {
      utf8_stringview Markup;
      std::string_view Expected;
    };

    const Page pages[] = {
      {u8"<text id=t x='10 20 30' y='20 25' dx=3 rotate='0 10'>Hello <tspan fill=red "
       u8"dy=5>world</tspan></text>",
       "length 79.140625, box 13.000000 5.515625 78.078125 29.843750, 13.000000 20.000000, "
       "20.000000 25.000000, 30.000000 25.000000, 33.554688 25.000000, 37.109375 25.000000, "
       "46.007812 25.000000, 50.453125 30.000000, 62.007812 30.000000, 70.906250 30.000000, "
       "76.234375 30.000000, 79.789062 30.000000"},
      {u8"<text id=t x=100 y=40 text-anchor=middle font-size=20>centred <tspan "
       u8"font-weight=bold>bold</tspan></text>",
       "length 110.078125, box 44.953125 21.890625 110.093750 22.359375, 44.960938 40.000000, "
       "54.960938 40.000000, 66.083984 40.000000, 77.207031 40.000000, 82.763672 40.000000, "
       "89.423828 40.000000, 100.546875 40.000000, 111.669922 40.000000, 117.226562 40.000000, "
       "128.349609 40.000000, 139.472656 40.000000, 143.916016 40.000000"},
      {u8"<path id=p d='M10 80 Q 100 0 190 80' fill=none /><text id=t><textPath href='#p'>along the "
       u8"path</textPath></text>",
       "length 101.421875, box 0.562500 25.421875 100.250000 57.125000, 9.970817 79.966331, "
       "16.757298 74.210892, 19.496681 71.943604, 26.582964 66.559212, 33.905529 61.499470, "
       "41.509548 56.877476, 45.388000 54.704323, 49.312889 52.611694, 57.392918 48.874626, "
       "65.733894 45.773430, 69.954491 44.363754, 78.585030 42.178680, 87.369972 40.759083, "
       "91.789825 40.240345"},
      {u8"<text id=t x=50 y=10 writing-mode=vertical-rl>vertical</text>",
       "length 143.000000, box 41.062500 10.000000 17.875000 50.687500, 44.453125 10.000000, "
       "44.453125 18.000000, 44.453125 26.898438, 44.453125 32.226562, 44.453125 36.671875, "
       "44.453125 40.226562, 44.453125 48.226562, 44.453125 57.125000"},
      {u8"<text id=t x=10 y=50 direction=rtl>abc \u05D0\u05D1\u05D2 def</text>",
       "length 83.312500, box -73.312500 35.515625 83.312500 17.875000, -73.312500 50.000000, "
       "-64.414062 50.000000, -55.515625 50.000000, -51.070312 50.000000, -46.625000 50.000000, "
       "-46.625000 50.000000, -36.578125 50.000000, -36.578125 50.000000, -27.007812 50.000000, "
       "-27.007812 50.000000, -20.242188 50.000000, -15.796875 50.000000, -6.898438 50.000000, "
       "2.000000 50.000000"},
      {u8"<text id=t x=10 y=50 textLength=150 lengthAdjust=spacingAndGlyphs>stretched</text>",
       "length 65.812500, box 10.000000 35.515625 150.000000 17.875000, 10.000000 50.000000, "
       "18.000000 50.000000, 22.445312 50.000000, 27.773438 50.000000, 36.671875 50.000000, "
       "41.117188 50.000000, 49.117188 50.000000, 58.015625 50.000000, 66.914062 50.000000"},
      {u8"<text id=t x=10 y=50 style='letter-spacing: 3px; text-decoration: underline'>deco <a href='#'>"
       u8"<tspan>link</tspan></a></text>",
       "length 63.148438, box 10.000000 35.515625 87.156250 17.875000, 10.000000 50.000000, "
       "21.898438 50.000000, 33.796875 50.000000, 44.796875 50.000000, 56.695312 50.000000, "
       "64.140625 50.000000, 70.695312 50.000000, 77.250000 50.000000, 89.148438 50.000000"},
    };

    for (const auto &page : pages)
    {
      INFO(reinterpret_cast<const char *>(utf8_string(page.Markup).c_str()));
      boo::tests::PaintFixture fixture {
        u8"<!DOCTYPE html><body style=\"margin: 0\"><svg width=200 height=100>" + utf8_string(page.Markup)
        + u8"</svg></body>"};
      std::ignore = fixture.Painted();

      auto &element = Downcast<SVGElement>(*fixture.Document->GetElementById(u8"t"));
      CHECK(Downcast<RenderBlockFlow>(*element.Renderer()).InlineLayout() != nullptr);

      auto &text = Downcast<SVGTextContentElement>(element);
      auto box = Downcast<SVGGraphicsElement>(element).GetBBox();
      std::string result = "length " + std::to_string(text.GetComputedTextLength()) + ", box "
                           + std::to_string(box.X()) + " " + std::to_string(box.Y()) + " "
                           + std::to_string(box.Width()) + " " + std::to_string(box.Height());
      for (uint32 index = 0; index < text.GetNumberOfChars(); ++index)
      {
        auto start = text.GetStartPositionOfChar(index);
        if (!start.HasException())
        {
          auto point = start.ReleaseValue();
          result += ", " + std::to_string(point->X()) + " " + std::to_string(point->Y());
        }
      }
      CHECK(result == page.Expected);
    }
  }
}
