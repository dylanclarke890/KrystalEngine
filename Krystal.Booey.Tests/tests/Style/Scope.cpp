#include "Krystal.Booey/Style/Scope.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
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

    Ref<html::HTMLLinkElement> AppendLink(dom::Node &parent, dom::DOMString &&rel) noexcept
    {
      auto link = CreateRef<html::HTMLLinkElement>(parent.NodeDocument());
      REQUIRE_FALSE(link->SetAttribute(u8"rel", krys::move(rel)).HasException());
      REQUIRE_FALSE(parent.AppendChild(*link).HasException());
      return link;
    }

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

  // FIXME: The active style sheets, their tree order, filtering and style sheet sets are tested with e4 of
  // the DOM integration, when `<style>` elements own sheets.
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
}
