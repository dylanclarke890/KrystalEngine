#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
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

    CommonTestData(bool appendChild = false)
        : test_document(CreateRef<Document>()), test_node(CreateRef<TestNode>(*test_document))
    {
      if (appendChild)
      {
        auto result = test_document->AppendChild(*test_node);
        REQUIRE_FALSE(result.HasException());
      }
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

  TEST_CASE("Node::NodeType", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->NodeType() == NodeType::ELEMENT_NODE);
  }

  TEST_CASE("Node::BaseURI", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->BaseURI().Href == u8"about:blank");
  }

  TEST_CASE("Node::IsConnected", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE_FALSE(data.test_node->IsConnected());
    REQUIRE(data.test_document->IsConnected());

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->IsConnected());
    REQUIRE(data.test_document->IsConnected());
  }

  TEST_CASE("Node::OwnerDocument", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->OwnerDocument() == data.test_document.get());
    REQUIRE(data.test_document->OwnerDocument() == nullptr);
  }

  TEST_CASE("Node::GetRootNode", "[HTML][Node]")
  {
    CommonTestData data {};

    REQUIRE(&data.test_node->GetRootNode({.Composed = false}) == data.test_node.get());
    REQUIRE(&data.test_node->GetRootNode({.Composed = true}) == data.test_node.get());

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&data.test_node->GetRootNode({.Composed = false}) == data.test_document.get());
    REQUIRE(&data.test_node->GetRootNode({.Composed = true}) == data.test_document.get());
  }

  TEST_CASE("Node::ParentNode", "[HTML][Node]")
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

  TEST_CASE("Node::ParentElement", "[HTML][Node]")
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

    auto removeResult = data.test_node->RemoveChild(*div);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::HasChildNodes", "[HTML][Node]")
  {
    CommonTestData data {true};
    REQUIRE_FALSE(data.test_node->HasChildNodes());
    REQUIRE(data.test_document->HasChildNodes());

    auto child = CreateRef<TestNode>(*data.test_document);
    auto appendResult = data.test_node->AppendChild(*child);

    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.test_node->HasChildNodes());
    REQUIRE(data.test_document->HasChildNodes());

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }

  // TODO(impl):
  // TEST_CASE("Node::ChildNodes", "[HTML][Node]")

  TEST_CASE("Node::FirstChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.test_document);

    REQUIRE(data.test_node->FirstChild() == nullptr);

    auto appendResult = data.test_node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->FirstChild() == child.get());

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    REQUIRE(data.test_node->FirstChild() == nullptr);
  }

  TEST_CASE("Node::LastChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.test_document);
    REQUIRE(data.test_node->LastChild() == nullptr);

    auto appendResult = data.test_node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->LastChild() == child.get());

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    REQUIRE(data.test_node->LastChild() == nullptr);
  }

  TEST_CASE("Node::NextSibling/PreviousSibling", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto sibling1 = CreateRef<TestNode>(*data.test_document);
    auto sibling2 = CreateRef<TestNode>(*data.test_document);

    REQUIRE(sibling1->NextSibling() == nullptr);
    REQUIRE(sibling1->PreviousSibling() == nullptr);

    REQUIRE(sibling2->NextSibling() == nullptr);
    REQUIRE(sibling2->PreviousSibling() == nullptr);

    auto appendResult = data.test_node->AppendChild(*sibling1);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = data.test_node->AppendChild(*sibling2);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(sibling2->PreviousSibling() == sibling1.get());

    auto removeResult = data.test_node->RemoveChild(*sibling1);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = data.test_node->RemoveChild(*sibling2);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::NodeValue", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.test_node->NodeValue() == Null);

    auto setValueResult = data.test_node->NodeValue(u8"test value");
    REQUIRE_FALSE(setValueResult.HasException());

    REQUIRE(data.test_node->NodeValue() == Null);
  }

  // NOTE: TextContent is tested in the tests for the derived classes that override it, so we don't need to
  // test it here.

  TEST_CASE("Node::Normalize(no children)", "[HTML][Node]")
  {
    CommonTestData data {};
    auto normalizeResult = data.test_node->Normalize();
    REQUIRE_FALSE(normalizeResult.HasException());
  }

  TEST_CASE("Node::Normalize(With only an empty text node)", "[HTML][Node]")
  {
    CommonTestData data {};

    auto emptyTextNode = CreateRef<HTML::Text>(*data.test_document);
    auto appendResult = data.test_node->AppendChild(*emptyTextNode);
    REQUIRE_FALSE(appendResult.HasException());

    auto normalizeResult = data.test_node->Normalize();
    REQUIRE_FALSE(normalizeResult.HasException());
    REQUIRE(data.test_node->FirstChild() == nullptr); // was removed by normalization since it was empty.
  }

  TEST_CASE("Node::Normalize(With children)", "[HTML][Node]")
  {
    CommonTestData data {};

    auto textNode1 = CreateRef<HTML::Text>(*data.test_document);
    auto textNode2 = CreateRef<HTML::Text>(*data.test_document);

    auto appendResult = data.test_node->AppendChild(*textNode1);
    REQUIRE_FALSE(appendResult.HasException());

    appendResult = data.test_node->AppendChild(*textNode2);
    REQUIRE_FALSE(appendResult.HasException());

    auto setValueResult = textNode1->NodeValue(u8"test");
    REQUIRE_FALSE(setValueResult.HasException());

    setValueResult = textNode2->NodeValue(u8" value");
    REQUIRE_FALSE(setValueResult.HasException());

    auto normalizeResult = data.test_node->Normalize();
    REQUIRE_FALSE(normalizeResult.HasException());

    REQUIRE(data.test_node->FirstChild() == textNode1.get());
    REQUIRE(data.test_node->LastChild() == textNode1.get());
    REQUIRE(data.test_node->FirstChild()->NodeValue() == u8"test value");

    auto removeResult = data.test_node->RemoveChild(*textNode1);
    REQUIRE_FALSE(removeResult.HasException());
  }

  // TODO(impl):
  // TEST_CASE("Node::CloneNode", "[HTML][Node]")

  TEST_CASE("Node::IsEqualNode", "[HTML][Node]")
  {
    CommonTestData data {};
    auto equalNode = CreateRef<TestNode>(*data.test_document);

    REQUIRE_FALSE(data.test_node->IsEqualNode(nullptr));
    REQUIRE(data.test_node->IsEqualNode(data.test_node.get()));
    REQUIRE(data.test_node->IsEqualNode(equalNode.get()));

    // TODO(impl): other test cases for equality.
  }

  TEST_CASE("Node::IsSameNode", "[HTML][Node]")
  {
    CommonTestData data {};

    REQUIRE_FALSE(data.test_node->IsSameNode(nullptr));
    REQUIRE(data.test_node->IsSameNode(data.test_node.get()));
    REQUIRE_FALSE(data.test_node->IsSameNode(data.test_document.get()));
  }

  // TODO(impl):
  // TEST_CASE("Node::CompareDocumentPosition", "[HTML][Node]")

  TEST_CASE("Node::Contains", "[HTML][Node]")
  {
    CommonTestData data {};
    auto grandchild = CreateRef<TestNode>(*data.test_document);

    REQUIRE_FALSE(data.test_node->Contains(nullptr));
    REQUIRE_FALSE(data.test_node->Contains(data.test_document.get()));
    REQUIRE_FALSE(data.test_document->Contains(data.test_node.get()));
    REQUIRE_FALSE(data.test_node->Contains(grandchild.get()));
    REQUIRE_FALSE(data.test_document->Contains(grandchild.get()));

    auto appendResult = data.test_document->AppendChild(*data.test_node);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = data.test_node->AppendChild(*grandchild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_document->Contains(data.test_node.get()));
    REQUIRE(data.test_node->Contains(grandchild.get()));
    REQUIRE(data.test_document->Contains(grandchild.get()));
    REQUIRE_FALSE(grandchild->Contains(data.test_node.get()));
    REQUIRE_FALSE(grandchild->Contains(data.test_document.get()));
    REQUIRE_FALSE(data.test_node->Contains(data.test_document.get()));

    auto removeResult = data.test_node->RemoveChild(*grandchild);
    REQUIRE_FALSE(removeResult.HasException());
  }

  // TODO(impl):
  // TEST_CASE("Node::LookupPrefix", "[HTML][Node]")

  // TODO(impl):
  // TEST_CASE("Node::LookupNamespaceURI", "[HTML][Node]")

  // TODO(impl):
  // TEST_CASE("Node::IsDefaultNamespace", "[HTML][Node]")

  TEST_CASE("Node::InsertBefore", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.test_document);

    REQUIRE(data.test_node->FirstChild() == nullptr);
    REQUIRE(data.test_node->LastChild() == nullptr);

    auto insertResult = data.test_node->InsertBefore(*child, nullptr);
    REQUIRE_FALSE(insertResult.HasException());

    REQUIRE(&insertResult.Value() == child.get());
    REQUIRE(data.test_node->FirstChild() == child.get());
    REQUIRE(data.test_node->LastChild() == child.get());

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::ReplaceChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child1 = CreateRef<TestNode>(*data.test_document);
    auto child2 = CreateRef<TestNode>(*data.test_document);

    auto appendResult = data.test_node->AppendChild(*child1);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->FirstChild() == child1.get());
    REQUIRE(data.test_node->LastChild() == child1.get());

    auto replaceResult = data.test_node->ReplaceChild(*child2, *child1);
    REQUIRE_FALSE(replaceResult.HasException());

    REQUIRE(&replaceResult.Value() == child1.get());
    REQUIRE(data.test_node->FirstChild() == child2.get());
    REQUIRE(data.test_node->LastChild() == child2.get());

    auto removeResult = data.test_node->RemoveChild(*child2);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::RemoveChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.test_document);

    auto appendResult = data.test_node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->FirstChild() == child.get());
    REQUIRE(data.test_node->LastChild() == child.get());

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    REQUIRE(data.test_node->FirstChild() == nullptr);
    REQUIRE(data.test_node->LastChild() == nullptr);
  }

  TEST_CASE("Node::AppendChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.test_document);

    REQUIRE(data.test_node->FirstChild() == nullptr);
    REQUIRE(data.test_node->LastChild() == nullptr);

    auto appendResult = data.test_node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&appendResult.Value() == child.get());
    REQUIRE(data.test_node->FirstChild() == child.get());
    REQUIRE(data.test_node->LastChild() == child.get());

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::CountChildNodes", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.test_document);

    REQUIRE(data.test_node->CountChildNodes() == 0uz);
    REQUIRE(data.test_document->CountChildNodes() == 1uz);

    auto appendResult = data.test_node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.test_node->CountChildNodes() == 1uz);
    REQUIRE(data.test_document->CountChildNodes() == 1uz);

    auto removeResult = data.test_node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }
}
