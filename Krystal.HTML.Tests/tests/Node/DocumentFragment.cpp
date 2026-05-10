#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

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

  // TODO(impl):
  // TEST_CASE("DocumentFragment::TextContent", "[HTML][DocumentFragment]")
  //{
  //}
}