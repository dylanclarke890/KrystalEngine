#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
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
    Ref<TestNode> Node;

    CommonTestData(bool appendChild = false)
        : Document(CreateRef<HTML::Document>()), Node(CreateRef<TestNode>(*Document))
    {
      if (appendChild)
      {
        auto result = Document->AppendChild(*Node);
        REQUIRE_FALSE(result.HasException());
      }
    }

    ~CommonTestData()
    {
      if (Node->IsConnected())
      {
        auto result = Document->RemoveChild(*Node);
        REQUIRE_FALSE(result.HasException());
      }
    }
  };

  TEST_CASE("Node::NodeType", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->NodeType() == NodeType::ELEMENT_NODE);
  }

  TEST_CASE("Node::BaseURI", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->BaseURI() == u8"about:blank");
  }

  TEST_CASE("Node::IsConnected", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE_FALSE(data.Node->IsConnected());
    REQUIRE(data.Document->IsConnected());

    auto appendResult = data.Document->AppendChild(*data.Node);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->IsConnected());
    REQUIRE(data.Document->IsConnected());
  }

  TEST_CASE("Node::OwnerDocument", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->OwnerDocument() == data.Document.get());
    REQUIRE(data.Document->OwnerDocument() == nullptr);
  }

  TEST_CASE("Node::GetRootNode", "[HTML][Node]")
  {
    CommonTestData data {};

    REQUIRE(&data.Node->GetRootNode({.Composed = false}) == data.Node.get());
    REQUIRE(&data.Node->GetRootNode({.Composed = true}) == data.Node.get());

    auto appendResult = data.Document->AppendChild(*data.Node);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&data.Node->GetRootNode({.Composed = false}) == data.Document.get());
    REQUIRE(&data.Node->GetRootNode({.Composed = true}) == data.Document.get());
  }

  TEST_CASE("Node::ParentNode", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->ParentNode() == nullptr);

    auto appendResult = data.Document->AppendChild(*data.Node);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.Node->ParentNode() == data.Document.get());

    auto div = CreateRef<TestNode>(*data.Document);
    auto appendDivResult = data.Node->AppendChild(*div);
    REQUIRE_FALSE(appendDivResult.HasException());
    REQUIRE(div->ParentNode() == data.Node.get());

    data.Node->RemoveChild(*div);
  }

  TEST_CASE("Node::ParentElement", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->ParentElement() == nullptr);

    auto appendResult = data.Document->AppendChild(*data.Node);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.Node->ParentElement() == nullptr);

    auto div = CreateRef<TestNode>(*data.Document);
    auto appendDivResult = data.Node->AppendChild(*div);
    REQUIRE_FALSE(appendDivResult.HasException());
    REQUIRE(div->ParentElement() == data.Node.get());

    auto removeResult = data.Node->RemoveChild(*div);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::HasChildNodes", "[HTML][Node]")
  {
    CommonTestData data {true};
    REQUIRE_FALSE(data.Node->HasChildNodes());
    REQUIRE(data.Document->HasChildNodes());

    auto child = CreateRef<TestNode>(*data.Document);
    auto appendResult = data.Node->AppendChild(*child);

    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(data.Node->HasChildNodes());
    REQUIRE(data.Document->HasChildNodes());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::ChildNodes", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto childA = CreateRef<TestNode>(*data.Document);
    auto childB = CreateRef<TestNode>(*data.Document);
    auto childC = CreateRef<TestNode>(*data.Document);

    Ref<NodeList> childNodes = data.Node->ChildNodes();

    REQUIRE(childNodes->Length() == 0uz);
    REQUIRE(childNodes->Item(0uz) == nullptr);
    REQUIRE(childNodes->Item(1uz) == nullptr);

    auto result = data.Node->AppendChild(*childA);
    REQUIRE_FALSE(result.HasException());

    REQUIRE(childNodes->Length() == 1uz);
    REQUIRE(childNodes->Item(0uz) == childA.get());
    REQUIRE(childNodes->Item(1uz) == nullptr);

    result = data.Node->AppendChild(*childB);
    REQUIRE_FALSE(result.HasException());

    REQUIRE(childNodes->Length() == 2uz);
    REQUIRE(childNodes->Item(0uz) == childA.get());
    REQUIRE(childNodes->Item(1uz) == childB.get());

    result = data.Node->InsertBefore(*childC, childA.get());
    REQUIRE_FALSE(result.HasException());

    REQUIRE(childNodes->Length() == 3uz);
    REQUIRE(childNodes->Item(0uz) == childC.get());
    REQUIRE(childNodes->Item(1uz) == childA.get());
    REQUIRE(childNodes->Item(2uz) == childB.get());

    data.Node->RemoveChild(*childA);
    data.Node->RemoveChild(*childB);
    data.Node->RemoveChild(*childC);
  }

  TEST_CASE("Node::FirstChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.Document);

    REQUIRE(data.Node->FirstChild() == nullptr);

    auto appendResult = data.Node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->FirstChild() == child.get());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    REQUIRE(data.Node->FirstChild() == nullptr);
  }

  TEST_CASE("Node::LastChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.Document);
    REQUIRE(data.Node->LastChild() == nullptr);

    auto appendResult = data.Node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->LastChild() == child.get());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    REQUIRE(data.Node->LastChild() == nullptr);
  }

  TEST_CASE("Node::NextSibling/PreviousSibling", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto sibling1 = CreateRef<TestNode>(*data.Document);
    auto sibling2 = CreateRef<TestNode>(*data.Document);

    REQUIRE(sibling1->NextSibling() == nullptr);
    REQUIRE(sibling1->PreviousSibling() == nullptr);

    REQUIRE(sibling2->NextSibling() == nullptr);
    REQUIRE(sibling2->PreviousSibling() == nullptr);

    auto appendResult = data.Node->AppendChild(*sibling1);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = data.Node->AppendChild(*sibling2);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(sibling2->PreviousSibling() == sibling1.get());

    auto removeResult = data.Node->RemoveChild(*sibling1);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = data.Node->RemoveChild(*sibling2);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::NodeValue", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->NodeValue() == Null);

    auto setValueResult = data.Node->NodeValue(u8"test value");
    REQUIRE_FALSE(setValueResult.HasException());

    REQUIRE(data.Node->NodeValue() == Null);
  }

  // NOTE: TextContent is tested in the tests for the derived classes that override it, so we don't need to
  // test it here.

  TEST_CASE("Node::Normalize(no children)", "[HTML][Node]")
  {
    CommonTestData data {};
    auto normalizeResult = data.Node->Normalize();
    REQUIRE_FALSE(normalizeResult.HasException());
  }

  TEST_CASE("Node::Normalize(With only an empty text node)", "[HTML][Node]")
  {
    CommonTestData data {};

    auto emptyTextNode = CreateRef<HTML::Text>(*data.Document);
    auto appendResult = data.Node->AppendChild(*emptyTextNode);
    REQUIRE_FALSE(appendResult.HasException());

    auto normalizeResult = data.Node->Normalize();
    REQUIRE_FALSE(normalizeResult.HasException());
    REQUIRE(data.Node->FirstChild() == nullptr); // was removed by normalization since it was empty.
  }

  TEST_CASE("Node::Normalize(With children)", "[HTML][Node]")
  {
    CommonTestData data {};

    auto textNode1 = CreateRef<HTML::Text>(*data.Document);
    auto textNode2 = CreateRef<HTML::Text>(*data.Document);

    auto appendResult = data.Node->AppendChild(*textNode1);
    REQUIRE_FALSE(appendResult.HasException());

    appendResult = data.Node->AppendChild(*textNode2);
    REQUIRE_FALSE(appendResult.HasException());

    auto setValueResult = textNode1->NodeValue(u8"test");
    REQUIRE_FALSE(setValueResult.HasException());

    setValueResult = textNode2->NodeValue(u8" value");
    REQUIRE_FALSE(setValueResult.HasException());

    auto normalizeResult = data.Node->Normalize();
    REQUIRE_FALSE(normalizeResult.HasException());

    REQUIRE(data.Node->FirstChild() == textNode1.get());
    REQUIRE(data.Node->LastChild() == textNode1.get());
    REQUIRE(data.Node->FirstChild()->NodeValue() == u8"test value");

    auto removeResult = data.Node->RemoveChild(*textNode1);
    REQUIRE_FALSE(removeResult.HasException());
  }

  // TODO(test):
  // TEST_CASE("Node::CloneNode", "[HTML][Node]")

  TEST_CASE("Node::IsEqualNode", "[HTML][Node]")
  {
    CommonTestData data {};
    auto equalNode = CreateRef<TestNode>(*data.Document);

    REQUIRE_FALSE(data.Node->IsEqualNode(nullptr));
    REQUIRE(data.Node->IsEqualNode(data.Node.get()));
    REQUIRE(data.Node->IsEqualNode(equalNode.get()));

    // TODO(test): other test cases for equality.
  }

  TEST_CASE("Node::IsSameNode", "[HTML][Node]")
  {
    CommonTestData data {};

    REQUIRE_FALSE(data.Node->IsSameNode(nullptr));
    REQUIRE(data.Node->IsSameNode(data.Node.get()));
    REQUIRE_FALSE(data.Node->IsSameNode(data.Document.get()));
  }

  TEST_CASE("Node::CompareDocumentPosition(same node returns equivalent)", "[HTML][Node]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->CompareDocumentPosition(*data.Node) == DocumentPosition::DOCUMENT_POSITION_EQUIVALENT);
  }

  TEST_CASE("Node::CompareDocumentPosition(disconnected node)", "[HTML][Node]")
  {
    CommonTestData data {true};

    auto disconnectedNode = CreateRef<TestNode>(*data.Document);
    auto position = data.Node->CompareDocumentPosition(*disconnectedNode);

    REQUIRE(HasFlag(position, DocumentPosition::DOCUMENT_POSITION_DISCONNECTED));
    REQUIRE(HasFlag(position, DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC));

    auto nodePrecedesDisconnected = HasFlag(position, DocumentPosition::DOCUMENT_POSITION_PRECEDING);
    // Should be either preceding or following, but not both.
    REQUIRE(nodePrecedesDisconnected ^ HasFlag(position, DocumentPosition::DOCUMENT_POSITION_FOLLOWING));

    auto reversePosition = disconnectedNode->CompareDocumentPosition(*data.Node);

    REQUIRE(HasFlag(reversePosition, DocumentPosition::DOCUMENT_POSITION_DISCONNECTED));
    REQUIRE(HasFlag(reversePosition, DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC));

    auto reverseNodePrecedesDisconnected =
      HasFlag(reversePosition, DocumentPosition::DOCUMENT_POSITION_PRECEDING);
    // Should be either preceding or following, but not both.
    REQUIRE(reverseNodePrecedesDisconnected
            ^ HasFlag(reversePosition, DocumentPosition::DOCUMENT_POSITION_FOLLOWING));

    REQUIRE(reverseNodePrecedesDisconnected
            != nodePrecedesDisconnected); // if it was following one way, it should be preceding the other.
  }

  TEST_CASE("Node::CompareDocumentPosition(nodes in same document)")
  {
    CommonTestData data {true};
    auto sibling1 = CreateRef<TestNode>(*data.Document);
    auto sibling2 = CreateRef<TestNode>(*data.Document);

    auto appendResult = data.Node->AppendChild(*sibling1);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = data.Node->AppendChild(*sibling2);
    REQUIRE_FALSE(appendResult.HasException());

    auto nodePositionToSibling1 = data.Node->CompareDocumentPosition(*sibling1);
    REQUIRE(HasFlag(nodePositionToSibling1, DocumentPosition::DOCUMENT_POSITION_CONTAINED_BY));
    REQUIRE(HasFlag(nodePositionToSibling1, DocumentPosition::DOCUMENT_POSITION_FOLLOWING));

    auto sibling1PositionToNode = sibling1->CompareDocumentPosition(*data.Node);
    REQUIRE(HasFlag(sibling1PositionToNode, DocumentPosition::DOCUMENT_POSITION_CONTAINS));
    REQUIRE(HasFlag(sibling1PositionToNode, DocumentPosition::DOCUMENT_POSITION_PRECEDING));

    auto nodePositionToSibling2 = data.Node->CompareDocumentPosition(*sibling2);
    REQUIRE(HasFlag(nodePositionToSibling2, DocumentPosition::DOCUMENT_POSITION_CONTAINED_BY));
    REQUIRE(HasFlag(nodePositionToSibling2, DocumentPosition::DOCUMENT_POSITION_FOLLOWING));

    auto sibling2PositionToNode = sibling2->CompareDocumentPosition(*data.Node);
    REQUIRE(HasFlag(sibling2PositionToNode, DocumentPosition::DOCUMENT_POSITION_CONTAINS));
    REQUIRE(HasFlag(sibling2PositionToNode, DocumentPosition::DOCUMENT_POSITION_PRECEDING));

    auto sibling1PositionToSibling2 = sibling1->CompareDocumentPosition(*sibling2);
    REQUIRE(HasFlag(sibling1PositionToSibling2, DocumentPosition::DOCUMENT_POSITION_FOLLOWING));

    auto sibling2PositionToSibling1 = sibling2->CompareDocumentPosition(*sibling1);
    REQUIRE(HasFlag(sibling2PositionToSibling1, DocumentPosition::DOCUMENT_POSITION_PRECEDING));

    auto removeResult = data.Node->RemoveChild(*sibling1);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = data.Node->RemoveChild(*sibling2);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::Contains", "[HTML][Node]")
  {
    CommonTestData data {};
    auto grandchild = CreateRef<TestNode>(*data.Document);

    REQUIRE_FALSE(data.Node->Contains(nullptr));
    REQUIRE_FALSE(data.Node->Contains(data.Document.get()));
    REQUIRE_FALSE(data.Document->Contains(data.Node.get()));
    REQUIRE_FALSE(data.Node->Contains(grandchild.get()));
    REQUIRE_FALSE(data.Document->Contains(grandchild.get()));

    auto appendResult = data.Document->AppendChild(*data.Node);
    REQUIRE_FALSE(appendResult.HasException());
    appendResult = data.Node->AppendChild(*grandchild);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Document->Contains(data.Node.get()));
    REQUIRE(data.Node->Contains(grandchild.get()));
    REQUIRE(data.Document->Contains(grandchild.get()));
    REQUIRE_FALSE(grandchild->Contains(data.Node.get()));
    REQUIRE_FALSE(grandchild->Contains(data.Document.get()));
    REQUIRE_FALSE(data.Node->Contains(data.Document.get()));

    auto removeResult = data.Node->RemoveChild(*grandchild);
    REQUIRE_FALSE(removeResult.HasException());
  }

  // TODO(test):
  // TEST_CASE("Node::LookupPrefix", "[HTML][Node]")

  // TODO(test):
  // TEST_CASE("Node::LookupNamespaceURI", "[HTML][Node]")
  // TEST_CASE("Node::LookupNamespaceURI(simple prefix lookup)", "[HTML][Node]")
  //{
  //  CommonTestData data {};

  // auto root = CreateRef<TestNode>(*data.Document);
  // auto child = CreateRef<TestNode>(*data.Document);

  // REQUIRE_FALSE(data.Document->AppendChild(*root).HasException());
  // REQUIRE_FALSE(root->AppendChild(*child).HasException());

  // REQUIRE_FALSE(root->SetAttributeNS(Namespaces::XMLNS, u8"xmlns:foo", u8"http://a").HasException());

  // REQUIRE(child->LookupNamespaceURI(u8"foo") == u8"http://a");
  // }

  // TODO(test):
  // TEST_CASE("Node::IsDefaultNamespace", "[HTML][Node]")

  TEST_CASE("Node::InsertBefore", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.Document);

    REQUIRE(data.Node->FirstChild() == nullptr);
    REQUIRE(data.Node->LastChild() == nullptr);

    auto insertResult = data.Node->InsertBefore(*child, nullptr);
    REQUIRE_FALSE(insertResult.HasException());

    REQUIRE(&insertResult.Value() == child.get());
    REQUIRE(data.Node->FirstChild() == child.get());
    REQUIRE(data.Node->LastChild() == child.get());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::ReplaceChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child1 = CreateRef<TestNode>(*data.Document);
    auto child2 = CreateRef<TestNode>(*data.Document);

    auto appendResult = data.Node->AppendChild(*child1);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->FirstChild() == child1.get());
    REQUIRE(data.Node->LastChild() == child1.get());

    auto replaceResult = data.Node->ReplaceChild(*child2, *child1);
    REQUIRE_FALSE(replaceResult.HasException());

    REQUIRE(&replaceResult.Value() == child1.get());
    REQUIRE(data.Node->FirstChild() == child2.get());
    REQUIRE(data.Node->LastChild() == child2.get());

    auto removeResult = data.Node->RemoveChild(*child2);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::RemoveChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.Document);

    auto appendResult = data.Node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->FirstChild() == child.get());
    REQUIRE(data.Node->LastChild() == child.get());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());

    REQUIRE(data.Node->FirstChild() == nullptr);
    REQUIRE(data.Node->LastChild() == nullptr);
  }

  TEST_CASE("Node::AppendChild", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.Document);

    REQUIRE(data.Node->FirstChild() == nullptr);
    REQUIRE(data.Node->LastChild() == nullptr);

    auto appendResult = data.Node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&appendResult.Value() == child.get());
    REQUIRE(data.Node->FirstChild() == child.get());
    REQUIRE(data.Node->LastChild() == child.get());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }

  TEST_CASE("Node::CountChildNodes", "[HTML][Node]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestNode>(*data.Document);

    REQUIRE(data.Node->CountChildNodes() == 0uz);
    REQUIRE(data.Document->CountChildNodes() == 1uz);

    auto appendResult = data.Node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(data.Node->CountChildNodes() == 1uz);
    REQUIRE(data.Document->CountChildNodes() == 1uz);

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
  }
}
