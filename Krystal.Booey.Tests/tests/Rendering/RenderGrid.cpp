#include "Krystal.Booey/Rendering/RenderGrid.hpp"
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

    /// @brief The border box of the element with id @p id, relative to its container.
    KRYS_NODISCARD LayoutRect RectOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return BoxOf(fixture, id).FrameRect();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderGrid: placement", "[Rendering][RenderGrid]")
  {
    SECTION("Explicit and auto placement")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-columns: 50px 100px; grid-template-rows: 20px 30px } #a { "
        u8"grid-column: 2; grid-row: 2 }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      // The explicitly placed item is placed first; the auto-placed one takes the first free cell. Both
      // stretch to fill their area.
      CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x50
  RenderBlock {HTML} at (0,0) size 800x50
    RenderBody {BODY} at (0,0) size 800x50
      RenderGrid {DIV} at (0,0) size 800x50 id="g"
        RenderBlock {DIV} at (50,20) size 100x30 id="a"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "a"
        RenderBlock {DIV} at (0,0) size 50x20 id="b"
          RenderText {#text} at (0,0) size 10x10
            text run at (0,0) width 10: "b"
)");
    }

    SECTION("Spans")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-columns: 50px 50px 50px } #a { grid-column: span 2 }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 100, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(100, 0, 50, 10));
      // The first row is full, so the next item starts a new implicit row.
      CHECK(RectOf(fixture, u8"c") == LayoutRect(0, 10, 50, 10));
    }

    SECTION("Dense packing")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-columns: 50px 50px 50px; grid-auto-flow: dense } #a { "
        u8"grid-column: span 2 } #b { grid-column: span 2 }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      // b doesn't fit beside a, so it starts a new row, and c backfills the hole after a.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 100, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 10, 100, 10));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(100, 0, 50, 10));
    }

    SECTION("Named areas")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-areas: \"head head\" \"side main\"; grid-template-columns: "
        u8"100px "
        u8"200px; grid-template-rows: 20px 40px } #h { grid-area: head } #m { grid-area: main }",
        u8"<div id=g><div id=m>m</div><div id=h>h</div></div>")};

      CHECK(RectOf(fixture, u8"h") == LayoutRect(0, 0, 300, 20));
      CHECK(RectOf(fixture, u8"m") == LayoutRect(100, 20, 200, 40));
    }

    SECTION("Named lines")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; grid-template-columns: [start] 100px [mid] 100px "
                                    u8"[end] } #a { grid-column: mid / "
                                    u8"end }",
                                    u8"<div id=g><div id=a>a</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(100, 0, 100, 10));
    }

    SECTION("Negative lines")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-columns: 50px 60px 70px } #a { grid-column: -2 / -1 }",
        u8"<div id=g><div id=a>a</div></div>")};

      // -1 is the last line of the explicit grid, so the item is in the last column.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(110, 0, 70, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderGrid: track sizing", "[Rendering][RenderGrid]")
  {
    SECTION("Flexible tracks")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; width: 300px; grid-template-columns: 1fr 2fr }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 100, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(100, 0, 200, 10));
    }

    SECTION("Auto tracks")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; width: 200px; grid-template-columns: auto auto }",
                                    u8"<div id=g><div id=a>a</div><div id=b>bbb</div></div>")};

      // The tracks are sized to their contents (10px and 30px), then the 160px left is shared equally
      // between the auto tracks.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 90, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(90, 0, 110, 10));
    }

    SECTION("minmax()")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; width: 200px; grid-template-columns: minmax(20px, 50px) 1fr }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      // The first track grows to its maximum, the flexible track takes the rest.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 50, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(50, 0, 150, 10));
    }

    SECTION("Rows sized by their content")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; width: 40px; grid-template-columns: 20px 20px }",
        u8"<div id=g><div id=a>a</div><div id=b>bbbb</div><div id=c>c</div></div>")};

      // "bbbb" has no break opportunity, so it overflows its 20px column on a single line and the rows stay
      // 10px tall.
      CHECK(RectOf(fixture, u8"b") == LayoutRect(20, 0, 20, 10));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(0, 10, 20, 10));
    }

    SECTION("Gaps")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-columns: 50px 50px; grid-template-rows: 20px 20px; column-gap: "
        u8"10px; row-gap: 5px }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div><div id=c>c</div><div id=d>d</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 50, 20));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(60, 0, 50, 20));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(0, 25, 50, 20));
      CHECK(RectOf(fixture, u8"d") == LayoutRect(60, 25, 50, 20));
      CHECK(BoxOf(fixture, u8"g").Height() == 45_lu);
    }

    SECTION("repeat(auto-fill)")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; width: 350px; grid-template-columns: repeat(auto-fill, 100px) }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div><div id=c>c</div><div id=d>d</div></div>")};

      // Three 100px tracks fit, so the fourth item wraps.
      CHECK(RectOf(fixture, u8"c") == LayoutRect(200, 0, 100, 10));
      CHECK(RectOf(fixture, u8"d") == LayoutRect(0, 10, 100, 10));
    }

    SECTION("repeat(auto-fit) collapses the empty tracks")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; width: 350px; grid-template-columns: "
                                    u8"repeat(auto-fit, 100px); justify-content: "
                                    u8"center }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      // Only the two used tracks keep their size, and are centred in the 350px.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(75, 0, 100, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(175, 0, 100, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderGrid: alignment", "[Rendering][RenderGrid]")
  {
    SECTION("Self-alignment")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; grid-template-columns: 100px; grid-template-rows: "
                                    u8"50px; justify-items: center; "
                                    u8"align-items: end }",
                                    u8"<div id=g><div id=a>aa</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(40, 40, 20, 10));
    }

    SECTION("Content distribution")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; width: 300px; grid-template-columns: 100px 100px; "
                                    u8"justify-content: space-between }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 100, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(200, 0, 100, 10));
    }

    SECTION("Baselines")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; grid-template-columns: 100px 100px; align-items: "
                                    u8"baseline } #a { font-size: 20px }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      // Ahem's ascent is 0.8em, so b moves down by 16px - 8px to share a's baseline.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 100, 20));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(100, 8, 100, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderGrid: subgrid and masonry", "[Rendering][RenderGrid]")
  {
    SECTION("Subgrid")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid; grid-template-columns: 50px 100px 150px } #s { grid-column: 1 / 4; display: "
        u8"grid; "
        u8"grid-template-columns: subgrid }",
        u8"<div id=g><div id=s><div id=x>x</div><div id=y>y</div><div id=z>z</div></div></div>")};

      // The subgrid's items use the parent grid's columns.
      CHECK(RectOf(fixture, u8"s") == LayoutRect(0, 0, 300, 10));
      CHECK(RectOf(fixture, u8"x") == LayoutRect(0, 0, 50, 10));
      CHECK(RectOf(fixture, u8"y") == LayoutRect(50, 0, 100, 10));
      CHECK(RectOf(fixture, u8"z") == LayoutRect(150, 0, 150, 10));
    }

    SECTION("Masonry")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#g { display: grid-lanes; grid-template-columns: 50px 50px } #a { height: 60px }",
        u8"<div id=g><div id=a>a</div><div id=b>b</div><div id=c>c</div></div>")};

      // Each item goes to the shortest column.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 50, 60));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(50, 0, 50, 10));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(50, 10, 50, 10));
      CHECK(BoxOf(fixture, u8"g").Height() == 60_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderGrid: grid formatting context", "[Rendering][RenderGrid]")
  {
    constexpr auto gridStyle =
      u8"#g { display: grid; width: 200px; height: 100px; grid-template-columns: 100px 100px; "
      u8"grid-template-rows: 50px 50px; column-gap: 10px } #g > div { width: 80px; height: 40px } #a { "
      u8"grid-column: 2 / 3; grid-row: 2 / 3 } #b { grid-column: 1 / 2; grid-row: 1 / 2 }";

    SECTION("Supported content")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(gridStyle, u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      // The second column starts after the first column and the gap.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(110, 50, 80, 40));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 0, 80, 40));
      // Only the grid's own layout gives the items their grid area.
      CHECK_FALSE(BoxOf(fixture, u8"a").GridAreaContentLogicalWidth());
    }

    SECTION("Unsupported content")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(utf8_string {gridStyle} + u8" #b { margin-left: 5px }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(110, 50, 80, 40));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(5, 0, 80, 40));
      CHECK(BoxOf(fixture, u8"a").GridAreaContentLogicalWidth());
    }

    SECTION("Items spanning several tracks")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(utf8_string {gridStyle} + u8" #b { grid-column: 1 / 3 }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      // The grid formatting context only places items in a single track, so the grid lays them out.
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 0, 80, 40));
      CHECK(BoxOf(fixture, u8"b").GridAreaContentLogicalWidth());
    }

    SECTION("Content becoming unsupported")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(gridStyle, u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 0, 80, 40));

      // The grid falls back to its own layout, which places the items again.
      REQUIRE_FALSE(fixture.ById(u8"b").SetAttribute(u8"style", u8"margin-top: 5px").HasException());
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 5, 80, 40));
      CHECK(RectOf(fixture, u8"a") == LayoutRect(110, 50, 80, 40));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderGrid: relayout", "[Rendering][RenderGrid]")
  {
    SECTION("A grid item changing position")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; grid-template-columns: 50px 50px }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};
      CHECK(RectOf(fixture, u8"b") == LayoutRect(50, 0, 50, 10));

      REQUIRE_FALSE(fixture.ById(u8"a").SetAttribute(u8"style", u8"grid-column: 2").HasException());
      // a moves to the second column, so b moves to the next row.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(50, 0, 50, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 10, 50, 10));
    }

    SECTION("A grid item becoming out-of-flow")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"#g { display: grid; position: relative; grid-template-columns: 50px "
                                    u8"50px; grid-template-rows: 20px "
                                    u8"20px }",
                                    u8"<div id=g><div id=a>a</div><div id=b>b</div></div>")};

      REQUIRE_FALSE(fixture.ById(u8"a")
                      .SetAttribute(u8"style", u8"position: absolute; grid-column: 2; grid-row: 2")
                      .HasException());
      // b takes the first cell, and a's containing block is its grid area.
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 0, 50, 20));
      CHECK(RectOf(fixture, u8"a") == LayoutRect(50, 20, 10, 10));
    }
  }
}
