#include "Krystal.Booey/CSS/CSSStyleSheetObservableArray.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    void RunTasks() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
    }

    KRYS_NODISCARD Ref<CSSStyleSheet> ConstructedSheet(dom::Document &document,
                                                       const CSSOMString &text = u8"a {}")
    {
      auto sheet = CSSStyleSheet::Create(document, {});
      REQUIRE(sheet.HasValue());
      REQUIRE_FALSE(sheet.Value()->ReplaceSync(CSSOMString(text)).HasException());
      return sheet.ReleaseValue();
    }

    KRYS_NODISCARD bool IsAdoptedBy(const CSSStyleSheet &sheet, const dom::ContainerNode &treeScope) noexcept
    {
      return std::ranges::any_of(sheet.AdoptingTreeScopes(), [&](const WeakPtr<dom::ContainerNode> &scope)
                                 { return scope.get() == &treeScope; });
    }

    void RequireNotAllowed(const dom::ExceptionOr<void> &result) noexcept
    {
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == dom::ExceptionCode::NotAllowedError);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSStyleSheetObservableArray: adding sheets",
                   "[CSS][CSSOM][CSSStyleSheetObservableArray]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><body>");
    auto &adopted = document->AdoptedStyleSheets();
    auto sheet = ConstructedSheet(*document);
    REQUIRE(adopted.Length() == 0u);
    REQUIRE(sheet->IsDetached());

    SECTION("Constructed sheets can be adopted")
    {
      REQUIRE_FALSE(document->SetAdoptedStyleSheets({sheet}).HasException());
      REQUIRE(adopted.Length() == 1u);
      REQUIRE(adopted.ValueAt(0) == sheet.get());
      REQUIRE(adopted.ValueAt(1) == nullptr);
      REQUIRE(document->GetAdoptedStyleSheets().size() == 1uz);
      REQUIRE(IsAdoptedBy(*sheet, *document));
      REQUIRE_FALSE(sheet->IsDetached());
    }

    SECTION("Only sheets constructed for the same document")
    {
      auto contentsSheet = SheetFromText(u8"a {}");
      RequireNotAllowed(adopted.SetSheets({contentsSheet}));

      auto otherDocument = CreateRef<dom::HTMLDocument>();
      auto otherSheet = ConstructedSheet(*otherDocument);
      RequireNotAllowed(adopted.SetValueAt(0, *otherSheet));

      // A failed replacement leaves the array as it was.
      REQUIRE_FALSE(adopted.SetSheets({sheet}).HasException());
      RequireNotAllowed(document->SetAdoptedStyleSheets({sheet, otherSheet}));
      REQUIRE(adopted.Length() == 1u);
      REQUIRE(adopted.ValueAt(0) == sheet.get());
      REQUIRE_FALSE(IsAdoptedBy(*otherSheet, *document));
    }

    SECTION("Setting, removing and shrinking")
    {
      auto second = ConstructedSheet(*document);
      REQUIRE_FALSE(adopted.SetValueAt(0, *sheet).HasException());
      REQUIRE_FALSE(adopted.SetValueAt(1, *second).HasException());
      REQUIRE(adopted.Length() == 2u);

      // Replacing a sheet releases it.
      auto third = ConstructedSheet(*document);
      REQUIRE_FALSE(adopted.SetValueAt(0, *third).HasException());
      REQUIRE(adopted.ValueAt(0) == third.get());
      REQUIRE_FALSE(IsAdoptedBy(*sheet, *document));
      REQUIRE(sheet->IsDetached());

      adopted.RemoveLast();
      REQUIRE(adopted.Length() == 1u);
      REQUIRE_FALSE(IsAdoptedBy(*second, *document));

      adopted.ShrinkTo(0);
      REQUIRE(adopted.Length() == 0u);
      REQUIRE_FALSE(IsAdoptedBy(*third, *document));
    }

    SECTION("A sheet adopted twice stays adopted until both are removed")
    {
      REQUIRE_FALSE(adopted.SetSheets({sheet, sheet}).HasException());
      REQUIRE(sheet->AdoptingTreeScopes().size() == 1uz);
      adopted.RemoveLast();
      REQUIRE(IsAdoptedBy(*sheet, *document));
      adopted.RemoveLast();
      REQUIRE_FALSE(IsAdoptedBy(*sheet, *document));
    }

    SECTION("A sheet can be adopted by several tree scopes")
    {
      auto host = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(document->Body()->AppendChild(*host).HasException());
      auto shadow = host->AttachShadow({}).ReleaseValue();

      REQUIRE_FALSE(document->SetAdoptedStyleSheets({sheet}).HasException());
      REQUIRE_FALSE(shadow->SetAdoptedStyleSheets({sheet}).HasException());
      REQUIRE(IsAdoptedBy(*sheet, *document));
      REQUIRE(IsAdoptedBy(*sheet, *shadow));

      REQUIRE_FALSE(document->SetAdoptedStyleSheets({}).HasException());
      REQUIRE_FALSE(IsAdoptedBy(*sheet, *document));
      REQUIRE_FALSE(sheet->IsDetached());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSStyleSheetObservableArray: adopted sheets are active",
                   "[CSS][CSSOM][CSSStyleSheetObservableArray][Style][Scope]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><style>a {}</style>");
    auto &scope = document->StyleScope();
    auto sheet = ConstructedSheet(*document);

    SECTION("After the owned sheets, in the style sheet list too")
    {
      REQUIRE_FALSE(document->SetAdoptedStyleSheets({sheet}).HasException());
      REQUIRE(scope.HasPendingUpdate());
      RunTasks();
      REQUIRE(scope.ActiveStyleSheets().size() == 2uz);
      REQUIRE(scope.ActiveStyleSheets()[1].get() == sheet.get());
      REQUIRE(document->StyleSheets().Length() == 2u);
      REQUIRE(document->StyleSheets().Item(1) == sheet.get());

      REQUIRE_FALSE(document->SetAdoptedStyleSheets({}).HasException());
      REQUIRE(scope.HasPendingUpdate());
      RunTasks();
      REQUIRE(scope.ActiveStyleSheets().size() == 1uz);
      REQUIRE(document->StyleSheets().Length() == 1u);
    }

    SECTION("Changing an adopted sheet updates its tree scopes")
    {
      REQUIRE_FALSE(document->SetAdoptedStyleSheets({sheet}).HasException());
      RunTasks();
      REQUIRE_FALSE(sheet->ReplaceSync(u8"b {}").HasException());
      REQUIRE(scope.HasPendingUpdate());
      RunTasks();

      sheet->SetDisabled(true);
      REQUIRE(scope.HasPendingUpdate());
      RunTasks();
      REQUIRE_FALSE(scope.ActiveStyleSheetsContains(*sheet));
      REQUIRE(document->StyleSheets().Length() == 1u);
    }

    SECTION("Titled sheets need the preferred set")
    {
      sheet->SetTitle(u8"Other");
      REQUIRE_FALSE(document->SetAdoptedStyleSheets({sheet}).HasException());
      RunTasks();
      REQUIRE_FALSE(scope.ActiveStyleSheetsContains(*sheet));

      scope.SetPreferredStylesheetSetName(u8"Other");
      scope.FlushPendingUpdate();
      REQUIRE(scope.ActiveStyleSheetsContains(*sheet));
    }

    SECTION("In a shadow root")
    {
      auto host = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(document->Body()->AppendChild(*host).HasException());
      auto shadow = host->AttachShadow({}).ReleaseValue();
      RunTasks();

      REQUIRE_FALSE(shadow->SetAdoptedStyleSheets({sheet}).HasException());
      RunTasks();
      REQUIRE(shadow->StyleScope().ActiveStyleSheetsContains(*sheet));
      REQUIRE_FALSE(scope.ActiveStyleSheetsContains(*sheet));
      REQUIRE(shadow->StyleSheets().Length() == 1u);

      SECTION("Adoption into another document drops them")
      {
        auto otherDocument = CreateRef<dom::HTMLDocument>();
        REQUIRE_FALSE(otherDocument->AdoptNode(*host).HasException());
        REQUIRE(shadow->GetAdoptedStyleSheets().empty());
        REQUIRE(sheet->IsDetached());
        RunTasks();
      }
    }

    SECTION("A sheet outlives the tree scope that adopted it")
    {
      // FIXME: Also a document, once documents can be freed (each holds a reference to itself as its node
      // document).
      {
        auto host = document->CreateElement(u8"div").ReleaseValue();
        auto shadow = host->AttachShadow({}).ReleaseValue();
        REQUIRE_FALSE(shadow->SetAdoptedStyleSheets({sheet}).HasException());
        REQUIRE_FALSE(sheet->IsDetached());
      }
      RunTasks();
      REQUIRE(sheet->IsDetached());
      REQUIRE_FALSE(sheet->ReplaceSync(u8"b {}").HasException());
    }
  }
}
