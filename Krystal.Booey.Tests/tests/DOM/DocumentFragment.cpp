#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

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