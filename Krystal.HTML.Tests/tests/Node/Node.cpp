#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  struct CommonTestData
  {
    Ref<Document> test_document;
    Ref<TestNode> test_node;

    CommonTestData() : test_document(CreateRef<Document>()), test_node(CreateRef<TestNode>(*test_document))
    {
    }

    ~CommonTestData()
    {
      if (test_node->IsConnected())
      {
        auto result = test_document->RemoveChild(*test_node);
        REQUIRE_FALSE(result.HasException());
      }
    }
  };

  TEST_CASE("Node::GetNodeType", "[Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->GetNodeType() == NodeType::ELEMENT_NODE);
  }

  TEST_CASE("Node::BaseURI", "[Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->BaseURI().Href == u8"about:blank");
  }

  TEST_CASE("Node::IsConnected", "[Node]")
  {
    CommonTestData data {};
    REQUIRE_FALSE(data.test_node->IsConnected());

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.test_node->IsConnected());
  }

  TEST_CASE("Node::OwnerDocument", "[Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->OwnerDocument() == data.test_document.get());
    REQUIRE(data.test_document->OwnerDocument() == nullptr);
  }

  TEST_CASE("Node::GetRootNode", "[Node]")
  {
    CommonTestData data {};

    REQUIRE(&data.test_node->GetRootNode({.Composed = false}) == data.test_node.get());
    REQUIRE(&data.test_node->GetRootNode({.Composed = true}) == data.test_node.get());

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&data.test_node->GetRootNode({.Composed = false}) == data.test_document.get());
    REQUIRE(&data.test_node->GetRootNode({.Composed = true}) == data.test_document.get());
  }

  TEST_CASE("Node::ParentNode", "[Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->ParentNode() == nullptr);

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.test_node->ParentNode() == data.test_document.get());

    auto div = CreateRef<TestNode>(*data.test_document);
    auto appendDivResult = data.test_node->AppendChild(*div);
    REQUIRE_FALSE(appendDivResult.HasException());
    REQUIRE(div->ParentNode() == data.test_node.get());

    data.test_node->RemoveChild(*div);
  }

  TEST_CASE("Node::ParentElement", "[Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->ParentElement() == nullptr);

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.test_node->ParentElement() == nullptr);

    auto div = CreateRef<TestNode>(*data.test_document);
    auto appendDivResult = data.test_node->AppendChild(*div);
    REQUIRE_FALSE(appendDivResult.HasException());
    REQUIRE(div->ParentElement() == data.test_node.get());

    data.test_node->RemoveChild(*div);
  }

  TEST_CASE("Node::IsSameNode", "[Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->IsSameNode(data.test_node.get()));

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->IsSameNode(data.test_node.get()));
    REQUIRE_FALSE(data.test_node->IsSameNode(data.test_document.get()));
    REQUIRE_FALSE(data.test_node->IsSameNode(nullptr));
  }
}
