#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The box of the element with id @p id, after updating layout.
    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderFlexibleBox &FlexBoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderFlexibleBox>(BoxOf(fixture, id));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFlexibleBox: flexing along the main axis",
                   "[Rendering][RenderFlexibleBox]")
  {
    SECTION("Growing")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; width: 300px } #a { flex: 1 } #b { flex: 2 } #c { flex: none }",
        u8"<div id=f><div id=a>a</div><div id=b>bb</div><div id=c>ccc</div></div>")};

      // The free space left by the inflexible item is shared 1:2.
      CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x10
  RenderBlock {HTML} at (0,0) size 800x10
    RenderBody {BODY} at (0,0) size 800x10
      RenderFlexibleBox {DIV} at (0,0) size 300x10 id="f"
        RenderBlock {DIV} at (0,0) size 90x10 id="a"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "a"
        RenderBlock {DIV} at (90,0) size 180x10 id="b"
          RenderText {#text} at (0,0) size 20x10
            text run at (0,0) width 20: "bb"
        RenderBlock {DIV} at (270,0) size 30x10 id="c"
          RenderText {#text} at (0,0) size 30x10
            text run at (0,0) width 30: "ccc"
)");
      CHECK_FALSE(FlexBoxOf(fixture, u8"f").HasModernLayout());
    }

    SECTION("Shrinking, in order")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; width: 100px; justify-content: space-between } #f > div { width: 60px } #a { "
        u8"order: 2 } #b { flex-shrink: 0 }",
        u8"<div id=f><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      // a is placed last; the overflow is taken from the two shrinkable items, leaving no space to
      // distribute.
      CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x10
  RenderBlock {HTML} at (0,0) size 800x10
    RenderBody {BODY} at (0,0) size 800x10
      RenderFlexibleBox {DIV} at (0,0) size 100x10 id="f"
        RenderBlock {DIV} at (80,0) size 20x10 id="a"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "a"
        RenderBlock {DIV} at (0,0) size 60x10 id="b"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "b"
        RenderBlock {DIV} at (60,0) size 20x10 id="c"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "c"
)");
    }

    SECTION("Trimmed margins")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; margin-trim: inline; width: 100px } #f > div { margin: 0 10px }",
        u8"<div id=f><div id=a>a</div><div id=b>b</div></div>")};

      // The first item's start margin and the last item's end margin are trimmed.
      CHECK(BoxOf(fixture, u8"a").Location() == LayoutPoint(0, 0));
      CHECK(BoxOf(fixture, u8"b").Location() == LayoutPoint(30, 0));
      CHECK(BoxOf(fixture, u8"b").MarginRight() == 0_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFlexibleBox: columns, lines and alignment",
                   "[Rendering][RenderFlexibleBox]")
  {
    SECTION("A column aligned to its end and centered across")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; flex-direction: column; height: 100px; width: 50px; align-items: center; "
        u8"justify-content: flex-end }",
        u8"<div id=f><div id=a>a</div><div id=b>bbb</div></div>")};

      CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x100
  RenderBlock {HTML} at (0,0) size 800x100
    RenderBody {BODY} at (0,0) size 800x100
      RenderFlexibleBox {DIV} at (0,0) size 50x100 id="f"
        RenderBlock {DIV} at (20,80) size 10x10 id="a"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "a"
        RenderBlock {DIV} at (10,90) size 30x10 id="b"
          RenderText {#text} at (0,0) size 30x10
            text run at (0,0) width 30: "bbb"
)");
      // The baseline is the first item's.
      CHECK(FlexBoxOf(fixture, u8"f").FirstLineBaseline() == 88_lu);
    }

    SECTION("Wrapping lines with gaps")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; flex-wrap: wrap; width: 100px; height: 100px; gap: 10px 5px; align-content: "
        u8"space-between } #f > div { width: 40px }",
        u8"<div id=f><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      // Two items and a 5px column gap fit on the first line; the lines are pushed apart.
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 0, 40, 10));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(45, 0, 40, 10));
      CHECK(BoxOf(fixture, u8"c").FrameRect() == LayoutRect(0, 90, 40, 10));
    }

    SECTION("Baseline alignment")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; align-items: baseline } #a { font-size: 20px } #b { margin-top: 5px }",
        u8"<div id=f><div id=a>a</div><div id=b>b</div><div id=c></div></div>")};

      // b's and the empty c's (synthesized from its bottom edge) baselines line up with a's at 16px.
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 0, 20, 20));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(20, 8, 10, 10));
      CHECK(BoxOf(fixture, u8"c").FrameRect() == LayoutRect(30, 16, 0, 0));
      CHECK(FlexBoxOf(fixture, u8"f").Height() == 20_lu);
      CHECK(FlexBoxOf(fixture, u8"f").FirstLineBaseline() == 16_lu);
    }

    SECTION("Stretching and auto margins")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; height: 50px } #a { margin: auto 0 } #b { margin-left: auto }",
        u8"<div id=f><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      // Auto margins center a across the line instead of stretching it, and push b and c to the end.
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 20, 10, 10));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(780, 0, 10, 50));
      CHECK(BoxOf(fixture, u8"c").FrameRect() == LayoutRect(790, 0, 10, 50));
    }

    SECTION("An inline flex container, reversed both ways")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#f { display: inline-flex; flex-direction: row-reverse; flex-wrap: "
                                    u8"wrap-reverse; width: 30px } #a { "
                                    u8"font-size: 20px }",
                                    u8"x<div id=f><div id=a>a</div><div id=b>b</div></div>y")};

      // The items run right to left, and the container sits on the line by the baseline of its visually first
      // item.
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(10, 0, 20, 20));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(0, 0, 10, 20));
      CHECK(FlexBoxOf(fixture, u8"f").FrameRect() == LayoutRect(10, 0, 30, 20));
      CHECK(FlexBoxOf(fixture, u8"f").FirstLineBaseline() == 8_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFlexibleBox: out-of-flow children",
                   "[Rendering][RenderFlexibleBox]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"#f { display: flex; position: relative; width: 100px; height: 50px; justify-content: center; "
      u8"align-items: flex-end } #p { position: absolute; width: 20px; height: 10px }",
      u8"<div id=f><div id=p></div><div id=a>a</div></div>")};

    // The out-of-flow child's static position is aligned as if it were the only item.
    CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(45, 40, 10, 10));
    CHECK(BoxOf(fixture, u8"p").FrameRect() == LayoutRect(40, 40, 20, 10));

    SECTION("An item going out of flow")
    {
      REQUIRE_FALSE(fixture.ById(u8"a").SetAttribute(u8"style", u8"position: absolute").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(45, 40, 10, 10));
      CHECK(FlexBoxOf(fixture, u8"f").FirstLineBaseline() == null);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFlexibleBox: relayout", "[Rendering][RenderFlexibleBox]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"#f { display: flex; height: 50px }", u8"<div id=f><div id=a>a</div><div id=b>bb</div></div>")};

    CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 0, 10, 50));

    SECTION("No longer stretching")
    {
      REQUIRE_FALSE(fixture.ById(u8"f").SetAttribute(u8"style", u8"align-items: center").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 20, 10, 10));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(10, 20, 20, 10));
    }

    SECTION("Removing an item")
    {
      REQUIRE_FALSE(fixture.ById(u8"a").Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(0, 0, 20, 50));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderFlexibleBox: the flex formatting context",
                   "[Rendering][RenderFlexibleBox]")
  {
    SECTION("Supported content")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#f { display: flex; width: 200px } #f > div { flex: 1 1 40px; height: "
                                    u8"20px } #f > #b { flex-grow: 3 "
                                    u8"} #f > #c { margin-left: auto }",
                                    u8"<div id=f><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      // The 80px of free space is shared 1:3:1.
      CHECK(FlexBoxOf(fixture, u8"f").HasModernLayout());
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 0, 56, 20));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(56, 0, 88, 20));
      CHECK(BoxOf(fixture, u8"c").FrameRect() == LayoutRect(144, 0, 56, 20));
      CHECK(FlexBoxOf(fixture, u8"f").Height() == 20_lu);
    }

    SECTION("Wrapping, reversed")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#f { display: flex; flex-flow: row-reverse wrap; width: 100px; column-gap: 10px; justify-content: "
        u8"center } #f > div { flex: 0 0 40px; height: 10px }",
        u8"<div id=f><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      CHECK(FlexBoxOf(fixture, u8"f").HasModernLayout());
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(55, 0, 40, 10));
      CHECK(BoxOf(fixture, u8"b").FrameRect() == LayoutRect(5, 0, 40, 10));
      CHECK(BoxOf(fixture, u8"c").FrameRect() == LayoutRect(30, 10, 40, 10));
    }

    SECTION("Unsupported content")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#f { display: flex; width: 200px } #f > div { flex: 1 1 40px }",
                                    u8"<div id=f><div id=a>a</div></div>")};

      // The items need a fixed height.
      CHECK_FALSE(FlexBoxOf(fixture, u8"f").HasModernLayout());
      CHECK(BoxOf(fixture, u8"a").FrameRect() == LayoutRect(0, 0, 200, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "OrderIterator: flex order", "[Rendering][OrderIterator]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"#f { display: flex } #a { order: 2 } #d { position: absolute }",
      u8"<div id=f><div id=a></div><div id=b></div><div id=c></div><div id=d></div></div>")};

    auto ids = [](OrderIterator iterator)
    {
      auto result = utf8_string {};
      for (auto *child = iterator.First(); child; child = iterator.Next())
      {
        if (!iterator.ShouldSkipChild(*child))
        {
          result += child->GetElement()->Id();
        }
      }
      return result;
    };

    // Lower order values come first, and document order breaks ties; out-of-flow children are skipped. The
    // reverse iterator visits the order groups in reverse too.
    auto &iterator = FlexBoxOf(fixture, u8"f").GetOrderIterator();
    CHECK(ids(iterator) == u8"bca");
    CHECK(ids(iterator.Reverse()) == u8"acb");
  }
}
