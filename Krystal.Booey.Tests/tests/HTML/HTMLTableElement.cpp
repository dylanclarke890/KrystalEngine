#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/TableTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLTableCaptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableRowElement.hpp"
#include "Krystal.Booey/HTML/HTMLTableSectionElement.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLTableElement: caption",
            "[HTML][HTMLTableElement][html:dom-table-caption][html:dom-table-createcaption]"
            "[html:dom-table-deletecaption][html:create-a-table-element]")
  {
    TableDocument data;

    SECTION("is the first caption child, created as the first child")
    {
      auto &table = data.Table(u8"<tr><td></td></tr>");
      CHECK_FALSE(table.Caption());
      auto caption = table.CreateCaption();
      CHECK(table.Caption() == caption.get());
      CHECK(table.FirstChild() == caption.get());
      CHECK(table.CreateCaption().get() == caption.get());

      table.DeleteCaption();
      CHECK_FALSE(table.Caption());
      CHECK(data.Markup() == "<tbody><tr><td></td></tr></tbody>");
    }

    SECTION("setting it replaces the first caption")
    {
      auto &table = data.Table(u8"<caption id=a></caption><caption id=b></caption>");
      auto caption = CreateTableElement<HTMLTableCaptionElement>(*data.Document, u8"caption");
      (void)caption->SetAttribute(dom::DOMStringAtom(u8"id"), u8"c");
      REQUIRE_FALSE(table.Caption(RefPtr<HTMLTableCaptionElement>(caption)).HasException());
      CHECK(data.Markup() == "<caption id=\"c\"></caption><caption id=\"b\"></caption>");
      REQUIRE_FALSE(table.Caption(nullptr).HasException());
      CHECK(data.Markup() == "<caption id=\"b\"></caption>");
    }
  }

  TEST_CASE(
    "HTMLTableElement: tHead and tFoot",
    "[HTML][HTMLTableElement][html:dom-table-thead][html:dom-table-createthead][html:dom-table-deletethead]"
    "[html:dom-table-tfoot][html:dom-table-createtfoot][html:dom-table-deletetfoot]")
  {
    TableDocument data;

    SECTION("createTHead inserts after the caption and column groups")
    {
      auto &table = data.Table(u8"<caption></caption><colgroup></colgroup><tbody></tbody>");
      auto head = table.CreateTHead();
      CHECK(table.THead() == head.get());
      CHECK(table.CreateTHead().get() == head.get());
      CHECK(data.Markup() == "<caption></caption><colgroup></colgroup><thead></thead><tbody></tbody>");
      table.DeleteTHead();
      CHECK_FALSE(table.THead());
    }

    SECTION("createTFoot appends")
    {
      auto &table = data.Table(u8"<tbody></tbody>");
      auto foot = table.CreateTFoot();
      CHECK(table.TFoot() == foot.get());
      CHECK(data.Markup() == "<tbody></tbody><tfoot></tfoot>");
      table.DeleteTFoot();
      CHECK(data.Markup() == "<tbody></tbody>");
    }

    SECTION("setting them checks the section's kind")
    {
      auto &table = data.Table(u8"");
      auto body = CreateTableElement<HTMLTableSectionElement>(*data.Document, u8"tbody");
      auto result = table.THead(RefPtr<HTMLTableSectionElement>(body));
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::HierarchyRequestError);
      result = table.TFoot(RefPtr<HTMLTableSectionElement>(body));
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::HierarchyRequestError);

      auto foot = CreateTableElement<HTMLTableSectionElement>(*data.Document, u8"tfoot");
      REQUIRE_FALSE(table.TFoot(RefPtr<HTMLTableSectionElement>(foot)).HasException());
      CHECK(table.TFoot() == foot.get());
    }
  }

  TEST_CASE("HTMLTableElement: tBodies and createTBody",
            "[HTML][HTMLTableElement][html:dom-table-tbodies][html:dom-table-createtbody]")
  {
    TableDocument data;
    auto &table = data.Table(u8"<tbody id=a></tbody><tfoot></tfoot><tbody id=b><tr><td><table><tbody "
                             u8"id=nested></tbody></table></td></tr></tbody>");
    auto tBodies = table.TBodies();
    CHECK(TableIds(*tBodies) == List<std::string> {"a", "b"});
    CHECK(table.TBodies().get() == tBodies.get());

    auto body = table.CreateTBody();
    (void)body->SetAttribute(dom::DOMStringAtom(u8"id"), u8"c");
    CHECK(TableIds(*tBodies) == List<std::string> {"a", "b", "c"});
    CHECK(table.LastElementChild() == body.get());
  }

  TEST_CASE("HTMLTableElement: rows", "[HTML][HTMLTableElement][html:dom-table-rows]")
  {
    TableDocument data;

    SECTION("are the heads' rows, then the bodies' and the table's own, then the feet's")
    {
      auto &table = data.Table(
        u8"<tfoot><tr id=f1></tr></tfoot><tbody><tr id=b1></tr><tr id=b2></tr></tbody>"
        u8"<thead><tr id=h1></tr></thead><tbody><tr id=b3></tr></tbody><thead><tr id=h2></tr></thead>");
      // A row directly in the table, which the parser never makes.
      auto row = CreateTableElement<HTMLTableRowElement>(*data.Document, u8"tr");
      (void)row->SetAttribute(dom::DOMStringAtom(u8"id"), u8"t1");
      (void)table.InsertBefore(*row, table.LastChild());

      auto rows = table.Rows();
      CHECK(TableIds(*rows) == List<std::string> {"h1", "h2", "b1", "b2", "b3", "t1", "f1"});
      CHECK(table.Rows().get() == rows.get());
      CHECK(rows->NamedItem(u8"b2").get() == table.Rows()->Item(3).get());
      CHECK_FALSE(rows->NamedItem(u8""));
    }

    SECTION("stay live")
    {
      auto &table = data.Table(u8"<tr id=a></tr>");
      auto rows = table.Rows();
      CHECK(rows->Length() == 1);
      (void)table.InsertRow();
      CHECK(rows->Length() == 2);
    }
  }

  TEST_CASE("HTMLTableElement: insertRow and deleteRow",
            "[HTML][HTMLTableElement][html:dom-table-insertrow][html:dom-table-deleterow]")
  {
    TableDocument data;

    SECTION("insertRow makes a body for a table with none")
    {
      auto &table = data.Table(u8"");
      auto row = table.InsertRow();
      REQUIRE_FALSE(row.HasException());
      CHECK(data.Markup() == "<tbody><tr></tr></tbody>");
    }

    SECTION("insertRow goes into the last body when there are no rows")
    {
      auto &table = data.Table(u8"<tbody id=a></tbody><tbody id=b></tbody>");
      REQUIRE_FALSE(table.InsertRow(0).HasException());
      CHECK(data.Markup() == "<tbody id=\"a\"></tbody><tbody id=\"b\"><tr></tr></tbody>");
    }

    SECTION("insertRow inserts before the row at the index, or after the last")
    {
      auto &table = data.Table(u8"<thead><tr id=h></tr></thead><tbody><tr id=b></tr></tbody>");
      auto first = table.InsertRow(0);
      REQUIRE_FALSE(first.HasException());
      CHECK(first.Value()->ParentNode() == table.THead().get());
      auto last = table.InsertRow(-1);
      REQUIRE_FALSE(last.HasException());
      CHECK(table.Rows()->Item(3).get() == last.Value().get());
      auto end = table.InsertRow(4);
      REQUIRE_FALSE(end.HasException());
      CHECK(table.Rows()->Length() == 5);

      auto result = table.InsertRow(6);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::IndexSizeError);
      result = table.InsertRow(-2);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::IndexSizeError);
    }

    SECTION("deleteRow removes the row at the index, -1 for the last")
    {
      auto &table = data.Table(u8"<tr id=a></tr><tr id=b></tr><tr id=c></tr>");
      REQUIRE_FALSE(table.DeleteRow(1).HasException());
      CHECK(TableIds(*table.Rows()) == List<std::string> {"a", "c"});
      REQUIRE_FALSE(table.DeleteRow(-1).HasException());
      CHECK(TableIds(*table.Rows()) == List<std::string> {"a"});

      auto result = table.DeleteRow(1);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::IndexSizeError);
      result = table.DeleteRow(-2);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::IndexSizeError);

      REQUIRE_FALSE(table.DeleteRow(0).HasException());
      CHECK_FALSE(table.DeleteRow(-1).HasException());
    }
  }
}
