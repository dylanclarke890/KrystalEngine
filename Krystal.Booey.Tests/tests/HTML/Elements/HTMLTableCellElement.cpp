#include "Krystal.Booey/HTML/Elements/HTMLTableCellElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/TableTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD HTMLTableCellElement &Cell(dom::Element &element) noexcept
    {
      return Downcast<HTMLTableCellElement>(Downcast<HTMLElement>(element));
    }
  }

  TEST_CASE("HTMLTableCellElement.cellIndex: the cell's index in its row's cells, or -1 without a row",
            "[HTML][HTMLTableCellElement][html:dom-tdth-cellindex]")
  {
    TableDocument data;
    auto &table = data.Table(u8"<tr><td id=a></td><th id=b></th></tr>");
    auto &row = Downcast<HTMLTableRowElement>(Downcast<HTMLElement>(*table.Rows()->Item(0)));
    CHECK(Cell(*row.Cells()->Item(0)).CellIndex() == 0);
    CHECK(Cell(*row.Cells()->Item(1)).CellIndex() == 1);

    auto orphan = CreateTableElement<HTMLTableCellElement>(*data.Document, u8"td");
    CHECK(orphan->CellIndex() == -1);
  }

  TEST_CASE(
    "HTMLTableCellElement.scope: reflects its known values, and the empty string for auto",
    "[HTML][HTMLTableCellElement][html:dom-th-scope][html:attr-th-scope][html:attr-th-scope-auto-state]")
  {
    TableDocument data;
    auto &table = data.Table(u8"<tr><th id=h></th></tr>");
    auto &cell =
      Cell(*Downcast<HTMLTableRowElement>(Downcast<HTMLElement>(*table.Rows()->Item(0))).Cells()->Item(0));
    CHECK(TableText(cell.Scope()) == "");
    cell.Scope(u8"ROWGROUP");
    CHECK(TableText(cell.Scope()) == "rowgroup");
    cell.Scope(u8"column");
    CHECK(TableText(cell.Scope()) == "");
    CHECK(cell.GetAttribute(u8"scope") == u8"column");
  }

  TEST_CASE("HTMLTableCellElement: colSpan and rowSpan are clamped, with a default of 1",
            "[HTML][HTMLTableCellElement][html:attr-tdth-colspan][html:attr-tdth-rowspan]")
  {
    TableDocument data;
    auto &table =
      data.Table(u8"<tr><td colspan=0 rowspan=0></td><td colspan=5000 rowspan=70000></td><td></td></tr>");
    auto cells = Downcast<HTMLTableRowElement>(Downcast<HTMLElement>(*table.Rows()->Item(0))).Cells();
    CHECK(Cell(*cells->Item(0)).ColSpan() == 1u);
    CHECK(Cell(*cells->Item(0)).RowSpan() == 0u);
    CHECK(Cell(*cells->Item(1)).ColSpan() == 1'000u);
    CHECK(Cell(*cells->Item(1)).RowSpan() == 65'534u);
    CHECK(Cell(*cells->Item(2)).ColSpan() == 1u);
    CHECK(Cell(*cells->Item(2)).RowSpan() == 1u);
  }
}
