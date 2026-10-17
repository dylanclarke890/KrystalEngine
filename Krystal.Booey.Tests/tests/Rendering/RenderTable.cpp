#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    // HTML tables without the default spacing and cell padding.
    constexpr auto PlainTables = u8"table { border-spacing: 0 } td { padding: 0 } ";

    /// @brief The box of the element with id @p id, after updating layout.
    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderTableCell &CellOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderTableCell>(BoxOf(fixture, id));
    }

    /// @brief The border box of the element with id @p id, relative to its container.
    KRYS_NODISCARD LayoutRect RectOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return BoxOf(fixture, id).FrameRect();
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTable: auto table layout", "[Rendering][RenderTable]")
  {
    SECTION("CSS tables")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#t { display: table } .r { display: table-row } .c { display: table-cell }",
        u8"<div id=t><div class=r><div class=c id=a>a</div><div class=c id=b>bbb</div></div></div>")};

      // The table shrinks to its columns' widths. The row and its section are anonymous.
      CHECK(layout::tests::DumpWithIds(fixture) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x10
  RenderBlock {HTML} at (0,0) size 800x10
    RenderBody {BODY} at (0,0) size 800x10
      RenderTable {DIV} at (0,0) size 40x10 id="t"
        RenderTableSection (anonymous) at (0,0) size 40x10
          RenderTableRow {DIV} at (0,0) size 40x10 class="r"
            RenderTableCell {DIV} at (0,0) size 10x10 [r=0 c=0 rs=1 cs=1] id="a" class="c"
              RenderText {#text} at (0,0) size 10x10
                text run at (0,0) width 10: "a"
            RenderTableCell {DIV} at (10,0) size 30x10 [r=0 c=1 rs=1 cs=1] id="b" class="c"
              RenderText {#text} at (0,0) size 30x10
                text run at (0,0) width 30: "bbb"
)");
    }

    SECTION("Border spacing")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#t { display: table; border-spacing: 5px 3px } .r { display: table-row } .c { display: table-cell "
        u8"}",
        u8"<div id=t><div class=r id=r><div class=c id=a>a</div><div class=c id=b>bbb</div></div></div>")};

      // The spacing goes around every column and row.
      CHECK(RectOf(fixture, u8"t") == LayoutRect(0, 0, 55, 16));
      CHECK(RectOf(fixture, u8"r") == LayoutRect(0, 3, 55, 10));
      CHECK(RectOf(fixture, u8"a") == LayoutRect(5, 3, 10, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(20, 3, 30, 10));
    }

    SECTION("Percentage columns")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(utf8_string {PlainTables} + u8"table { width: 200px } #a { width: 25% }",
                                    u8"<table><tr><td id=a>a</td><td id=b>b</td></tr></table>")};

      // The percentage column takes 25% of the table, the auto column the rest.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 50, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(50, 0, 150, 10));
    }

    SECTION("Column spans")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        PlainTables,
        u8"<table><tr><td id=a colspan=2>aaaaaa</td></tr><tr><td id=c>c</td><td id=d>dd</td></tr></table>")};

      // The spanning cell's 60px is shared by the columns in proportion to their own widths (10px and 20px).
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 60, 10));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(0, 10, 20, 10));
      CHECK(RectOf(fixture, u8"d") == LayoutRect(20, 10, 40, 10));
      CHECK(CellOf(fixture, u8"a").ColSpan() == 2);
    }

    SECTION("Row spans")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        PlainTables,
        u8"<table><tr><td id=a rowspan=2>a</td><td id=b>b</td></tr><tr><td id=c>c</td></tr></table>")};

      // c is placed after the slot a takes in the second row.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 10, 20));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(10, 10, 10, 10));
      CHECK(CellOf(fixture, u8"c").Col() == 1);
      // tbody's vertical-align: middle centres a's content in its two rows.
      CHECK(CellOf(fixture, u8"a").IntrinsicPaddingBefore() == 5_lu);
    }

    SECTION("rowspan=0 spans the rest of the row group")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        PlainTables, u8"<table><tr><td id=a "
                     u8"rowspan=0>a</td><td>b</td></tr><tr><td>c</td></tr><tr><td>d</td></tr></table>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 10, 30));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTable: fixed table layout", "[Rendering][RenderTable]")
  {
    SECTION("Widths from the first row")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        utf8_string {PlainTables} + u8"table { table-layout: fixed; width: 300px } #a { width: 100px }",
        u8"<table><tr><td id=a>a</td><td id=b>b</td><td id=c>c</td></tr></table>")};

      // The auto columns share what the fixed column leaves.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 100, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(100, 0, 100, 10));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(200, 0, 100, 10));
    }

    SECTION("Widths from columns")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        utf8_string {PlainTables} + u8"table { table-layout: fixed; width: 300px }",
        u8"<table><col style='width: 50px'><col><tr><td id=a>a</td><td id=b>b</td></tr></table>")};

      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 50, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(50, 0, 250, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTable: rows", "[Rendering][RenderTable]")
  {
    SECTION("Baseline alignment")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        utf8_string {PlainTables} + u8"td { vertical-align: baseline } #a { font-size: 20px }",
        u8"<table><tr><td id=a>a</td><td id=b>b</td></tr></table>")};

      // Ahem's ascent is 0.8em, so b is pushed down by 16px - 8px to a's baseline.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 20, 20));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(20, 0, 10, 20));
      CHECK(CellOf(fixture, u8"b").IntrinsicPaddingBefore() == 8_lu);
      CHECK(CellOf(fixture, u8"b").IntrinsicPaddingAfter() == 2_lu);
    }

    SECTION("A specified table height")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"#t { display: table; height: 50px } .r { display: table-row } .c { display: table-cell }",
        u8"<div id=t><div class=r><div class=c id=a>a</div></div><div class=r><div class=c "
        u8"id=b>b</div></div></div>")};

      // The 30px the rows don't use is shared between the auto rows.
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 10, 25));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 25, 10, 25));
    }

    SECTION("Captions")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(utf8_string {PlainTables} + u8"#bottom { caption-side: bottom }",
                                    u8"<table id=t><caption id=top>cap</caption><caption "
                                    u8"id=bottom>c</caption><tr><td id=a>a</td></tr></table>")};

      // The table is as wide as its widest caption, and its rows sit between the captions.
      CHECK(RectOf(fixture, u8"t") == LayoutRect(0, 0, 30, 30));
      CHECK(RectOf(fixture, u8"top") == LayoutRect(0, 0, 30, 10));
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 30, 10));
      CHECK(RectOf(fixture, u8"bottom") == LayoutRect(0, 20, 30, 10));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTable: sections", "[Rendering][RenderTable]")
  {
    SECTION("Header and footer order")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        PlainTables,
        u8"<table><tfoot id=f><tr><td>f</td></tr></tfoot><tbody id=b><tr><td>b</td></tr></tbody><thead "
        u8"id=h><tr><td>h</td></tr></thead></table>")};

      // The header comes first and the footer last, whatever their order in the document.
      CHECK(RectOf(fixture, u8"h") == LayoutRect(0, 0, 10, 10));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(0, 10, 10, 10));
      CHECK(RectOf(fixture, u8"f") == LayoutRect(0, 20, 10, 10));
    }

    SECTION("A collapsed row")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        PlainTables,
        u8"<table id=t><tr><td>a</td></tr><tr id=r style='visibility: collapse'><td>b</td></tr><tr "
        u8"id=c><td>c</td></tr></table>")};

      // The collapsed row takes no space.
      CHECK(RectOf(fixture, u8"r") == LayoutRect(0, 10, 10, 0));
      CHECK(RectOf(fixture, u8"c") == LayoutRect(0, 10, 10, 10));
      CHECK(RectOf(fixture, u8"t") == LayoutRect(0, 0, 10, 20));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTable: collapsed borders", "[Rendering][RenderTable]")
  {
    SECTION("Shared borders")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"table { border-collapse: collapse } td { padding: 0; border: 2px solid black }",
        u8"<table id=t><tr><td id=a>a</td><td id=b>b</td></tr></table>")};

      // Each cell has half of each border it shares; the table has the outer halves.
      CHECK(CellOf(fixture, u8"a").BorderLeft() == 1_lu);
      CHECK(CellOf(fixture, u8"a").BorderRight() == 1_lu);
      CHECK(Downcast<RenderTable>(BoxOf(fixture, u8"t")).BorderLeft() == 1_lu);
      CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 12, 12));
      CHECK(RectOf(fixture, u8"b") == LayoutRect(12, 0, 12, 12));
      CHECK(RectOf(fixture, u8"t") == LayoutRect(0, 0, 26, 14));
    }

    SECTION("A hidden border wins")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"table { border-collapse: collapse } td { padding: 0; border: 2px solid black } #b { "
        u8"border-left-style: hidden }",
        u8"<table id=t><tr><td id=a>a</td><td id=b>b</td></tr></table>")};

      // The border between the cells is suppressed on both sides.
      CHECK(CellOf(fixture, u8"a").BorderRight() == 0_lu);
      CHECK(CellOf(fixture, u8"b").BorderLeft() == 0_lu);
      CHECK(RectOf(fixture, u8"b") == LayoutRect(11, 0, 11, 12));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTable: relayout", "[Rendering][RenderTable]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      PlainTables,
      u8"<table><tr><td id=a>a</td><td id=b>b</td></tr><tr><td id=c>cccc</td><td id=d>d</td></tr></table>")};
    CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 40, 10));

    // a now spans both columns, so b moves to a third column.
    REQUIRE_FALSE(fixture.ById(u8"a").SetAttribute(u8"colspan", u8"2").HasException());
    CHECK(RectOf(fixture, u8"a") == LayoutRect(0, 0, 50, 10));
    CHECK(RectOf(fixture, u8"b") == LayoutRect(50, 0, 10, 10));
    CHECK(CellOf(fixture, u8"b").Col() == 2);
  }
}
