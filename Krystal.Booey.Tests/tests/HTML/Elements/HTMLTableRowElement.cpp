#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/TableTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableCellElement.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE(
    "HTMLTableRowElement: rowIndex and sectionRowIndex, the row's index in its table's or section's rows",
    "[HTML][HTMLTableRowElement][html:dom-tr-rowindex][html:dom-tr-sectionrowindex]")
  {
    TableDocument data;
    auto &table = data.Table(u8"<tbody><tr id=b1></tr><tr id=b2></tr></tbody><thead><tr id=h1></tr></thead>");
    auto rows = table.Rows();
    auto &b2 = Downcast<HTMLTableRowElement>(Downcast<HTMLElement>(*rows->NamedItem(u8"b2")));
    CHECK(b2.RowIndex() == 2);
    CHECK(b2.SectionRowIndex() == 1);

    auto orphan = CreateTableElement<HTMLTableRowElement>(*data.Document, u8"tr");
    CHECK(orphan->RowIndex() == -1);
    CHECK(orphan->SectionRowIndex() == -1);
  }

  TEST_CASE("HTMLTableRowElement: cells, insertCell and deleteCell",
            "[HTML][HTMLTableRowElement][html:dom-tr-cells][html:dom-tr-insertcell][html:dom-tr-deletecell]")
  {
    TableDocument data;
    auto &table = data.Table(u8"<tr><td id=a></td><th id=b></th></tr>");
    auto &row = Downcast<HTMLTableRowElement>(Downcast<HTMLElement>(*table.Rows()->Item(0)));
    auto cells = row.Cells();
    CHECK(TableIds(*cells) == List<std::string> {"a", "b"});
    CHECK(row.Cells().get() == cells.get());

    auto &b = Downcast<HTMLTableCellElement>(Downcast<HTMLElement>(*cells->Item(1)));
    CHECK(b.CellIndex() == 1);
    auto orphan = CreateTableElement<HTMLTableCellElement>(*data.Document, u8"td");
    CHECK(orphan->CellIndex() == -1);

    auto cell = row.InsertCell(0);
    REQUIRE_FALSE(cell.HasException());
    CHECK(cell.Value()->LocalName() == u8"td");
    CHECK(cells->Item(0).get() == cell.Value().get());
    CHECK(b.CellIndex() == 2);
    CHECK(row.InsertCell(-1).HasValue());
    CHECK(cells->Length() == 4);
    auto result = row.InsertCell(6);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == dom::ExceptionCode::IndexSizeError);

    REQUIRE_FALSE(row.DeleteCell(-1).HasException());
    REQUIRE_FALSE(row.DeleteCell(0).HasException());
    CHECK(TableIds(*cells) == List<std::string> {"a", "b"});
    auto deleteResult = row.DeleteCell(2);
    REQUIRE(deleteResult.HasException());
    CHECK(deleteResult.GetException().Code() == dom::ExceptionCode::IndexSizeError);
  }
}
