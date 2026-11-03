#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD bool Throws(const auto &result, ExceptionCode code) noexcept
    {
      return result.HasException() && result.GetException().Code() == code;
    }
  }

  TEST_CASE("XML document: any document that isn't an HTML document",
            "[DOM][DocumentAlgorithms][dom:xml-document][dom:html-document][dom:concept-document-type]")
  {
    CHECK(DocumentAlgorithms::IsXMLTypeDocument(*CreateRef<Document>()));
    CHECK(DocumentAlgorithms::IsXMLTypeDocument(*CreateRef<XMLDocument>()));
    CHECK_FALSE(DocumentAlgorithms::IsXMLTypeDocument(*CreateRef<HTMLDocument>()));
  }

  TEST_CASE("Flatten element creation options: the registry and is value, checked",
            "[DOM][DocumentAlgorithms][dom:flatten-element-creation-options]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto global = html::CustomElementRegistry::Create(*document);
    document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(global));
    auto scoped = html::CustomElementRegistry::CreateScoped();

    SECTION("a string gives the document's registry and no is value")
    {
      auto result = DocumentAlgorithms::FlattenElementCreationOptions(u8"x-a", *document);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value().CustomElementRegistry.get() == global.get());
      CHECK(result.Value().Is == DOMStringAtom::Null());
    }

    SECTION("3a. the is value")
    {
      auto result =
        DocumentAlgorithms::FlattenElementCreationOptions(ElementCreationOptions {.Is = u8"x-a"}, *document);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value().CustomElementRegistry.get() == global.get());
      CHECK(result.Value().Is == u8"x-a");
    }

    SECTION("3b. a scoped registry, but not with an is value")
    {
      auto result = DocumentAlgorithms::FlattenElementCreationOptions(
        ElementCreationOptions {.CustomElementRegistry = scoped}, *document);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value().CustomElementRegistry.get() == scoped.get());

      CHECK(Throws(DocumentAlgorithms::FlattenElementCreationOptions(
                     ElementCreationOptions {.CustomElementRegistry = scoped, .Is = u8"x-a"}, *document),
                   ExceptionCode::NotSupportedError));
    }

    SECTION("3c. another document's global registry")
    {
      auto other = CreateRef<HTMLDocument>();
      auto otherGlobal = html::CustomElementRegistry::Create(*other);
      CHECK(Throws(DocumentAlgorithms::FlattenElementCreationOptions(
                     ElementCreationOptions {.CustomElementRegistry = otherGlobal}, *document),
                   ExceptionCode::NotSupportedError));
      CHECK_FALSE(DocumentAlgorithms::FlattenElementCreationOptions(
                    ElementCreationOptions {.CustomElementRegistry = global}, *document)
                    .HasException());
    }
  }

  TEST_CASE("Internal createElementNS steps: the name is validated and extracted",
            "[DOM][DocumentAlgorithms][dom:internal-createelementns-steps][dom:dom-document-createelementns]")
  {
    auto document = CreateRef<XMLDocument>();

    auto element =
      DocumentAlgorithms::InternalCreateElementNS(*document, u8"urn:x", u8"p:Local", {}).ReleaseValue();
    CHECK(element->NamespaceURI() == u8"urn:x");
    CHECK(element->Prefix() == u8"p");
    CHECK(element->LocalName() == u8"Local");
    CHECK(&element->NodeDocument() == document.get());

    auto html = document->CreateElementNS(infra::Namespaces::HTML, u8"DIV").ReleaseValue();
    CHECK(html->LocalName() == u8"DIV");
    CHECK(Is<html::HTMLElement>(*html));

    CHECK(Throws(document->CreateElementNS(u8"urn:x", u8"1a"), ExceptionCode::InvalidCharacterError));
    CHECK(Throws(document->CreateElementNS(DOMStringAtom::Null(), u8"p:a"), ExceptionCode::NamespaceError));
    CHECK(Throws(document->CreateElementNS(u8"urn:x", u8"xml:a"), ExceptionCode::NamespaceError));
    CHECK(Throws(document->CreateElementNS(
                   u8"urn:x", u8"a",
                   ElementCreationOptions {
                     .CustomElementRegistry = html::CustomElementRegistry::CreateScoped(), .Is = u8"x-a"}),
                 ExceptionCode::NotSupportedError));
  }

  TEST_CASE(
    "Adopt: the node leaves its parent, and its shadow-including subtree and attributes move document",
    "[DOM][DocumentAlgorithms][dom:concept-node-adopt][dom:concept-node-adopt-ext]"
    "[dom:infrastructure][dom:other-applicable-specifications]")
  {
    auto oldDocument = CreateRef<HTMLDocument>();
    auto document = CreateRef<HTMLDocument>();
    auto parent = oldDocument->CreateElement(u8"div").ReleaseValue();
    auto node = oldDocument->CreateElement(u8"section").ReleaseValue();
    REQUIRE_FALSE(node->SetAttribute(u8"a", u8"1").HasException());
    auto child = oldDocument->CreateTextNode(u8"text");
    REQUIRE_FALSE(node->AppendChild(*child).HasException());
    auto shadow = node->AttachShadow({}).ReleaseValue();
    auto inShadow = oldDocument->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());
    REQUIRE_FALSE(parent->AppendChild(*node).HasException());

    REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*node, *document).HasException());

    // 2. Removed from its parent.
    CHECK(node->ParentNode() == nullptr);
    CHECK_FALSE(parent->HasChildNodes());

    // 3a. and 3c.i. Everything moves.
    auto attribute = node->GetAttributeNode(u8"a");
    List<Node *> moved {node.get(), child.get(), shadow.get(), inShadow.get(), attribute};
    for (auto *movedNode : moved)
    {
      CHECK(&movedNode->NodeDocument() == document.get());
    }
    CHECK(&parent->NodeDocument() == oldDocument.get());

    // Into the same document, it's only removed.
    REQUIRE_FALSE(parent->AppendChild(*oldDocument->CreateTextNode(u8"t")).HasException());
    auto sameDocument = ShareRef(*parent->FirstChild());
    REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*sameDocument, *oldDocument).HasException());
    CHECK_FALSE(parent->HasChildNodes());
    CHECK(&sameDocument->NodeDocument() == oldDocument.get());
  }

  TEST_CASE("Adopt: elements and shadow roots without a scoped registry take the new document's global one",
            "[DOM][DocumentAlgorithms][dom:concept-node-adopt][dom:effective-global-custom-element-registry]"
            "[dom:is-a-global-custom-element-registry]")
  {
    auto oldDocument = CreateRef<HTMLDocument>();
    oldDocument->SetCustomElementRegistry(html::CustomElementRegistry::Create(*oldDocument));
    auto document = CreateRef<HTMLDocument>();
    auto global = html::CustomElementRegistry::Create(*document);
    document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(global));
    auto scoped = html::CustomElementRegistry::CreateScoped();

    SECTION("3c.ii.B. a global registry, or no parent")
    {
      auto element = oldDocument->CreateElement(u8"div").ReleaseValue();
      auto withoutRegistry = CreateRef<TestElement>(*oldDocument);
      REQUIRE(withoutRegistry->CustomElementRegistry() == nullptr);
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*element, *document).HasException());
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*withoutRegistry, *document).HasException());
      CHECK(element->CustomElementRegistry().get() == global.get());
      CHECK(withoutRegistry->CustomElementRegistry().get() == global.get());
    }

    SECTION("3c.ii. a scoped registry is kept")
    {
      auto element =
        oldDocument->CreateElement(u8"div", ElementCreationOptions {.CustomElementRegistry = scoped})
          .ReleaseValue();
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*element, *document).HasException());
      CHECK(element->CustomElementRegistry().get() == scoped.get());
    }

    SECTION("3c.ii.C. without a registry, under a parent with a scoped one, it stays without")
    {
      auto parent =
        oldDocument->CreateElement(u8"div", ElementCreationOptions {.CustomElementRegistry = scoped})
          .ReleaseValue();
      auto child = CreateRef<TestElement>(*oldDocument);
      REQUIRE_FALSE(parent->AppendChild(*child).HasException());
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*parent, *document).HasException());
      CHECK(child->CustomElementRegistry() == nullptr);
    }

    SECTION("3c.ii.B. without a registry, in a document fragment")
    {
      auto fragment = oldDocument->CreateDocumentFragment();
      auto child = CreateRef<TestElement>(*oldDocument);
      REQUIRE_FALSE(fragment->AppendChild(*child).HasException());
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*fragment, *document).HasException());
      CHECK(child->CustomElementRegistry().get() == global.get());
    }

    SECTION("3b. a shadow root with a global registry, or none it needn't keep")
    {
      auto host = oldDocument->CreateElement(u8"div").ReleaseValue();
      auto shadow = host->AttachShadow({}).ReleaseValue();
      REQUIRE(
        html::CustomElementAlgorithms::IsGlobalCustomElementRegistry(shadow->CustomElementRegistry().get()));
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*host, *document).HasException());
      CHECK(shadow->CustomElementRegistry().get() == global.get());

      auto scopedHost = oldDocument->CreateElement(u8"div").ReleaseValue();
      auto scopedShadow = scopedHost->AttachShadow({.CustomElementRegistry = scoped.get()}).ReleaseValue();
      REQUIRE_FALSE(DocumentAlgorithms::AdoptNode(*scopedHost, *document).HasException());
      CHECK(scopedShadow->CustomElementRegistry().get() == scoped.get());
    }

    SECTION("effective global custom element registry: a global registry, else null")
    {
      CHECK(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(global.get())
            == global.get());
      CHECK(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(scoped.get()) == nullptr);
      CHECK(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(
              static_cast<html::CustomElementRegistry *>(nullptr))
            == nullptr);
      CHECK(html::CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(*document) == global.get());
      CHECK_FALSE(html::CustomElementAlgorithms::IsGlobalCustomElementRegistry(scoped.get()));
      CHECK_FALSE(html::CustomElementAlgorithms::IsGlobalCustomElementRegistry(nullptr));
    }
  }

  TEST_CASE("Update the current document readiness: sets it and fires readystatechange, once per change",
            "[DOM][DocumentAlgorithms][html:update-the-current-document-readiness]")
  {
    auto document = CreateRef<HTMLDocument>();
    List<html::DocumentReadyState> seen;
    auto listener = document->AddEventListener(u8"readystatechange",
                                               [&](Event &) { seen.push_back(document->ReadyState()); });

    DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Complete);
    CHECK(seen.empty());

    DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Loading);
    DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Interactive);
    DocumentAlgorithms::UpdateCurrentDocumentReadiness(*document, html::DocumentReadyState::Interactive);
    CHECK(seen
          == List<html::DocumentReadyState> {html::DocumentReadyState::Loading,
                                             html::DocumentReadyState::Interactive});

    // Not parsed in a frame, so no load timing is recorded.
    CHECK(document->LoadTiming().DOMInteractiveTime == 0);
  }
}
