#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
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

    class NamespaceTestElement : public Element
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(NamespaceTestElement);

    public:
      NamespaceTestElement(Document &document, DOMStringAtom namespacePrefix) noexcept
          : Element(document, {u8"http://a", namespacePrefix, u8"test-element"}, NodeFlag::None)
      {
      }
    };
  }

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

  TEST_CASE("Node::Normalize", "[HTML][Node]")
  {
    CommonTestData data {};

    SECTION("no children")
    {
      auto normalizeResult = data.Node->Normalize();
      REQUIRE_FALSE(normalizeResult.HasException());
    }

    SECTION("With only an empty text node")
    {
      auto emptyTextNode = CreateRef<HTML::Text>(*data.Document);
      auto appendResult = data.Node->AppendChild(*emptyTextNode);
      REQUIRE_FALSE(appendResult.HasException());

      auto normalizeResult = data.Node->Normalize();
      REQUIRE_FALSE(normalizeResult.HasException());
      REQUIRE(data.Node->FirstChild() == nullptr); // was removed by normalization since it was empty.
    }

    SECTION("with children")
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
  }

  TEST_CASE("Node::CloneNode", "[HTML][Node]")
  {
    auto document = CreateRef<Document>();

    SECTION("Element")
    {
      // TODO(test): cloning an element
    }

    SECTION("Document")
    {
      // TODO(test): cloning a document
    }

    SECTION("DocumentType")
    {
      auto doctype = CreateRef<DocumentType>(*document, u8"html", u8"public-id", u8"system-id");

      auto clone = doctype->CloneNode();
      REQUIRE(clone.HasValue());

      auto &doctypeClone = Downcast<DocumentType>(*clone.Value());
      REQUIRE(doctypeClone.Name() == u8"html");
      REQUIRE(doctypeClone.PublicId() == u8"public-id");
      REQUIRE(doctypeClone.SystemId() == u8"system-id");
    }

    SECTION("Attr")
    {
      auto attribute = document->CreateAttributeNS(Namespaces::XML, u8"xml:name");
      REQUIRE_FALSE(attribute.HasException());
      attribute->Value(u8"value");

      auto clone = attribute->CloneNode();

      auto &attributeClone = Downcast<Attr>(*clone.Value());
      REQUIRE(attributeClone.NamespaceURI() == Namespaces::XML);
      REQUIRE(attributeClone.LocalName() == u8"name");
      REQUIRE(attributeClone.Prefix() == u8"xml");
      REQUIRE(attributeClone.Value() == u8"value");
    }

    SECTION("Text")
    {
      auto textNode = CreateRef<HTML::Text>(*document, u8"text");

      auto clone = textNode->CloneNode();
      REQUIRE(clone.HasValue());

      auto &textNodeClone = Downcast<HTML::Text>(*clone.Value());
      REQUIRE(textNodeClone.Data() == u8"text");
    }

    SECTION("Comment")
    {
      auto comment = CreateRef<HTML::Comment>(*document, u8"text");

      auto clone = comment->CloneNode();
      REQUIRE(clone.HasValue());

      auto &commentClone = Downcast<HTML::Comment>(*clone.Value());
      REQUIRE(commentClone.Data() == u8"text");
    }

    SECTION("ProcessingInstruction")
    {
      auto processingInstruction = CreateRef<HTML::ProcessingInstruction>(*document, u8"target", u8"data");

      auto clone = processingInstruction->CloneNode();
      REQUIRE(clone.HasValue());

      REQUIRE(clone->NodeType() == NodeType::PROCESSING_INSTRUCTION_NODE);
      auto &processingInstructionClone = Downcast<HTML::ProcessingInstruction>(*clone.Value());

      REQUIRE(processingInstructionClone.Target() == u8"target");
      REQUIRE(processingInstructionClone.Data() == u8"data");
    }
  }

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

  TEST_CASE("Node::CompareDocumentPosition", "[HTML][Node]")
  {
    SECTION("same node returns equivalent")
    {
      CommonTestData data {};
      REQUIRE(data.Node->CompareDocumentPosition(*data.Node)
              == DocumentPosition::DOCUMENT_POSITION_EQUIVALENT);
    }

    SECTION("disconnected node")
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

    SECTION("nodes in same document")
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

    // TODO(test): comparing attribute nodes, both with each other and with other node types.
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

  TEST_CASE("Node::LookupPrefix", "[HTML][Node]")
  {
    CommonTestData data {};

    SECTION("node is an Element")
    {
      SECTION("returns its namespace prefix if it has one")
      {
        auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE(element->LookupPrefix(u8"http://a") == u8"testprefix");
      }

      SECTION("returns null if it doesn't have a namespace prefix")
      {
        auto element = CreateRef<NamespaceTestElement>(*data.Document, DOMStringAtom::Null());
        REQUIRE(element->LookupPrefix(u8"http://a") == DOMStringAtom::Null());
      }

      SECTION("returns null if its namespace doesn't match")
      {
        auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE(element->LookupPrefix(u8"http://b") == DOMStringAtom::Null());
      }
    }

    SECTION("Node is a Document")
    {
      SECTION("returns null if its documentElement is null")
      {
        REQUIRE(data.Document->LookupPrefix(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns its documentElement's namespace prefix if its documentElement is not null")
      {
        auto child = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE_FALSE(data.Document->AppendChild(*child).HasException());
        REQUIRE(data.Document->LookupPrefix(u8"http://a") == u8"testprefix");
        REQUIRE_FALSE(data.Document->RemoveChild(*child).HasException());
      }
    }

    SECTION("returns null if Node is a DocumentFragment")
    {
      auto fragment = CreateRef<DocumentFragment>(*data.Document);
      REQUIRE(fragment->LookupPrefix(u8"foo") == DOMStringAtom::Null());
    }

    SECTION("returns null if Node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"test", u8"", u8"");
      REQUIRE(doctype->LookupPrefix(u8"foo") == DOMStringAtom::Null());
    }

    SECTION("Node is an Attr")
    {
      SECTION("returns null if it doesn't have an element owner")
      {
        auto attr = data.Document->CreateAttribute(u8"test");
        REQUIRE_FALSE(attr.HasException());
        REQUIRE(attr->LookupPrefix(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns its element owner's namespace prefix if it has an element owner")
      {
        auto attr = data.Document->CreateAttribute(u8"test");
        REQUIRE_FALSE(attr.HasException());
        auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE_FALSE(element->SetAttributeNode(*attr.Value()).HasException());
        REQUIRE(attr->LookupPrefix(u8"http://a") == u8"testprefix");
      }
    }

    SECTION("Node that is none of the above returns it's parent element's namespace prefix")
    {
      auto textNode = CreateRef<HTML::Text>(*data.Document, u8"");

      SECTION("returns null if parent element is null")
      {
        REQUIRE(textNode->LookupPrefix(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns parent element's namespace prefix if parent element is not null")
      {
        auto parentElement = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE_FALSE(parentElement->AppendChild(*textNode).HasException());
        REQUIRE(textNode->LookupPrefix(u8"http://a") == u8"testprefix");
        REQUIRE_FALSE(parentElement->RemoveChild(*textNode).HasException());
      }
    }
  }

  TEST_CASE("Node::LookupNamespaceURI", "[HTML][Node]")
  {
    CommonTestData data {};

    SECTION("node is an Element")
    {
      auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");

      SECTION("returns XML namespace if prefix is 'xml'")
      {
        REQUIRE(element->LookupNamespaceURI(u8"xml") == Namespaces::XML);
      }

      SECTION("returns XMLNS namespace if prefix is 'xmlns'")
      {
        REQUIRE(element->LookupNamespaceURI(u8"xmlns") == Namespaces::XMLNS);
      }

      SECTION("returns it's namespace when non null and it's prefix matches")
      {
        REQUIRE(element->LookupNamespaceURI(u8"testprefix") == u8"http://a");
      }

      // TODO(test): If it has an attribute whose namespace is the XMLNS namespace, namespace prefix is
      // "xmlns", and local name is prefix, or if prefix is null and it has an attribute whose namespace is
      // the XMLNS namespace, namespace prefix is null, and local name is "xmlns", then return its value if it
      // is not the empty string, and null otherwise.

      SECTION("returns null if prefix doesn't match and parent is null")
      {
        REQUIRE(element->LookupNamespaceURI(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns it's parent's namespace if prefix doesn't match and parent is not null")
      {
        auto parentElement = CreateRef<NamespaceTestElement>(*data.Document, u8"foo");
        REQUIRE_FALSE(parentElement->AppendChild(*element).HasException());

        REQUIRE(element->LookupNamespaceURI(u8"foo") == u8"http://a");

        REQUIRE_FALSE(parentElement->RemoveChild(*element).HasException());
      }
    }

    SECTION("Node is a Document")
    {
      SECTION("returns null if it's documentElement is null")
      {
        REQUIRE(data.Document->LookupNamespaceURI(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns it's documentElement's namespace if it's documentElement is not null")
      {
        auto child = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE_FALSE(data.Document->AppendChild(*child).HasException());

        REQUIRE(data.Document->LookupNamespaceURI(u8"testprefix") == u8"http://a");

        REQUIRE_FALSE(data.Document->RemoveChild(*child).HasException());
      }
    }

    SECTION("returns null if Node is a DocumentFragment")
    {
      auto fragment = CreateRef<DocumentFragment>(*data.Document);
      REQUIRE(fragment->LookupNamespaceURI(u8"foo") == DOMStringAtom::Null());
    }

    SECTION("returns null if Node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"test", u8"", u8"");
      REQUIRE(doctype->LookupNamespaceURI(u8"foo") == DOMStringAtom::Null());
    }

    SECTION("Node is an Attr")
    {
      SECTION("returns null if it doesn't have an element owner")
      {
        auto attr = data.Document->CreateAttribute(u8"test");
        REQUIRE_FALSE(attr.HasException());

        REQUIRE(attr->LookupNamespaceURI(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns it's element owner's namespace if it has an element owner")
      {
        auto attr = data.Document->CreateAttribute(u8"test");
        REQUIRE_FALSE(attr.HasException());

        auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE_FALSE(element->SetAttributeNode(*attr.Value()).HasException());

        REQUIRE(attr->LookupNamespaceURI(u8"testprefix") == u8"http://a");
      }
    }

    SECTION("Node that is none of the above returns it's parent element's namespace")
    {
      auto textNode = CreateRef<HTML::Text>(*data.Document, u8"");

      SECTION("returns null if parent element is null")
      {
        REQUIRE(textNode->LookupNamespaceURI(u8"foo") == DOMStringAtom::Null());
      }

      SECTION("returns parent element's namespace if parent element is not null")
      {
        auto parentElement = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE_FALSE(parentElement->AppendChild(*textNode).HasException());

        REQUIRE(textNode->LookupNamespaceURI(u8"testprefix") == u8"http://a");

        REQUIRE_FALSE(parentElement->RemoveChild(*textNode).HasException());
      }
    }
  }

  TEST_CASE("Node::IsDefaultNamespace", "[HTML][Node]")
  {
    CommonTestData data {};
    auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
    REQUIRE_FALSE(element->IsDefaultNamespace(u8"foo"));
    REQUIRE_FALSE(element->IsDefaultNamespace(u8"http://a"));
    REQUIRE(element->IsDefaultNamespace(DOMStringAtom::Null()));
  }

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
