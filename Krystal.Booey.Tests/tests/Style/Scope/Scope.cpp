#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/StyleSheets/CSSStyleSheet.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    void RunTasks() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
    }

    Ref<html::HTMLLinkElement> AppendLink(dom::Node &parent, dom::DOMString &&rel) noexcept
    {
      auto link = CreateRef<html::HTMLLinkElement>(parent.NodeDocument());
      REQUIRE_FALSE(link->SetAttribute(u8"rel", krys::move(rel)).HasException());
      REQUIRE_FALSE(parent.AppendChild(*link).HasException());
      return link;
    }

    /// @brief An element that calls back while its style is resolved and its renderers are attached.
    class StyleCallbackElement final : public html::HTMLDivElement
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(StyleCallbackElement);

    public:
      explicit StyleCallbackElement(dom::Document &document) noexcept
          : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
      {
      }

      void WillRecalcStyle(EnumSet<Change>) noexcept override
      {
        if (DuringStyleResolution)
        {
          std::exchange(DuringStyleResolution, nullptr)();
        }
      }

      void DidAttachRenderers() noexcept override
      {
        if (DuringRenderTreeUpdate)
        {
          std::exchange(DuringRenderTreeUpdate, nullptr)();
        }
      }

      Func<void()> DuringStyleResolution;
      Func<void()> DuringRenderTreeUpdate;
    };

    bool Contains(const List<WeakPtr<dom::ShadowRoot>> &roots, const dom::ShadowRoot &shadowRoot) noexcept
    {
      return std::ranges::any_of(roots, [&](const WeakPtr<dom::ShadowRoot> &root)
                                 { return root.get() == &shadowRoot; });
    }
  }

  TEST_CASE("Scope: shadow trees follow their host's connection", "[Style][Scope][DOM]")
  {
    auto document = Parse(u8"<div></div>");
    RefPtr<dom::Element> host = document->Body()->FirstElementChild();
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto inner = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    SECTION("Attaching a shadow root to a connected host connects it")
    {
      REQUIRE(shadow->IsConnected());
      REQUIRE(inner->IsConnected());
      REQUIRE(Contains(document->InDocumentShadowRoots(), *shadow));
    }

    SECTION("Removing and reinserting the host")
    {
      auto shadowRefs = shadow->GetRefCount();
      auto innerRefs = inner->GetRefCount();

      REQUIRE_FALSE(host->Remove().HasException());
      REQUIRE_FALSE(shadow->IsConnected());
      REQUIRE_FALSE(inner->IsConnected());
      REQUIRE(dom::TreeQueries::IsInShadowTree(*inner));
      REQUIRE_FALSE(Contains(document->InDocumentShadowRoots(), *shadow));
      REQUIRE(shadow->GetRefCount() == shadowRefs);

      REQUIRE_FALSE(document->Body()->AppendChild(*host).HasException());
      REQUIRE(shadow->IsConnected());
      REQUIRE(inner->IsConnected());
      REQUIRE(Contains(document->InDocumentShadowRoots(), *shadow));
      REQUIRE(shadow->GetRefCount() == shadowRefs);
      REQUIRE(inner->GetRefCount() == innerRefs);
    }

    SECTION("Moving the host keeps the shadow tree connected")
    {
      auto other = document->CreateElement(u8"p").ReleaseValue();
      REQUIRE_FALSE(document->Body()->AppendChild(*other).HasException());
      REQUIRE_FALSE(other->MoveBefore(*host, nullptr).HasException());
      REQUIRE(shadow->IsConnected());
      REQUIRE(inner->IsConnected());
      REQUIRE(dom::TreeQueries::IsInShadowTree(*inner));
    }

    SECTION("Descendants keep one tree reference across insertions")
    {
      auto div = document->CreateElement(u8"div").ReleaseValue();
      auto child = document->CreateElement(u8"b").ReleaseValue();
      REQUIRE_FALSE(div->AppendChild(*child).HasException());
      auto childRefs = child->GetRefCount();

      for (int i = 0; i < 3; i++)
      {
        REQUIRE_FALSE(document->Body()->AppendChild(*div).HasException());
        REQUIRE_FALSE(div->Remove().HasException());
      }
      REQUIRE(child->GetRefCount() == childRefs);
    }
  }

  TEST_CASE("Scope: the scope of a node", "[Style][Scope]")
  {
    auto document = Parse(u8"<div></div>");
    RefPtr<dom::Element> host = document->Body()->FirstElementChild();
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto inner = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    REQUIRE(&Scope::ForNode(*host) == &document->StyleScope());
    REQUIRE(&Scope::ForNode(*inner) == &shadow->StyleScope());
    REQUIRE(&Scope::ForNode(*shadow) == &shadow->StyleScope());
    REQUIRE(&Scope::ForNode(*document->CreateElement(u8"p").ReleaseValue()) == &document->StyleScope());

    REQUIRE(document->StyleScope().ShadowRoot() == nullptr);
    REQUIRE(document->StyleScope().HostScope() == nullptr);
    REQUIRE(shadow->StyleScope().ShadowRoot() == shadow.get());
    REQUIRE(&shadow->StyleScope().Document() == document.get());
    REQUIRE(shadow->StyleScope().HostScope() == &document->StyleScope());

    SECTION("Adopting a shadow host gives its shadow root a scope for the new document")
    {
      auto otherDocument = CreateRef<dom::HTMLDocument>();
      REQUIRE_FALSE(otherDocument->AdoptNode(*host).HasException());
      REQUIRE(&shadow->StyleScope().Document() == otherDocument.get());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Scope: active style sheets", "[Style][Scope]")
  {
    RunTasks();

    SECTION("Empty and disabled sheets are listed but not active")
    {
      auto document = Parse(u8"<!DOCTYPE html><style>a {}</style><style></style><style>b {}</style>");
      auto &scope = document->StyleScope();
      auto &sheets = scope.StyleSheetsForStyleSheetList();
      REQUIRE(sheets.size() == 3uz);
      REQUIRE(scope.ActiveStyleSheets().size() == 2uz);

      auto &third = Downcast<css::CSSStyleSheet>(*sheets[2]);
      third.SetDisabled(true);
      REQUIRE(scope.HasPendingUpdate());
      RunTasks();
      REQUIRE(scope.ActiveStyleSheets().size() == 1uz);
      REQUIRE_FALSE(scope.ActiveStyleSheetsContains(third));
    }

    SECTION("The first titled sheet sets the preferred set")
    {
      auto document = Parse(u8"<!DOCTYPE html><style>a {}</style><style title=Default>b {}</style>"
                            u8"<style title=Other>c {}</style><style title=Default>d {}</style>");
      auto &scope = document->StyleScope();
      auto &sheets = scope.StyleSheetsForStyleSheetList();
      REQUIRE(sheets.size() == 4uz);

      auto active = [&](uint32 index)
      {
        return scope.ActiveStyleSheetsContains(Downcast<css::CSSStyleSheet>(*sheets[index]));
      };
      REQUIRE(active(0));
      REQUIRE(active(1));
      REQUIRE_FALSE(active(2));
      REQUIRE(active(3));

      scope.SetPreferredStylesheetSetName(u8"Other");
      scope.FlushPendingUpdate();
      REQUIRE(active(0));
      REQUIRE_FALSE(active(1));
      REQUIRE(active(2));
      REQUIRE_FALSE(active(3));
    }
  }

  TEST_CASE("Scope: updates", "[Style][Scope]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><body>");
    auto &scope = document->StyleScope();
    auto *body = document->Body().get();

    SECTION("Updates wait for the event loop")
    {
      scope.DidChangeActiveStyleSheetCandidates();
      REQUIRE(scope.HasPendingUpdate());
      RunTasks();
      REQUIRE_FALSE(scope.HasPendingUpdate());
    }

    SECTION("Reading the sheets flushes the update")
    {
      scope.DidChangeStyleSheetContents();
      REQUIRE(scope.StyleSheetsForStyleSheetList().empty());
      REQUIRE_FALSE(scope.HasPendingUpdate());
    }

    SECTION("A shadow root's update is flushed with its document's")
    {
      auto host = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(body->AppendChild(*host).HasException());
      auto shadow = host->AttachShadow({}).ReleaseValue();

      auto &shadowScope = shadow->StyleScope();
      shadowScope.DidChangeActiveStyleSheetCandidates();
      REQUIRE(scope.HasPendingUpdate());
      scope.FlushPendingUpdate();
      REQUIRE_FALSE(shadowScope.HasPendingUpdate());
      REQUIRE_FALSE(scope.HasPendingUpdate());
    }

    SECTION("Links without sheets provide nothing")
    {
      auto link = AppendLink(*body, u8"stylesheet");
      scope.AddStyleSheetCandidateNode(*link, false);
      scope.DidChangeActiveStyleSheetCandidates();
      REQUIRE(scope.StyleSheetsForStyleSheetList().empty());
      REQUIRE(scope.ActiveStyleSheets().empty());

      scope.RemoveStyleSheetCandidateNode(*link);
      REQUIRE(scope.HasPendingUpdate());
      scope.FlushPendingUpdate();
    }
  }

  TEST_CASE("Scope: updates while style is being resolved", "[Style][Scope]")
  {
    ResolverFixture fixture(u8"<!DOCTYPE html><body><div id=host></div>");
    (void)fixture.Document->UpdateStyleIfNeeded();
    auto &scope = fixture.Document->StyleScope();
    auto element = CreateRef<StyleCallbackElement>(*fixture.Document);
    bool called = false;

    SECTION("wait until style resolution has finished")
    {
      element->DuringStyleResolution = [&]
      {
        called = true;
        CHECK(fixture.Document->InStyleRecalc());
        scope.DidChangeActiveStyleSheetCandidates();
        scope.FlushPendingUpdate();
        CHECK(scope.HasPendingUpdate());
      };
      REQUIRE_FALSE(fixture.Document->Body()->AppendChild(*element).HasException());
      (void)fixture.Document->UpdateStyleIfNeeded();
      REQUIRE(called);
      CHECK(scope.HasPendingUpdate());

      html::EventLoop::Current().RunUntilIdle();
      CHECK_FALSE(scope.HasPendingUpdate());
    }

    SECTION("wait until the render tree has been updated")
    {
      element->DuringRenderTreeUpdate = [&]
      {
        called = true;
        CHECK(fixture.Document->InRenderTreeUpdate());
        scope.DidChangeActiveStyleSheetCandidates();
        scope.FlushPendingUpdate();
        CHECK(scope.HasPendingUpdate());
      };
      REQUIRE_FALSE(fixture.Document->Body()->AppendChild(*element).HasException());
      (void)fixture.Document->UpdateStyleIfNeeded();
      REQUIRE(called);
      CHECK(fixture.Document->HasPendingFullStyleRebuild());

      (void)fixture.Document->UpdateStyleIfNeeded();
      CHECK_FALSE(scope.HasPendingUpdate());
      CHECK_FALSE(fixture.Document->HasPendingFullStyleRebuild());
    }

    SECTION("of a shadow root are still flushed with its document's")
    {
      auto shadow = fixture.ById(u8"host").AttachShadow({}).ReleaseValue();
      auto &shadowScope = shadow->StyleScope();
      element->DuringStyleResolution = [&]
      {
        called = true;
        shadowScope.DidChangeActiveStyleSheetCandidates();
        scope.FlushPendingUpdate();
        CHECK(shadowScope.HasPendingUpdate());
      };
      REQUIRE_FALSE(fixture.Document->Body()->AppendChild(*element).HasException());
      (void)fixture.Document->UpdateStyleIfNeeded();
      REQUIRE(called);
      CHECK(scope.HasPendingUpdate());

      scope.FlushPendingUpdate();
      CHECK_FALSE(shadowScope.HasPendingUpdate());
      CHECK_FALSE(scope.HasPendingUpdate());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Scope: updates without a render tree", "[Style][Scope]")
  {
    RunTasks();

    SECTION("still list the sheets")
    {
      auto document = CreateRef<dom::HTMLDocument>();
      REQUIRE_FALSE(document->HasLivingRenderTree());
      auto style = document->CreateElement(u8"style").ReleaseValue();
      REQUIRE_FALSE(style->TextContent(u8"a{}").HasException());
      REQUIRE_FALSE(document->AppendChild(*style).HasException());
      REQUIRE(document->StyleScope().HasPendingUpdate());

      CHECK(document->StyleScope().StyleSheetsForStyleSheetList().size() == 1);
      CHECK(document->StyleScope().ActiveStyleSheets().size() == 1);
    }

    SECTION("leave the resolver to be built from the sheets when it's asked for")
    {
      auto document = Parse(u8"<!DOCTYPE html><style>a{}</style><body>");
      auto &scope = document->StyleScope();
      scope.FlushPendingUpdate();
      CHECK_FALSE(scope.ResolverIfExists());
      CHECK(scope.GetResolver().RuleSets().AuthorStyle().RuleCount() == 1);
    }
  }

  TEST_CASE("Scope: pending sheets", "[Style][Scope]")
  {
    RunTasks();
    auto document = Parse(u8"<!DOCTYPE html><head><link></head><body><link>");
    auto &scope = document->StyleScope();
    auto headLink = document->DocumentElement()->FirstElementChild()->FirstElementChild();
    auto bodyLink = document->Body()->FirstElementChild();

    REQUIRE_FALSE(scope.HasPendingSheets());
    scope.AddPendingSheet(*headLink);
    scope.AddPendingSheet(*bodyLink);
    REQUIRE(scope.HasPendingSheetsBeforeBody());
    REQUIRE(scope.HasPendingSheetsInBody());
    REQUIRE(scope.HasPendingSheet(*headLink));
    REQUIRE_FALSE(scope.HasPendingSheetInBody(*headLink));
    REQUIRE(scope.HasPendingSheetInBody(*bodyLink));

    scope.RemovePendingSheet(*headLink);
    REQUIRE_FALSE(scope.HasPendingSheetsBeforeBody());
    REQUIRE_FALSE(scope.HasPendingUpdate());

    scope.RemovePendingSheet(*bodyLink);
    REQUIRE_FALSE(scope.HasPendingSheets());
    REQUIRE(scope.HasPendingUpdate());
    RunTasks();
  }

  TEST_CASE("Scope: a change of viewport size restyles pseudo-elements using viewport units",
            "[Style][Scope]")
  {
    // The element shrinks to fit its ::before, so its width is the pseudo-element's; its own style uses no
    // viewport units.
    ResolverFixture fixture(u8"<!DOCTYPE html><style>body{margin:0}#p{display:inline-block}"
                            u8"#p::before{content:'';display:block;width:50vw}</style><span id=p></span>");
    auto &element = *fixture.Document->GetElementById(u8"p");
    auto width = [&]() -> int32
    {
      (void)fixture.Document->UpdateLayout();
      return Downcast<html::HTMLElement>(element).OffsetWidth();
    };
    REQUIRE(width() == 400);

    fixture.Frame->View()->Resize({1'000, 600});
    CHECK(width() == 500);
    fixture.Frame->View()->Resize({600, 600});
    CHECK(width() == 300);
  }

  TEST_CASE("Scope: a change of viewport size restyles the elements using viewport units", "[Style][Scope]")
  {
    ResolverFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0}.v{display:block;width:50vw}.n{display:none}"
      u8".f{display:block;width:10px}</style><div id=a class=v></div><div id=b class=f></div>"
      u8"<div id=host></div>");
    auto width = [&](dom::Element &element) -> int32
    {
      (void)fixture.Document->UpdateLayout();
      return Downcast<html::HTMLElement>(element).OffsetWidth();
    };
    auto resize = [&](int32 viewportWidth)
    {
      fixture.Frame->View()->Resize({viewportWidth, 600});
      (void)fixture.Document->UpdateLayout();
    };
    auto &a = *fixture.Document->GetElementById(u8"a");
    auto &b = *fixture.Document->GetElementById(u8"b");
    REQUIRE(width(a) == 400);

    SECTION("one that has them from the start")
    {
      resize(1'000);
      CHECK(width(a) == 500);
      CHECK(width(b) == 10);
      resize(600);
      CHECK(width(a) == 300);
    }

    SECTION("one that gains and loses them")
    {
      REQUIRE_FALSE(b.SetAttribute(u8"class", u8"v").HasException());
      CHECK(width(b) == 400);
      resize(1'000);
      CHECK(width(b) == 500);

      REQUIRE_FALSE(b.SetAttribute(u8"class", u8"f").HasException());
      CHECK(width(b) == 10);
      resize(600);
      CHECK(width(b) == 10);

      REQUIRE_FALSE(b.SetAttribute(u8"class", u8"v").HasException());
      CHECK(width(b) == 300);
      resize(1'000);
      CHECK(width(b) == 500);
    }

    SECTION("one that's hidden when the size changes, and shown after")
    {
      REQUIRE_FALSE(a.SetAttribute(u8"class", u8"n").HasException());
      (void)fixture.Document->UpdateLayout();
      resize(1'000);
      REQUIRE_FALSE(a.SetAttribute(u8"class", u8"v").HasException());
      CHECK(width(a) == 500);
      resize(600);
      CHECK(width(a) == 300);
    }

    SECTION("one that leaves the tree and returns")
    {
      Ref<dom::Element> kept = ShareRef(a);
      auto parent = ShareRefPtr(a.ParentNode());
      REQUIRE_FALSE(parent->RemoveChild(a).HasException());
      resize(1'000);
      REQUIRE_FALSE(parent->AppendChild(a).HasException());
      CHECK(width(a) == 500);
      resize(600);
      CHECK(width(a) == 300);
    }

    SECTION("one that is destroyed")
    {
      {
        Ref<dom::Element> removed = ShareRef(a);
        REQUIRE_FALSE(a.ParentNode()->RemoveChild(a).HasException());
      }
      resize(1'000);
      CHECK(width(b) == 10);
    }

    SECTION("more destroyed than the scope lets pile up")
    {
      auto &body = *fixture.Document->Body();
      for (int i = 0; i < 200; ++i)
      {
        auto element = fixture.Document->CreateElement(u8"div").ReleaseValue();
        REQUIRE_FALSE(element->SetAttribute(u8"class", u8"v").HasException());
        REQUIRE_FALSE(body.AppendChild(*element).HasException());
        (void)fixture.Document->UpdateLayout();
        REQUIRE_FALSE(body.RemoveChild(*element).HasException());
      }
      resize(1'000);
      CHECK(width(a) == 500);
    }

    SECTION("one in a shadow tree, and one moved into and out of it")
    {
      auto &host = *fixture.Document->GetElementById(u8"host");
      auto shadow = host.AttachShadow({}).ReleaseValue();
      auto style = fixture.Document->CreateElement(u8"style").ReleaseValue();
      (void)style->TextContent(u8".s{display:block;width:25vw}");
      REQUIRE_FALSE(shadow->AppendChild(*style).HasException());
      auto inner = fixture.Document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(inner->SetAttribute(u8"class", u8"s").HasException());
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
      CHECK(width(*inner) == 200);
      resize(1'000);
      CHECK(width(*inner) == 250);

      // In the shadow tree, a has no rule; with the same class as inner, it's using its units there.
      Ref<dom::Element> kept = ShareRef(a);
      REQUIRE_FALSE(a.SetAttribute(u8"class", u8"s v").HasException());
      REQUIRE_FALSE(shadow->AppendChild(a).HasException());
      CHECK(width(a) == 250);
      resize(600);
      CHECK(width(a) == 150);

      REQUIRE_FALSE(a.ParentNode()->RemoveChild(a).HasException());
      REQUIRE_FALSE(fixture.Document->Body()->AppendChild(a).HasException());
      CHECK(width(a) == 300);
      resize(1'000);
      CHECK(width(a) == 500);
      CHECK(width(*inner) == 250);
    }
  }
}
