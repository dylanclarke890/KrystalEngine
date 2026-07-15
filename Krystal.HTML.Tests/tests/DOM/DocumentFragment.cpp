#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML.Tests/DOM/TestElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<DocumentFragment> Node;

      CommonTestData() : Document(CreateRef<HTML::Document>()), Node(CreateRef<DocumentFragment>(*Document))
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
    auto textNode1 = CreateRef<HTML::Text>(*data.Document, u8"Hello");
    auto textNode2 = CreateRef<HTML::Text>(*data.Document, u8" world!");

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