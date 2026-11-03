#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<dom::Document> Document;
      Ref<DocumentFragment> Node;

      CommonTestData() : Document(CreateRef<dom::Document>()), Node(CreateRef<DocumentFragment>(*Document))
      {
      }

      ~CommonTestData()
      {
        while (auto *firstChild = Node->FirstChild())
        {
          REQUIRE_FALSE(Node->RemoveChild(*firstChild).HasException());
        }
      }
    };
  }

  TEST_CASE("Create a document fragment: an empty fragment in the document, without a host",
            "[DOM][DocumentFragment][dom:interface-documentfragment][dom:create-a-document-fragment]"
            "[dom:dom-documentfragment-documentfragment][dom:concept-documentfragment-host]")
  {
    CommonTestData data;
    CHECK(&data.Node->NodeDocument() == data.Document.get());
    CHECK_FALSE(data.Node->HasChildNodes());
    CHECK(data.Node->Host() == nullptr);
    CHECK(&data.Document->CreateDocumentFragment()->NodeDocument() == data.Document.get());

    // A shadow root's host is its element, as is a template's contents'.
    auto host = CreateRef<TestElement>(*data.Document);
    auto shadow = host->AttachShadow({}).ReleaseValue();
    CHECK(shadow->Host() == host.get());
    auto templateElement =
      data.Document->CreateElementNS(infra::Namespaces::HTML, u8"template").ReleaseValue();
    CHECK(Downcast<html::HTMLTemplateElement>(*templateElement).TemplateContents().Host()
          == templateElement.get());
  }

  TEST_CASE("Node.nodeType: a DocumentFragment is a DOCUMENT_FRAGMENT_NODE",
            "[DOM][DocumentFragment][dom:dom-node-nodetype][dom:dom-node-document_fragment_node]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeType() == NodeType::DOCUMENT_FRAGMENT_NODE);
  }

  TEST_CASE("Node.nodeName: a DocumentFragment's is #document-fragment",
            "[DOM][DocumentFragment][dom:dom-node-nodename]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeName() == u8"#document-fragment");
  }

  TEST_CASE("Node.textContent: a DocumentFragment's is its descendant text, which setting it replaces",
            "[DOM][DocumentFragment][dom:dom-node-textcontent][dom:get-text-content][dom:set-text-content]["
            "dom:string-replace-all]")
  {
    CommonTestData data;
    auto textNode1 = CreateRef<dom::Text>(*data.Document, u8"Hello");
    auto textNode2 = CreateRef<dom::Text>(*data.Document, u8" world!");

    REQUIRE_FALSE(data.Node->AppendChild(*textNode1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*textNode2).HasException());

    REQUIRE(data.Node->TextContent() == u8"Hello world!");

    REQUIRE_FALSE(data.Node->TextContent(u8"New text content").HasException());
    REQUIRE(data.Node->TextContent() == u8"New text content");

    REQUIRE_FALSE(data.Node->TextContent(u8"").HasException());
    REQUIRE_FALSE(data.Node->HasChildNodes());
    REQUIRE(data.Node->TextContent() == u8"");
  }
}