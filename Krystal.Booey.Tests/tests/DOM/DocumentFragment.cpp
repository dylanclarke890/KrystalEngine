#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "catch2/catch_test_macros.hpp"

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

  TEST_CASE("DocumentFragment::NodeType", "[HTML][DocumentFragment]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeType() == NodeType::DOCUMENT_FRAGMENT_NODE);
  }

  TEST_CASE("DocumentFragment::NodeName", "[HTML][DocumentFragment]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeName() == u8"#document-fragment");
  }

  TEST_CASE("DocumentFragment::TextContent", "[HTML][DocumentFragment]")
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