#include "Krystal.Booey/HTML/Elements/HTMLTableSectionElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/TableTestUtils.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTableRowElement.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLTableSectionElement: rows, insertRow and deleteRow",
            "[HTML][HTMLTableSectionElement][html:dom-tbody-rows][html:dom-tbody-insertrow][html:dom-tbody-"
            "deleterow]")
  {
    TableDocument data;
    auto &table = data.Table(u8"<tbody><tr id=a></tr><tr id=b></tr></tbody>");
    auto &body = *table.TBodies()->Item(0);
    auto &section = Downcast<HTMLTableSectionElement>(Downcast<HTMLElement>(body));
    auto rows = section.Rows();
    CHECK(TableIds(*rows) == List<std::string> {"a", "b"});
    CHECK(section.Rows().get() == rows.get());

    auto row = section.InsertRow(1);
    REQUIRE_FALSE(row.HasException());
    CHECK(rows->Item(1).get() == row.Value().get());
    CHECK(section.InsertRow(3).HasValue());
    auto result = section.InsertRow(5);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == dom::ExceptionCode::IndexSizeError);

    REQUIRE_FALSE(section.DeleteRow(0).HasException());
    CHECK(rows->Length() == 3);
    REQUIRE_FALSE(section.DeleteRow(-1).HasException());
    CHECK(rows->Length() == 2);
    auto deleteResult = section.DeleteRow(2);
    REQUIRE(deleteResult.HasException());
    CHECK(deleteResult.GetException().Code() == dom::ExceptionCode::IndexSizeError);
  }

}
