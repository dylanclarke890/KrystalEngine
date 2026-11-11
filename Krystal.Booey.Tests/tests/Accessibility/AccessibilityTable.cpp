#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityNodeObject.hpp"
#include "Krystal.Booey/Accessibility/AccessibilityTableColumn.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::ax::tests
{
  namespace
  {
    KRYS_NODISCARD bool Contains(const AccessibilityObject::AccessibilityChildrenVector &objects,
                                 const AccessibilityObject &object) noexcept
    {
      return std::ranges::any_of(objects, [&](const auto &candidate) { return candidate.get() == &object; });
    }
  }

  TEST_CASE("AccessibilityTable: a data table", "[Accessibility][AccessibilityTable]")
  {
    AccessibilityFixture page(u8"<table id=table><caption>Fruit</caption>"
                              u8"<thead><tr id=r0><th id=h0>Name</th><th id=h1>Colour</th></tr></thead>"
                              u8"<tbody><tr id=r1><td id=c10>Apple</td><td id=c11>Red</td></tr>"
                              u8"<tr id=r2><th id=rh>Banana</th><td id=c21>Yellow</td></tr></tbody></table>");

    auto &table = page.Object(u8"table");
    CHECK(table.IsExposableTable());
    CHECK(table.Role() == AccessibilityRole::Table);
    CHECK(table.RowCount() == 3);
    CHECK(table.ColumnCount() == 2);

    // The table's children are its rows (after its caption); the columns aren't in the tree.
    auto rows = table.Rows();
    REQUIRE(rows.size() == 3);
    CHECK(rows[0].get() == &page.Object(u8"r0"));
    CHECK(rows[2].get() == &page.Object(u8"r2"));
    for (const auto &row : rows)
    {
      CHECK(row->Role() == AccessibilityRole::Row);
      CHECK(row->ParentObjectUnignored() == &table);
      CHECK(Contains(table.Children(), *row));
    }
    REQUIRE(table.Columns().size() == 2);
    CHECK(table.Columns()[0]->IsIgnored());
    CHECK_FALSE(Contains(table.Children(), *table.Columns()[0]));

    auto &c11 = page.Object(u8"c11");
    CHECK(c11.Role() == AccessibilityRole::Cell);
    CHECK(c11.IsExposedTableCell());
    CHECK(c11.ParentTable() == &table);
    CHECK(c11.RowIndexRange() == Pair<uint32, uint32> {1, 1});
    CHECK(c11.ColumnIndexRange() == Pair<uint32, uint32> {1, 1});
    CHECK(table.CellForColumnAndRow(1, 2) == &page.Object(u8"c21"));
    CHECK(table.CellForColumnAndRow(2, 0) == nullptr);

    SECTION("headers")
    {
      CHECK(page.Object(u8"h0").IsColumnHeader());
      CHECK(page.Object(u8"h0").ComputedRoleString() == u8"columnheader");
      CHECK(page.Object(u8"rh").IsRowHeader());
      CHECK_FALSE(page.Object(u8"rh").IsColumnHeader());
      CHECK(page.Object(u8"rh").ComputedRoleString() == u8"rowheader");
      CHECK(c11.ComputedRoleString() == u8"cell");

      auto columnHeaders = c11.ColumnHeaders();
      REQUIRE(columnHeaders.size() == 1);
      CHECK(columnHeaders[0].get() == &page.Object(u8"h1"));
      auto rowHeaders = page.Object(u8"c21").RowHeaders();
      REQUIRE(rowHeaders.size() == 1);
      CHECK(rowHeaders[0].get() == &page.Object(u8"rh"));
      CHECK(page.Object(u8"r2").RowHeader() == &page.Object(u8"rh"));
      CHECK(page.Object(u8"r1").RowHeader() == nullptr);

      auto tableHeaders = table.ColumnHeaders();
      REQUIRE(tableHeaders.size() == 2);
      CHECK(tableHeaders[1].get() == &page.Object(u8"h1"));
      auto *container = table.TableHeaderContainer();
      REQUIRE(container);
      CHECK(container->IsIgnored());
      CHECK(container->Children().size() == 2);
    }

    SECTION("columns")
    {
      auto &column = *table.Columns()[0];
      CHECK(Is<AccessibilityTableColumn>(column));
      CHECK(column.ColumnIndex() == 0);
      CHECK(column.ColumnHeader() == &page.Object(u8"h0"));
      const auto &cells = column.Children();
      REQUIRE(cells.size() == 3);
      CHECK(cells[1].get() == &page.Object(u8"c10"));
      // The column doesn't take the cells from their rows.
      CHECK(cells[1]->IndexInParent() == 0);
      CHECK(page.Object(u8"c11").IndexInParent() == 1);
    }
  }

  TEST_CASE("AccessibilityTable: a layout table isn't a table", "[Accessibility][AccessibilityTable]")
  {
    AccessibilityFixture page(u8"<table id=table><tr id=row><td id=cell>One</td><td>Two</td></tr></table>");
    auto &table = page.Object(u8"table");
    CHECK(table.IsTable());
    CHECK_FALSE(table.IsExposableTable());
    CHECK(table.Role() != AccessibilityRole::Table);
    CHECK(table.RowCount() == 0);
    CHECK_FALSE(page.Object(u8"row").IsExposedTableRow());
    CHECK(page.Object(u8"cell").Role() == AccessibilityRole::TextGroup);
    CHECK_FALSE(page.Object(u8"cell").IsExposedTableCell());
  }

  TEST_CASE("AccessibilityTable: the data table heuristic", "[Accessibility][AccessibilityTable]")
  {
    AccessibilityFixture page(
      u8"<table id=summary summary=s><tr><td>A</td></tr></table>"
      u8"<table id=headers><tr><th>A</th><th>B</th></tr><tr><td>1</td><td>2</td></tr></table>"
      u8"<table id=scope><tr><td scope=row>A</td><td>B</td></tr></table>"
      u8"<table id=borders><tr><td style='border:1px solid'>A</td>"
      u8"<td style='border:1px solid'>B</td></tr></table>"
      u8"<table id=presentational role=presentation><caption>C</caption>"
      u8"<tr><td>A</td></tr></table>"
      u8"<div contenteditable><table id=editable><tr><td>A</td></tr></table></div>");
    CHECK(page.Object(u8"summary").IsExposableTable());
    CHECK(page.Object(u8"headers").IsExposableTable());
    CHECK(page.Object(u8"scope").IsExposableTable());
    CHECK(page.Object(u8"borders").IsExposableTable());
    CHECK_FALSE(page.Object(u8"presentational").IsTable());
    CHECK(page.Object(u8"editable").IsExposableTable());
  }

  TEST_CASE("AccessibilityTable: row and column spans", "[Accessibility][AccessibilityTable]")
  {
    AccessibilityFixture page(u8"<table id=table role=grid>"
                              u8"<tr><td id=a rowspan=2>A</td><td id=b colspan=2>B</td></tr>"
                              u8"<tr><td id=c>C</td><td id=d>D</td></tr></table>");
    auto &table = page.Object(u8"table");
    CHECK(table.Role() == AccessibilityRole::Grid);
    CHECK(table.RowCount() == 2);
    CHECK(table.ColumnCount() == 3);

    auto &a = page.Object(u8"a");
    CHECK(a.Role() == AccessibilityRole::GridCell);
    CHECK(a.RowIndexRange() == Pair<uint32, uint32> {0, 2});
    CHECK(page.Object(u8"b").ColumnIndexRange() == Pair<uint32, uint32> {1, 2});
    CHECK(page.Object(u8"c").ColumnIndexRange() == Pair<uint32, uint32> {1, 1});
    CHECK(page.Object(u8"d").RowIndexRange() == Pair<uint32, uint32> {1, 1});
    CHECK(table.CellForColumnAndRow(0, 1) == &a);
    CHECK(table.CellForColumnAndRow(2, 0) == &page.Object(u8"b"));
    CHECK(table.Cells().size() == 4);
  }

  TEST_CASE("AccessibilityTable: an ARIA grid", "[Accessibility][AccessibilityTable]")
  {
    AccessibilityFixture page(u8"<div id=grid role=grid aria-rowcount=10 aria-colcount=-1>"
                              u8"<div id=row role=row aria-rowindex=4 aria-colindex=3>"
                              u8"<div id=x role=gridcell>1</div><div id=y role=columnheader>2</div></div>"
                              u8"<div role=rowgroup><div role=row><div id=z role=gridcell>3</div></div></div>"
                              u8"</div>");
    auto &grid = page.Object(u8"grid");
    CHECK(grid.Role() == AccessibilityRole::Grid);
    CHECK(grid.RowCount() == 2);
    CHECK(grid.ColumnCount() == 2);
    CHECK(grid.AxRowCount() == 10);
    CHECK(grid.AxColumnCount() == -1);

    auto &row = page.Object(u8"row");
    CHECK(row.Role() == AccessibilityRole::Row);
    CHECK(row.AxRowIndex() == 4u);
    // The cells' indexes come from their row's.
    CHECK(page.Object(u8"x").AxRowIndex() == 4u);
    CHECK(page.Object(u8"x").AxColumnIndex() == 3u);
    CHECK(page.Object(u8"y").AxColumnIndex() == 4u);
    CHECK(page.Object(u8"y").IsColumnHeader());
    CHECK(page.Object(u8"z").RowIndexRange().first == 1);
    CHECK_FALSE(page.Object(u8"z").AxRowIndex());
  }

  TEST_CASE("AccessibilityTable: rows added later", "[Accessibility][AccessibilityTable]")
  {
    AccessibilityFixture page(
      u8"<table id=table><caption>C</caption><tbody id=body><tr><td>A</td></tr></tbody></table>");
    auto &table = page.Object(u8"table");
    REQUIRE(table.RowCount() == 1);
    auto oldColumn = ShareRef(*table.Columns()[0]);

    auto row = page.Document->CreateElement(u8"tr").ReleaseValue();
    auto cell = page.Document->CreateElement(u8"td").ReleaseValue();
    REQUIRE_FALSE(row->AppendChild(*cell).HasException());
    REQUIRE_FALSE(page.ById(u8"body").AppendChild(*row).HasException());
    (void)page.Document->UpdateLayout();
    page.Cache().ChildrenChanged(&table);
    page.Cache().PerformDeferredCacheUpdate();

    CHECK(table.RowCount() == 2);
    // The old columns went with the old children.
    CHECK(oldColumn->IsDetached());
    CHECK(table.Columns().size() == 1);
    CHECK_FALSE(table.Columns()[0]->IsDetached());
  }
}
