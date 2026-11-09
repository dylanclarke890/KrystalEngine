#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD utf8_string Dump(ResolverFixture &fixture) noexcept
    {
      return ExternalRepresentation(fixture.Frame.get(),
                                    {RenderAsTextFlag::ShowAllLayers, RenderAsTextFlag::ShowIDAndClass});
    }

    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderBlockFlow &BlockOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBlockFlow>(*fixture.ById(id).Renderer());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FloatingObjects: floats among block children",
                   "[Rendering][FloatingObjects]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #c { width: 300px; border: 1px solid }
      .l { float: left; width: 50px; height: 30px }
      #r1 { float: right; width: 60px; height: 20px }
      #bfc { overflow: hidden; height: 10px }
      #clear { clear: both; height: 10px }
    </style><body><div id=c><div class=l id=l1></div><div class=l id=l2></div><div id=r1></div><div id=bfc></div><div id=clear></div></div></body>)html"};

    // Floats stack along the line; the block formatting context root shrinks to avoid them; the clearing
    // block moves below them.
    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x42
  RenderBlock {HTML} at (0,0) size 800x42
    RenderBody {BODY} at (0,0) size 800x42
      RenderBlock {DIV} at (0,0) size 302x42 [border: (1px solid #000000)] id="c"
        RenderBlock (floating) {DIV} at (1,1) size 50x30 id="l1" class="l"
        RenderBlock (floating) {DIV} at (51,1) size 50x30 id="l2" class="l"
        RenderBlock (floating) {DIV} at (241,1) size 60x20 id="r1"
        RenderBlock {DIV} at (1,31) size 300x10 id="clear"
layer at (101,1) size 140x10
  RenderBlock {DIV} at (101,1) size 140x10 id="bfc"
)");

    auto &container = BlockOf(fixture, u8"c");
    CHECK(container.ContainsFloats());
    CHECK(container.ContainsFloat(BoxOf(fixture, u8"l1")));
    CHECK(container.LowestFloatLogicalBottom() == 31_lu);
    CHECK(container.LowestFloatLogicalBottom(FloatingObject::FloatRight) == 21_lu);
    // The line offsets avoid the floats where they are.
    CHECK(container.LogicalLeftOffsetForLine(5_lu, 1_lu) == 101_lu);
    CHECK(container.LogicalRightOffsetForLine(5_lu, 1_lu) == 241_lu);
    CHECK(container.LogicalRightOffsetForLine(25_lu, 1_lu) == 301_lu);
    CHECK(container.LogicalLeftOffsetForLine(35_lu, 1_lu) == 1_lu);

    SECTION("A removed float leaves its block")
    {
      auto *removed = &BoxOf(fixture, u8"l1");
      REQUIRE_FALSE(fixture.ById(u8"l1").Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(container.ContainsFloat(*removed));
      CHECK(BoxOf(fixture, u8"l2").Location() == LayoutPoint(1, 1));
      CHECK(BoxOf(fixture, u8"bfc").FrameRect() == LayoutRect(51, 1, 190, 10));
    }

    SECTION("A float that stops floating")
    {
      REQUIRE_FALSE(fixture.ById(u8"r1").SetAttribute(u8"style", u8"float: none").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(container.ContainsFloat(BoxOf(fixture, u8"r1")));
      CHECK(BoxOf(fixture, u8"r1").Location() == LayoutPoint(1, 1));
      CHECK(BoxOf(fixture, u8"bfc").FrameRect() == LayoutRect(101, 21, 200, 10));
      CHECK(BoxOf(fixture, u8"clear").Y() == 31_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FloatingObjects: overhanging and intruding floats",
                   "[Rendering][FloatingObjects]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      .f { float: left; width: 40px; height: 50px }
      #next { clear: left; height: 5px }
      #root { display: flow-root }
      #root > div { float: right; width: 10px; height: 25px }
      #narrow { width: 100px }
      #narrow > div { float: left; width: 60px; height: 30px; margin-bottom: 2px }
      #c2 { width: 200px }
      #c2 > .a { float: left; width: 20px; height: 20px }
      #c2 > .b { float: right; width: 20px; height: 10px; clear: left }
    </style><body><div id=o><div class=f id=f1></div></div><div id=next></div><div id=root><div></div></div><div id=narrow><div id=n1></div><div id=n2></div></div><div id=c2><div class=a></div><div class=b id=b1></div></div></body>)html"};

    // #f1 overhangs #o, which #next clears; the flow-root encloses its float; #n2 drops below #n1's margin
    // box; #n1 and #n2 intrude into #c2, where .a can't start above #n2 (CSS 2.1 9.5.1 rule 5) and .b clears
    // them; the root encloses the floats that overhang the body.
    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x154
  RenderBlock {HTML} at (0,0) size 800x154
    RenderBody {BODY} at (0,0) size 800x80
      RenderBlock {DIV} at (0,0) size 800x0 id="o"
        RenderBlock (floating) {DIV} at (0,0) size 40x50 id="f1" class="f"
      RenderBlock {DIV} at (0,50) size 800x5 id="next"
      RenderBlock {DIV} at (0,55) size 800x25 id="root"
        RenderBlock (floating) {DIV} at (790,0) size 10x25
      RenderBlock {DIV} at (0,80) size 100x0 id="narrow"
        RenderBlock (floating) {DIV} at (0,0) size 60x30 id="n1"
        RenderBlock (floating) {DIV} at (0,32) size 60x30 id="n2"
      RenderBlock {DIV} at (0,80) size 200x0 id="c2"
        RenderBlock (floating) {DIV} at (60,32) size 20x20 class="a"
        RenderBlock (floating) {DIV} at (180,64) size 20x10 id="b1" class="b"
)");

    auto &body = Downcast<RenderBlockFlow>(*fixture.Document->Body()->Renderer());
    CHECK(body.ContainsFloat(BoxOf(fixture, u8"f1")));
    CHECK(BlockOf(fixture, u8"c2").ContainsFloat(BoxOf(fixture, u8"n2")));
    CHECK_FALSE(BlockOf(fixture, u8"root").ContainsFloat(BoxOf(fixture, u8"f1")));

    SECTION("Removing an overhanging float")
    {
      REQUIRE_FALSE(fixture.ById(u8"f1").Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"next").Y() == 0_lu);
      CHECK(BoxOf(fixture, u8"root").Y() == 5_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FloatingObjects: floats in inline content",
                   "[Rendering][FloatingObjects]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0; font: 10px/10px monospace }
      div { width: 200px }
      .f { float: left; width: 30px; height: 30px }
      .r { float: right; width: 20px; height: 15px }
      #fits { clear: both }
      #below { clear: both; width: 60px }
    </style><body><div id=line>one<br>two <span class=f id=f1></span>three</div><div id=fits>x<div class=f id=f2></div><div class=r id=r2></div></div><div id=below>aaaa aaaa <span class=f id=f3></span>b</div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // A float is placed on the line it's on, and that line's content moves aside.
    CHECK(BoxOf(fixture, u8"f1").Location() == LayoutPoint(0, 10));
    CHECK(BlockOf(fixture, u8"line").LogicalLeftOffsetForLine(0_lu, 1_lu) == 0_lu);
    CHECK(BlockOf(fixture, u8"line").LogicalLeftOffsetForLine(10_lu, 1_lu) == 30_lu);
    CHECK(Dump(fixture).find(u8R"(text run at (30,9) width 19: "two ")") != utf8_string::npos);

    // Floats that fit beside the first line go to its top, even after its content.
    CHECK(BoxOf(fixture, u8"f2").Location() == LayoutPoint(0, 0));
    CHECK(BoxOf(fixture, u8"r2").Location() == LayoutPoint(180, 0));

    // A float that doesn't fit beside its line's content goes below the line; the content after it stays on
    // the line.
    CHECK(BoxOf(fixture, u8"f3").Location() == LayoutPoint(0, 10));
    CHECK(BlockOf(fixture, u8"below").LogicalHeight() == 10_lu);
  }
}
