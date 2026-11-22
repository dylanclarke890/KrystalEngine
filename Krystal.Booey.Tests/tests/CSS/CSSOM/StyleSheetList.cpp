#include "Krystal.Booey/CSS/CSSOM/StyleSheetList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey.Tests/DOM/DOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/MediaList.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"

namespace krys::boo::css::tests
{
  using boo::tests::ParseDocument;
  using boo::tests::RunTasks;

  TEST_CASE_METHOD(ValueFixture, "StyleSheetList: a document's sheets",
                   "[CSS][CSSOM][StyleSheetList][cssom-1:the-stylesheetlist-interface]")
  {
    RunTasks();
    auto document =
      ParseDocument(u8"<!DOCTYPE html><style>a {}</style><style media=print>b {}</style><body>");
    auto &list = document->StyleSheets();
    CHECK(&list == &document->StyleSheets());
    CHECK(list.OwnerNode() == document.get());

    REQUIRE(list.Length() == 2u);
    REQUIRE(list.Item(0));
    REQUIRE(list.Item(1));
    CHECK(ToNarrow(list.Item(1)->Media()->MediaText()) == "print");
    CHECK(list.Item(2) == nullptr);
    CHECK(list.IsSupportedPropertyIndex(1));
    CHECK_FALSE(list.IsSupportedPropertyIndex(2));

    SECTION("it's live")
    {
      auto first = ShareRefPtr(DynamicDowncast<Element>(list.Item(0)->OwnerNode()));
      REQUIRE(first);
      REQUIRE_FALSE(first->Remove().HasException());
      CHECK(list.Length() == 1u);

      auto style = document->CreateElement(u8"style").ReleaseValue();
      REQUIRE_FALSE(style->TextContent(u8"c {}").HasException());
      REQUIRE_FALSE(document->Body()->AppendChild(*style).HasException());
      REQUIRE(list.Length() == 2u);
      CHECK(list.Item(1)->OwnerNode() == style.get());
    }

    SECTION("adopted sheets aren't in it")
    {
      auto sheet = CSSStyleSheet::Create(*document, {}).ReleaseValue();
      REQUIRE_FALSE(document->SetAdoptedStyleSheets({sheet}).HasException());
      RunTasks();
      CHECK(list.Length() == 2u);
    }

    SECTION("detached, as when its document goes, it keeps the sheets it had")
    {
      list.Detach();
      CHECK(list.OwnerNode() == nullptr);
      auto style = document->CreateElement(u8"style").ReleaseValue();
      REQUIRE_FALSE(document->Body()->AppendChild(*style).HasException());
      REQUIRE(list.Length() == 2u);
      CHECK(ToNarrow(list.Item(1)->Media()->MediaText()) == "print");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetList: a shadow root's sheets",
                   "[CSS][CSSOM][StyleSheetList][cssom-1:the-stylesheetlist-interface]")
  {
    RunTasks();
    auto document = ParseDocument(u8"<!DOCTYPE html><style>a {}</style><body>");
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->Body()->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto &list = shadow->StyleSheets();
    CHECK(list.OwnerNode() == shadow.get());
    CHECK(list.Length() == 0u);

    auto style = document->CreateElement(u8"style").ReleaseValue();
    REQUIRE_FALSE(style->TextContent(u8"b {}").HasException());
    REQUIRE_FALSE(shadow->AppendChild(*style).HasException());
    REQUIRE(list.Length() == 1u);
    CHECK(list.Item(0)->OwnerNode() == style.get());
    CHECK(document->StyleSheets().Length() == 1u);
  }
}
