#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD utf8_string Dump(ResolverFixture &fixture) noexcept
    {
      return ExternalRepresentation(
        fixture.Frame.get(),
        {RenderAsTextFlag::ShowAllLayers, RenderAsTextFlag::ShowIDAndClass, RenderAsTextFlag::ShowOverflow});
    }

    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBlockFlow: margin collapsing and sizing",
                   "[Rendering][RenderBlockFlow]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 8px }
      #a { margin: 10px 20px; padding: 5px; border: 2px solid green; height: 50px }
      #b { margin-top: 30px; margin-bottom: 15px; width: 300px; height: 20px }
      #c { margin-top: 25px }
      #c > div { margin-top: 40px; height: 10px }
      #empty { margin: 12px 0 }
      #d { height: 100px; padding: 10px; box-sizing: border-box; margin: 0 auto; width: 50% }
      #rel { position: relative; top: 5px; height: 10px }
    </style><body><div id=a></div><div id=b></div><div id=c><div></div></div><div id=empty></div><div id=d></div><div id=rel></div></body>)html"};

    // #a's top margin collapses through the body, but not through the root, which is a formatting context
    // root. #c's child's margin collapses through #c with #b's bottom margin; #empty collapses through
    // itself.
    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x304
  RenderBlock {HTML} at (0,0) size 800x304
    RenderBody {BODY} at (8,10) size 784x286 (layout overflow 0,0 784x291)
      RenderBlock {DIV} at (20,0) size 744x64 [border: (2px solid #008000)] id="a"
      RenderBlock {DIV} at (0,94) size 300x20 id="b"
      RenderBlock {DIV} at (0,154) size 784x10 id="c"
        RenderBlock {DIV} at (0,0) size 784x10
      RenderBlock {DIV} at (0,176) size 784x0 id="empty"
      RenderBlock {DIV} at (196,176) size 392x100 id="d"
layer at (8,291) size 784x10
  RenderBlock (relative positioned) {DIV} at (0,276) size 784x10 id="rel"
)");

    SECTION("Relayout after a style change")
    {
      REQUIRE_FALSE(fixture.ById(u8"b").SetAttribute(u8"style", u8"height: 40px").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"b").Height() == 40_lu);
      CHECK(BoxOf(fixture, u8"c").Y() == 174_lu);
      CHECK(BoxOf(fixture, u8"d").Y() == 196_lu);
      CHECK(BoxOf(fixture, u8"d").X() == 196_lu);

      REQUIRE_FALSE(fixture.ById(u8"a").SetAttribute(u8"style", u8"padding-left: 50px").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"a").Width() == 744_lu);
      CHECK(BoxOf(fixture, u8"a").ContentBoxLogicalWidth() == 685_lu);
    }

    SECTION("The view's size")
    {
      fixture.Frame->View()->Resize({400, 300});
      (void)fixture.Document->UpdateLayout();
      CHECK(fixture.Document->GetRenderView()->Size() == LayoutSize(400, 300));
      CHECK(BoxOf(fixture, u8"d").Width() == 192_lu);
      CHECK(BoxOf(fixture, u8"d").X() == 96_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderBlockFlow: alignment, trimming, writing modes and scrolling",
                   "[Rendering][RenderBlockFlow]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #ac { height: 100px; align-content: center; border: 1px solid }
      #ac > div { height: 20px }
      #trim { margin-trim: block; border: 1px solid }
      #trim > div { margin: 10px 0; height: 10px }
      #v { writing-mode: vertical-rl; height: 50px; border: 1px solid }
      #v > div { width: 10px; margin-right: 5px }
      #s { overflow: scroll; height: 40px }
      #s > div { height: 100px }
      #bfc { display: flow-root; margin-top: 10px }
      #bfc > div { margin-top: 20px; height: 5px }
      p { margin: 1em 0 }
    </style><body><div id=ac><div></div></div><div id=trim><div></div><div></div></div><div id=v><div></div></div><div id=s><div></div></div><div id=bfc><div></div></div><p>text</p></body>)html"};

    // The paragraph's text has no lines until inline layout, so it's self-collapsing.
    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x277
  RenderBlock {HTML} at (0,0) size 800x277
    RenderBody {BODY} at (0,0) size 800x261
      RenderBlock {DIV} at (0,0) size 800x102 [border: (1px solid #000000)] id="ac"
        RenderBlock {DIV} at (1,41) size 798x20
      RenderBlock {DIV} at (0,102) size 800x32 [border: (1px solid #000000)] id="trim"
        RenderBlock {DIV} at (1,1) size 798x10
        RenderBlock {DIV} at (1,21) size 798x10
      RenderBlock {DIV} at (0,134) size 17x52 [border: (1px solid #000000)] id="v"
        RenderBlock {DIV} at (6,1) size 10x50
      RenderBlock {DIV} at (0,236) size 800x25 id="bfc"
        RenderBlock {DIV} at (0,20) size 800x5
      RenderBlock {P} at (0,277) size 800x0
        RenderText {#text} at (0,0) size 0x0
layer at (0,186) size 800x40 clip at (0,186) size 785x25 scrollHeight 100
  RenderBlock {DIV} at (0,186) size 800x40 id="s" (layout overflow 0,0 785x100)
    RenderBlock {DIV} at (0,0) size 785x100
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBlockFlow: quirky margins",
                   "[Rendering][RenderBlockFlow]")
  {
    // In quirks mode, the body fills the viewport, and the paragraphs' quirky margins collapse away at the
    // body's edges.
    ResolverFixture fixture {
      u8R"html(<body><p id=q>x</p><div style="height:10px"></div><p>y</p></body>)html"};

    CHECK(Dump(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x600
  RenderBlock {HTML} at (0,0) size 800x600
    RenderBody {BODY} at (8,8) size 784x576
      RenderBlock {P} at (0,0) size 784x0 id="q"
        RenderText {#text} at (0,0) size 0x0
      RenderBlock {DIV} at (0,0) size 784x10
      RenderBlock {P} at (0,26) size 784x0
        RenderText {#text} at (0,0) size 0x0
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBlockFlow: anonymous blocks follow their parent's style",
                   "[Rendering][RenderBlockFlow]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><div id=parent>text<div>block</div></div>)html"};
    auto &parent = Downcast<RenderBlock>(*fixture.ById(u8"parent").Renderer());
    auto *anonymous = Downcast<RenderBlock>(parent.FirstChild());
    REQUIRE(anonymous);
    REQUIRE(anonymous->IsAnonymousBlock());

    REQUIRE_FALSE(fixture.ById(u8"parent").SetAttribute(u8"style", u8"color: red").HasException());
    fixture.Document->UpdateStyleIfNeeded();
    REQUIRE(parent.FirstChild() == anonymous);
    CHECK(anonymous->Style().VisitedDependentColor(css::PropertyId::Color)
          == Color(SRGBA<uint8> {255, 0, 0}));
  }
}
