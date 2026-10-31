#include "Krystal.Booey/DOM/Node.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Node;

      CommonTestData(bool appendChild = false)
          : Document(CreateRef<dom::Document>()), Node(CreateRef<TestElement>(*Document))
      {
        if (appendChild)
        {
          REQUIRE_FALSE(Document->AppendChild(*Node).HasException());
        }
      }

      ~CommonTestData()
      {
        REQUIRE_FALSE(Node->Remove().HasException());
      }
    };

    class NamespaceTestElement : public Element
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(NamespaceTestElement);

    public:
      NamespaceTestElement(Document &document, dom::DOMStringAtom namespacePrefix) noexcept
          : Element(document, {u8"http://a", namespacePrefix, u8"test-element"}, NodeFlags::None)
      {
      }
    };
  }

  TEST_CASE("Create a node: a new node belongs to the document it's created with",
            "[DOM][Node][dom:interface-node][dom:create-a-node][dom:concept-node-document]")
  {
    auto document = CreateRef<HTMLDocument>();
    CHECK(&document->NodeDocument() == document.get());
    CHECK(&document->CreateElement(u8"div").ReleaseValue()->NodeDocument() == document.get());
    CHECK(&document->CreateTextNode(u8"text")->NodeDocument() == document.get());
    CHECK(&document->CreateAttribute(u8"a").ReleaseValue()->NodeDocument() == document.get());
    CHECK(&document->CreateDocumentFragment()->NodeDocument() == document.get());
  }

  TEST_CASE(
    "Node.nodeType: each interface's constant",
    "[DOM][Node][dom:dom-node-nodetype][dom:dom-node-element_node][dom:dom-node-attribute_node]"
    "[dom:dom-node-text_node][dom:dom-node-cdata_section_node][dom:dom-node-processing_instruction_node]"
    "[dom:dom-node-comment_node][dom:dom-node-document_node][dom:dom-node-document_type_node]"
    "[dom:dom-node-document_fragment_node]")
  {
    auto document = CreateRef<XMLDocument>();
    CHECK(document->CreateElement(u8"a").ReleaseValue()->NodeType() == NodeType::ELEMENT_NODE);
    CHECK(document->CreateAttribute(u8"a").ReleaseValue()->NodeType() == NodeType::ATTRIBUTE_NODE);
    CHECK(document->CreateTextNode(u8"a")->NodeType() == NodeType::TEXT_NODE);
    CHECK(document->CreateCDATASection(u8"a").ReleaseValue()->NodeType() == NodeType::CDATA_SECTION_NODE);
    CHECK(document->CreateProcessingInstruction(u8"a", u8"b").ReleaseValue()->NodeType()
          == NodeType::PROCESSING_INSTRUCTION_NODE);
    CHECK(document->CreateComment(u8"a")->NodeType() == NodeType::COMMENT_NODE);
    CHECK(document->NodeType() == NodeType::DOCUMENT_NODE);
    CHECK(document->Implementation().CreateDocumentType(u8"a", u8"", u8"").ReleaseValue()->NodeType()
          == NodeType::DOCUMENT_TYPE_NODE);
    CHECK(document->CreateDocumentFragment()->NodeType() == NodeType::DOCUMENT_FRAGMENT_NODE);

    CHECK(std::to_underlying(NodeType::ELEMENT_NODE) == 1);
    CHECK(std::to_underlying(NodeType::ATTRIBUTE_NODE) == 2);
    CHECK(std::to_underlying(NodeType::TEXT_NODE) == 3);
    CHECK(std::to_underlying(NodeType::CDATA_SECTION_NODE) == 4);
    CHECK(std::to_underlying(NodeType::PROCESSING_INSTRUCTION_NODE) == 7);
    CHECK(std::to_underlying(NodeType::COMMENT_NODE) == 8);
    CHECK(std::to_underlying(NodeType::DOCUMENT_NODE) == 9);
    CHECK(std::to_underlying(NodeType::DOCUMENT_TYPE_NODE) == 10);
    CHECK(std::to_underlying(NodeType::DOCUMENT_FRAGMENT_NODE) == 11);
  }

  TEST_CASE("Node.nodeName: a Document's is #document", "[DOM][Node][dom:dom-node-nodename]")
  {
    CHECK(CreateRef<Document>()->NodeName() == u8"#document");
    CHECK(CreateRef<HTMLDocument>()->NodeName() == u8"#document");
  }

  TEST_CASE("Node.baseURI: the node document's base URL, serialised", "[DOM][Node][dom:dom-node-baseuri]")
  {
    SECTION("the document's URL without a base element")
    {
      auto document = CreateRef<Document>();
      auto element = CreateRef<TestElement>(*document);
      CHECK(element->BaseURI() == u8"about:blank");
      document->SetURL(url::URL(u8"https://example.test/a/b?c"));
      CHECK(element->BaseURI() == u8"https://example.test/a/b?c");
      CHECK(document->BaseURI() == u8"https://example.test/a/b?c");
    }

    SECTION("a base element's URL")
    {
      auto context = CreateRef<HTMLDocument>();
      auto document =
        Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html><base href='https://example.test/dir/'><p>")
          .ReleaseValue();
      CHECK(document->BaseURI() == u8"https://example.test/dir/");
    }
  }

  TEST_CASE("Connected: a node whose shadow-including root is a document",
            "[DOM][Node][dom:connected][dom:dom-node-isconnected]")
  {
    SECTION("A document and its descendants")
    {
      CommonTestData data {};

      REQUIRE(data.Document->IsConnected());
      REQUIRE_FALSE(data.Node->IsConnected());

      REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());

      REQUIRE(data.Document->IsConnected());
      REQUIRE(data.Node->IsConnected());
    }

    SECTION("Through nested shadow hosts, until the outermost host is removed")
    {
      auto doc = CreateRef<HTMLDocument>();
      auto outerHost = doc->CreateElement(u8"div").ReleaseValue();
      auto outer = outerHost->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto innerHost = doc->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(outer->AppendChild(*innerHost).HasException());
      auto inner = innerHost->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto child = doc->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(inner->AppendChild(*child).HasException());
      CHECK_FALSE(child->IsConnected());

      REQUIRE_FALSE(doc->AppendChild(*outerHost).HasException());
      CHECK(outer->IsConnected());
      CHECK(inner->IsConnected());
      CHECK(child->IsConnected());

      REQUIRE_FALSE(outerHost->Remove().HasException());
      CHECK_FALSE(inner->IsConnected());
      CHECK_FALSE(child->IsConnected());
    }
  }

  TEST_CASE("Node.ownerDocument: the node document, or null for a document",
            "[DOM][Node][dom:dom-node-ownerdocument]")
  {
    CommonTestData data {};

    REQUIRE(data.Node->OwnerDocument() == data.Document.get());
    REQUIRE(data.Document->OwnerDocument() == nullptr);
  }

  TEST_CASE("Node.getRootNode(): the root, or the shadow-including root when composed",
            "[DOM][Node][dom:dom-node-getrootnode]")
  {
    CommonTestData data {};

    REQUIRE(&data.Node->GetRootNode({.Composed = false}) == data.Node.get());
    REQUIRE(&data.Node->GetRootNode({.Composed = true}) == data.Node.get());

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());

    REQUIRE(&data.Node->GetRootNode({.Composed = false}) == data.Document.get());
    REQUIRE(&data.Node->GetRootNode({.Composed = true}) == data.Document.get());

    auto shadow = data.Node->AttachShadow({}).ReleaseValue();
    auto inShadow = CreateRef<TestElement>(*data.Document);
    REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());
    CHECK(&inShadow->GetRootNode({.Composed = false}) == shadow.get());
    CHECK(&inShadow->GetRootNode({.Composed = true}) == data.Document.get());
  }

  TEST_CASE("Node.parentNode: the parent", "[DOM][Node][dom:dom-node-parentnode]")
  {
    CommonTestData data {};

    REQUIRE(data.Node->ParentNode() == nullptr);

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE(data.Node->ParentNode() == data.Document.get());

    auto div = CreateRef<TestElement>(*data.Document);
    REQUIRE_FALSE(data.Node->AppendChild(*div).HasException());
    REQUIRE(div->ParentNode() == data.Node.get());

    REQUIRE_FALSE(div->Remove().HasException());
  }

  TEST_CASE("Node.parentElement: the parent if it's an element", "[DOM][Node][dom:dom-node-parentelement]")
  {
    CommonTestData data {};
    REQUIRE(data.Node->ParentElement() == nullptr);

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE(data.Node->ParentElement() == nullptr);

    auto div = CreateRef<TestElement>(*data.Document);
    REQUIRE_FALSE(data.Node->AppendChild(*div).HasException());
    REQUIRE(div->ParentElement() == data.Node.get());

    REQUIRE_FALSE(div->Remove().HasException());
  }

  TEST_CASE("Node.hasChildNodes(): whether there are children", "[DOM][Node][dom:dom-node-haschildnodes]")
  {
    CommonTestData data {true};
    REQUIRE_FALSE(data.Node->HasChildNodes());
    REQUIRE(data.Document->HasChildNodes());

    auto child = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child).HasException());
    REQUIRE(data.Node->HasChildNodes());
    REQUIRE(data.Document->HasChildNodes());

    REQUIRE_FALSE(child->Remove().HasException());
  }

  TEST_CASE("Node.childNodes: a live list of the children, the same each time",
            "[DOM][Node][dom:dom-node-childnodes]")
  {
    CommonTestData data {true};
    auto childA = CreateRef<TestElement>(*data.Document);
    auto childB = CreateRef<TestElement>(*data.Document);
    auto childC = CreateRef<TestElement>(*data.Document);

    Ref<NodeList> childNodes = data.Node->ChildNodes();
    CHECK(data.Node->ChildNodes().get() == childNodes.get());

    REQUIRE(childNodes->Length() == 0uz);
    REQUIRE(childNodes->Item(0uz) == nullptr);
    REQUIRE(childNodes->Item(1uz) == nullptr);

    REQUIRE_FALSE(data.Node->AppendChild(*childA).HasException());

    REQUIRE(childNodes->Length() == 1uz);
    REQUIRE(childNodes->Item(0uz) == childA.get());
    REQUIRE(childNodes->Item(1uz) == nullptr);

    REQUIRE_FALSE(data.Node->AppendChild(*childB).HasException());

    REQUIRE(childNodes->Length() == 2uz);
    REQUIRE(childNodes->Item(0uz) == childA.get());
    REQUIRE(childNodes->Item(1uz) == childB.get());

    REQUIRE_FALSE(data.Node->InsertBefore(*childC, childA.get()).HasException());

    REQUIRE(childNodes->Length() == 3uz);
    REQUIRE(childNodes->Item(0uz) == childC.get());
    REQUIRE(childNodes->Item(1uz) == childA.get());
    REQUIRE(childNodes->Item(2uz) == childB.get());

    REQUIRE_FALSE(childA->Remove().HasException());
    REQUIRE_FALSE(childB->Remove().HasException());
    REQUIRE_FALSE(childC->Remove().HasException());
  }

  TEST_CASE("FirstChild: the first child, or null without children",
            "[DOM][Node][dom:concept-tree-first-child][dom:dom-node-firstchild]")
  {
    CommonTestData data {true};
    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);

    REQUIRE(data.Node->FirstChild() == nullptr);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE(data.Node->FirstChild() == child1.get());

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE(data.Node->FirstChild() == child2.get());

    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE(data.Node->FirstChild() == nullptr);
  }

  TEST_CASE("LastChild: the last child, or null without children",
            "[DOM][Node][dom:concept-tree-last-child][dom:dom-node-lastchild]")
  {
    CommonTestData data {true};
    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);

    REQUIRE(data.Node->LastChild() == nullptr);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());
    REQUIRE(data.Node->LastChild() == child2.get());

    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
    REQUIRE(data.Node->LastChild() == child1.get());

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE(data.Node->LastChild() == nullptr);
  }

  TEST_CASE("NextSibling and PreviousSibling: the adjacent siblings, or null at either end",
            "[DOM][Node][dom:concept-tree-next-sibling][dom:concept-tree-previous-sibling]"
            "[dom:dom-node-nextsibling][dom:dom-node-previoussibling]")
  {
    CommonTestData data {true};
    auto sibling1 = CreateRef<TestElement>(*data.Document);
    auto sibling2 = CreateRef<TestElement>(*data.Document);
    auto sibling3 = CreateRef<TestElement>(*data.Document);

    REQUIRE(sibling1->NextSibling() == nullptr);
    REQUIRE(sibling1->PreviousSibling() == nullptr);

    REQUIRE_FALSE(data.Node->AppendChild(*sibling1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*sibling2).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*sibling3).HasException());

    REQUIRE(sibling1->PreviousSibling() == nullptr);
    REQUIRE(sibling1->NextSibling() == sibling2.get());
    REQUIRE(sibling2->PreviousSibling() == sibling1.get());
    REQUIRE(sibling2->NextSibling() == sibling3.get());
    REQUIRE(sibling3->PreviousSibling() == sibling2.get());
    REQUIRE(sibling3->NextSibling() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*sibling2).HasException());
    REQUIRE(sibling1->NextSibling() == sibling3.get());
    REQUIRE(sibling3->PreviousSibling() == sibling1.get());
    REQUIRE(sibling2->NextSibling() == nullptr);
    REQUIRE(sibling2->PreviousSibling() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*sibling1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*sibling3).HasException());
  }

  TEST_CASE("Node.nodeValue and textContent: null for a document or doctype, and setting them does nothing",
            "[DOM][Node][dom:dom-node-nodevalue][dom:dom-node-textcontent][dom:get-text-content]"
            "[dom:set-text-content]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto doctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*doctype).HasException());
    REQUIRE_FALSE(document->AppendChild(*document->CreateElement(u8"html").ReleaseValue()).HasException());

    for (Node *node : {static_cast<Node *>(document.get()), static_cast<Node *>(doctype.get())})
    {
      CHECK(node->NodeValue() == null);
      CHECK(node->TextContent() == null);
      REQUIRE_FALSE(node->NodeValue(u8"value").HasException());
      REQUIRE_FALSE(node->TextContent(u8"value").HasException());
      CHECK(node->NodeValue() == null);
      CHECK(node->TextContent() == null);
    }
    CHECK(document->ChildNodes()->Length() == 2u);
  }

  TEST_CASE("Node.nodeValue: null for an element or fragment, and setting it does nothing",
            "[DOM][Node][dom:dom-node-nodevalue]")
  {
    CommonTestData data {};
    auto fragment = data.Document->CreateDocumentFragment();

    for (Node *node : {static_cast<Node *>(data.Node.get()), static_cast<Node *>(fragment.get())})
    {
      REQUIRE(node->NodeValue() == null);
      REQUIRE_FALSE(node->NodeValue(u8"test value").HasException());
      REQUIRE(node->NodeValue() == null);
      CHECK_FALSE(node->HasChildNodes());
    }
  }

  TEST_CASE("Node.normalize(): empty exclusive Text nodes go, and contiguous ones merge into the first",
            "[DOM][Node][dom:dom-node-normalize]")
  {
    CommonTestData data {};
    auto text = [&](DOMString &&value)
    {
      return CreateRef<dom::Text>(*data.Document, krys::move(value));
    };
    auto childData = [](Node &parent)
    {
      List<DOMString> values;
      for (auto *child = parent.FirstChild(); child; child = child->NextSibling())
      {
        values.push_back(child->NodeValue().value_or(child->NodeName()));
      }
      return values;
    };

    SECTION("no children")
    {
      REQUIRE_FALSE(data.Node->Normalize().HasException());
      CHECK_FALSE(data.Node->HasChildNodes());
    }

    SECTION("an empty text node is removed")
    {
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"")).HasException());
      REQUIRE_FALSE(data.Node->Normalize().HasException());
      CHECK_FALSE(data.Node->HasChildNodes());
    }

    SECTION("contiguous text nodes merge into the first")
    {
      auto first = text(u8"Hello");
      REQUIRE_FALSE(data.Node->AppendChild(*first).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8" ")).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"world")).HasException());

      REQUIRE_FALSE(data.Node->Normalize().HasException());
      CHECK(data.Node->FirstChild() == first.get());
      CHECK(childData(*data.Node) == List<DOMString> {u8"Hello world"});
    }

    SECTION("an empty text node doesn't end the walk")
    {
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"")).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"a")).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"b")).HasException());
      auto inner = CreateRef<TestElement>(*data.Document);
      REQUIRE_FALSE(inner->AppendChild(*text(u8"")).HasException());
      REQUIRE_FALSE(inner->AppendChild(*text(u8"c")).HasException());
      REQUIRE_FALSE(inner->AppendChild(*text(u8"d")).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*inner).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"e")).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*text(u8"")).HasException());

      REQUIRE_FALSE(data.Node->Normalize().HasException());
      CHECK(childData(*data.Node) == List<DOMString> {u8"ab", u8"test-element", u8"e"});
      CHECK(childData(*inner) == List<DOMString> {u8"cd"});
    }

    SECTION("CDATA sections aren't exclusive Text nodes")
    {
      auto document = CreateRef<XMLDocument>();
      auto element = document->CreateElement(u8"e").ReleaseValue();
      REQUIRE_FALSE(element->AppendChild(*document->CreateTextNode(u8"a")).HasException());
      REQUIRE_FALSE(element->AppendChild(*document->CreateCDATASection(u8"b").ReleaseValue()).HasException());
      REQUIRE_FALSE(element->AppendChild(*document->CreateTextNode(u8"c")).HasException());
      REQUIRE_FALSE(element->Normalize().HasException());
      CHECK(element->ChildNodes()->Length() == 3u);
    }

    SECTION("live ranges in the merged nodes move into the first")
    {
      auto first = text(u8"ab");
      auto second = text(u8"cd");
      auto third = text(u8"ef");
      REQUIRE_FALSE(data.Node->AppendChild(*first).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*second).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*third).HasException());

      // 6a. and 6b. Boundary points in a merged node.
      auto inText = data.Document->CreateRange();
      REQUIRE_FALSE(inText->SetStart(*second, 1).HasException());
      REQUIRE_FALSE(inText->SetEnd(*third, 2).HasException());

      // 6c. and 6d. Boundary points in the parent, before a merged node.
      auto inParent = data.Document->CreateRange();
      REQUIRE_FALSE(inParent->SetStart(*data.Node, 1).HasException());
      REQUIRE_FALSE(inParent->SetEnd(*data.Node, 2).HasException());

      REQUIRE_FALSE(data.Node->Normalize().HasException());
      CHECK(inText->StartContainer() == first.get());
      CHECK(inText->StartOffset() == 3u);
      CHECK(inText->EndContainer() == first.get());
      CHECK(inText->EndOffset() == 6u);
      CHECK(inParent->StartContainer() == first.get());
      CHECK(inParent->StartOffset() == 2u);
      CHECK(inParent->EndContainer() == first.get());
      CHECK(inParent->EndOffset() == 4u);
    }

    REQUIRE_FALSE(data.Node->TextContent(u8"").HasException());
  }

  TEST_CASE("Node.cloneNode(): a clone of the node, and of its subtree if asked, but never of a shadow root",
            "[DOM][Node][dom:dom-node-clonenode]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(element->AppendChild(*document->CreateTextNode(u8"text")).HasException());

    auto shallow = element->CloneNode().ReleaseValue();
    CHECK(shallow.get() != element.get());
    CHECK_FALSE(shallow->HasChildNodes());

    auto deep = element->CloneNode(true).ReleaseValue();
    CHECK(deep->IsEqualNode(element.get()));

    auto shadow = element->AttachShadow({.Clonable = Clonable(true)}).ReleaseValue();
    auto result = shadow->CloneNode(true);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
  }

  TEST_CASE("Node.isEqualNode(): false for null, else whether the nodes are equal",
            "[DOM][Node][dom:dom-node-isequalnode]")
  {
    CommonTestData data {};
    auto equalNode = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->IsEqualNode(nullptr));
    REQUIRE(data.Node->IsEqualNode(data.Node.get()));
    REQUIRE(data.Node->IsEqualNode(equalNode.get()));
    REQUIRE_FALSE(data.Node->IsEqualNode(data.Document.get()));
  }

  TEST_CASE("Node.isSameNode(): whether the other node is this one", "[DOM][Node][dom:dom-node-issamenode]")
  {
    CommonTestData data {};

    REQUIRE_FALSE(data.Node->IsSameNode(nullptr));
    REQUIRE(data.Node->IsSameNode(data.Node.get()));
    REQUIRE_FALSE(data.Node->IsSameNode(data.Document.get()));
    REQUIRE_FALSE(data.Node->IsSameNode(CreateRef<TestElement>(*data.Document).get()));
  }

  TEST_CASE("Node.compareDocumentPosition(): where the other node is relative to this one",
            "[DOM][Node][dom:dom-node-comparedocumentposition][dom:dom-node-document_position_disconnected]"
            "[dom:dom-node-document_position_preceding][dom:dom-node-document_position_following]"
            "[dom:dom-node-document_position_contains][dom:dom-node-document_position_contained_by]"
            "[dom:dom-node-document_position_implementation_specific]")
  {
    using enum DocumentPosition;

    SECTION("same node returns equivalent")
    {
      CommonTestData data {};
      REQUIRE(data.Node->CompareDocumentPosition(*data.Node) == DOCUMENT_POSITION_EQUIVALENT);
    }

    SECTION("disconnected node")
    {
      CommonTestData data {true};

      auto disconnectedNode = CreateRef<TestElement>(*data.Document);
      auto position = data.Node->CompareDocumentPosition(*disconnectedNode);

      REQUIRE(HasFlag(position, DOCUMENT_POSITION_DISCONNECTED));
      REQUIRE(HasFlag(position, DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC));

      auto nodePrecedesDisconnected = HasFlag(position, DOCUMENT_POSITION_PRECEDING);
      // Should be either preceding or following, but not both.
      REQUIRE(nodePrecedesDisconnected ^ HasFlag(position, DOCUMENT_POSITION_FOLLOWING));

      auto reversePosition = disconnectedNode->CompareDocumentPosition(*data.Node);

      REQUIRE(HasFlag(reversePosition, DOCUMENT_POSITION_DISCONNECTED));
      REQUIRE(HasFlag(reversePosition, DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC));

      auto reverseNodePrecedesDisconnected = HasFlag(reversePosition, DOCUMENT_POSITION_PRECEDING);
      // Should be either preceding or following, but not both.
      REQUIRE(reverseNodePrecedesDisconnected ^ HasFlag(reversePosition, DOCUMENT_POSITION_FOLLOWING));

      // If it was following one way, it should be preceding the other.
      REQUIRE(reverseNodePrecedesDisconnected != nodePrecedesDisconnected);
    }

    SECTION("nodes in same document")
    {
      CommonTestData data {true};
      auto sibling1 = CreateRef<TestElement>(*data.Document);
      auto sibling2 = CreateRef<TestElement>(*data.Document);

      REQUIRE_FALSE(data.Node->AppendChild(*sibling1).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*sibling2).HasException());

      CHECK(data.Node->CompareDocumentPosition(*sibling1)
            == (DOCUMENT_POSITION_CONTAINED_BY | DOCUMENT_POSITION_FOLLOWING));
      CHECK(sibling1->CompareDocumentPosition(*data.Node)
            == (DOCUMENT_POSITION_CONTAINS | DOCUMENT_POSITION_PRECEDING));
      CHECK(data.Node->CompareDocumentPosition(*sibling2)
            == (DOCUMENT_POSITION_CONTAINED_BY | DOCUMENT_POSITION_FOLLOWING));
      CHECK(sibling2->CompareDocumentPosition(*data.Node)
            == (DOCUMENT_POSITION_CONTAINS | DOCUMENT_POSITION_PRECEDING));
      CHECK(sibling1->CompareDocumentPosition(*sibling2) == DOCUMENT_POSITION_FOLLOWING);
      CHECK(sibling2->CompareDocumentPosition(*sibling1) == DOCUMENT_POSITION_PRECEDING);

      REQUIRE_FALSE(data.Node->RemoveChild(*sibling1).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*sibling2).HasException());
    }

    SECTION("attributes")
    {
      auto document = CreateRef<HTMLDocument>();
      auto element = document->CreateElement(u8"div").ReleaseValue();
      auto child = document->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(element->AppendChild(*child).HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"a", u8"1").HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"b", u8"2").HasException());
      auto a = element->GetAttributeNode(u8"a");
      auto b = element->GetAttributeNode(u8"b");

      // 5b. Two attributes of the same element: by their order in its attribute list.
      CHECK(b->CompareDocumentPosition(*a)
            == (DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC | DOCUMENT_POSITION_PRECEDING));
      CHECK(a->CompareDocumentPosition(*b)
            == (DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC | DOCUMENT_POSITION_FOLLOWING));

      // 7. and 8. An attribute is inside its element, and before its element's children.
      CHECK(a->CompareDocumentPosition(*element)
            == (DOCUMENT_POSITION_CONTAINS | DOCUMENT_POSITION_PRECEDING));
      CHECK(element->CompareDocumentPosition(*a)
            == (DOCUMENT_POSITION_CONTAINED_BY | DOCUMENT_POSITION_FOLLOWING));
      CHECK(a->CompareDocumentPosition(*child) == DOCUMENT_POSITION_FOLLOWING);
      CHECK(child->CompareDocumentPosition(*a) == DOCUMENT_POSITION_PRECEDING);
    }

    SECTION("attributes without an element are disconnected, consistently")
    {
      auto document = CreateRef<HTMLDocument>();
      auto a = document->CreateAttribute(u8"a").ReleaseValue();
      auto b = document->CreateAttribute(u8"b").ReleaseValue();
      auto element = document->CreateElement(u8"div").ReleaseValue();

      for (auto [first, second] : {std::pair<Node *, Node *> {a.get(), b.get()}, {a.get(), element.get()}})
      {
        auto forward = first->CompareDocumentPosition(*second);
        auto backward = second->CompareDocumentPosition(*first);
        CHECK(HasFlag(forward, DOCUMENT_POSITION_DISCONNECTED | DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC));
        CHECK(HasFlag(backward, DOCUMENT_POSITION_DISCONNECTED | DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC));
        CHECK(HasFlag(forward, DOCUMENT_POSITION_PRECEDING)
              == HasFlag(backward, DOCUMENT_POSITION_FOLLOWING));
        CHECK(HasFlag(forward, DOCUMENT_POSITION_PRECEDING) != HasFlag(forward, DOCUMENT_POSITION_FOLLOWING));
      }
    }
  }

  TEST_CASE("Node.contains(): whether the other node is an inclusive descendant",
            "[DOM][Node][dom:dom-node-contains]")
  {
    CommonTestData data {};
    auto grandchild = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->Contains(nullptr));
    REQUIRE(data.Node->Contains(data.Node.get()));
    REQUIRE_FALSE(data.Node->Contains(data.Document.get()));
    REQUIRE_FALSE(data.Document->Contains(data.Node.get()));
    REQUIRE_FALSE(data.Node->Contains(grandchild.get()));
    REQUIRE_FALSE(data.Document->Contains(grandchild.get()));

    REQUIRE_FALSE(data.Document->AppendChild(*data.Node).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*grandchild).HasException());

    REQUIRE(data.Document->Contains(data.Node.get()));
    REQUIRE(data.Node->Contains(grandchild.get()));
    REQUIRE(data.Document->Contains(grandchild.get()));
    REQUIRE_FALSE(grandchild->Contains(data.Node.get()));
    REQUIRE_FALSE(grandchild->Contains(data.Document.get()));
    REQUIRE_FALSE(data.Node->Contains(data.Document.get()));

    REQUIRE_FALSE(data.Node->RemoveChild(*grandchild).HasException());
  }

  TEST_CASE("Node.lookupPrefix(): null for no namespace, else located from the relevant element",
            "[DOM][Node][dom:dom-node-lookupprefix]")
  {
    CommonTestData data {};

    SECTION("a null or empty namespace has no prefix")
    {
      auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
      CHECK(element->LookupPrefix(DOMStringAtom::Null()) == DOMStringAtom::Null());
      CHECK(element->LookupPrefix(DOMStringAtom::Empty()) == DOMStringAtom::Null());
    }

    SECTION("node is an Element")
    {
      SECTION("returns its namespace prefix if it has one")
      {
        auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE(element->LookupPrefix(u8"http://a") == u8"testprefix");
      }

      SECTION("returns null if it doesn't have a namespace prefix")
      {
        auto element = CreateRef<NamespaceTestElement>(*data.Document, dom::DOMStringAtom::Null());
        REQUIRE(element->LookupPrefix(u8"http://a") == dom::DOMStringAtom::Null());
      }

      SECTION("returns null if its namespace doesn't match")
      {
        auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
        REQUIRE(element->LookupPrefix(u8"http://b") == dom::DOMStringAtom::Null());
      }
    }

    SECTION("Node is a Document")
    {
      SECTION("returns null if its documentElement is null")
      {
        REQUIRE(data.Document->LookupPrefix(u8"foo") == dom::DOMStringAtom::Null());
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
      REQUIRE(fragment->LookupPrefix(u8"foo") == dom::DOMStringAtom::Null());
    }

    SECTION("returns null if Node is a DocumentType")
    {
      auto doctype = data.Document->Implementation().CreateDocumentType(u8"test", u8"", u8"");
      REQUIRE(doctype->LookupPrefix(u8"foo") == dom::DOMStringAtom::Null());
    }

    SECTION("Node is an Attr")
    {
      SECTION("returns null if it doesn't have an element owner")
      {
        auto attr = data.Document->CreateAttribute(u8"test");
        REQUIRE_FALSE(attr.HasException());
        REQUIRE(attr->LookupPrefix(u8"foo") == dom::DOMStringAtom::Null());
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
      auto textNode = CreateRef<dom::Text>(*data.Document, u8"");

      SECTION("returns null if parent element is null")
      {
        REQUIRE(textNode->LookupPrefix(u8"foo") == dom::DOMStringAtom::Null());
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

  TEST_CASE("Node.lookupNamespaceURI(): the empty prefix is null",
            "[DOM][Node][dom:dom-node-lookupnamespaceuri]")
  {
    CommonTestData data {};
    auto element = CreateRef<NamespaceTestElement>(*data.Document, DOMStringAtom::Null());
    CHECK(element->LookupNamespaceURI(DOMStringAtom::Empty()) == u8"http://a");
    CHECK(element->LookupNamespaceURI(DOMStringAtom::Null()) == u8"http://a");
    CHECK(element->LookupNamespaceURI(u8"other") == DOMStringAtom::Null());
  }

  TEST_CASE(
    "Node.isDefaultNamespace(): whether the namespace is the default one, the empty string being null",
    "[DOM][Node][dom:dom-node-isdefaultnamespace]")
  {
    CommonTestData data {};
    auto element = CreateRef<NamespaceTestElement>(*data.Document, u8"testprefix");
    REQUIRE_FALSE(element->IsDefaultNamespace(u8"foo"));
    REQUIRE_FALSE(element->IsDefaultNamespace(u8"http://a"));
    REQUIRE(element->IsDefaultNamespace(dom::DOMStringAtom::Null()));
    REQUIRE(element->IsDefaultNamespace(dom::DOMStringAtom::Empty()));

    auto unprefixed = CreateRef<NamespaceTestElement>(*data.Document, DOMStringAtom::Null());
    REQUIRE(unprefixed->IsDefaultNamespace(u8"http://a"));
    REQUIRE_FALSE(unprefixed->IsDefaultNamespace(DOMStringAtom::Empty()));
  }

  TEST_CASE("Node.insertBefore(): pre-inserts the node before the child",
            "[DOM][Node][dom:dom-node-insertbefore]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestElement>(*data.Document);
    auto first = CreateRef<TestElement>(*data.Document);

    auto insertResult = data.Node->InsertBefore(*child, nullptr);
    REQUIRE_FALSE(insertResult.HasException());
    REQUIRE(&insertResult.Value() == child.get());
    REQUIRE(data.Node->FirstChild() == child.get());

    REQUIRE_FALSE(data.Node->InsertBefore(*first, child.get()).HasException());
    REQUIRE(data.Node->FirstChild() == first.get());
    REQUIRE(data.Node->LastChild() == child.get());

    // A node that can't have children throws as pre-insert's validity check does.
    auto text = data.Document->CreateTextNode(u8"text");
    auto result = text->InsertBefore(*CreateRef<TestElement>(*data.Document), nullptr);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

    REQUIRE_FALSE(data.Node->RemoveChild(*child).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*first).HasException());
  }

  TEST_CASE("Node.appendChild(): appends the node", "[DOM][Node][dom:dom-node-appendchild]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestElement>(*data.Document);

    auto appendResult = data.Node->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(&appendResult.Value() == child.get());
    REQUIRE(data.Node->FirstChild() == child.get());
    REQUIRE(data.Node->LastChild() == child.get());

    auto text = data.Document->CreateTextNode(u8"text");
    auto result = text->AppendChild(*CreateRef<TestElement>(*data.Document));
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

    REQUIRE_FALSE(data.Node->RemoveChild(*child).HasException());
  }

  TEST_CASE("Node.replaceChild(): replaces the child with the node, returning the child",
            "[DOM][Node][dom:dom-node-replacechild]")
  {
    CommonTestData data {true};
    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());

    auto replaceResult = data.Node->ReplaceChild(*child2, *child1);
    REQUIRE_FALSE(replaceResult.HasException());
    REQUIRE(&replaceResult.Value() == child1.get());
    REQUIRE(data.Node->FirstChild() == child2.get());
    REQUIRE(data.Node->LastChild() == child2.get());

    auto text = data.Document->CreateTextNode(u8"text");
    auto result = text->ReplaceChild(*child1, *child2);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);

    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
  }

  TEST_CASE("Node.removeChild(): pre-removes the child", "[DOM][Node][dom:dom-node-removechild]")
  {
    CommonTestData data {true};
    auto child = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child).HasException());

    auto removeResult = data.Node->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    REQUIRE(&removeResult.Value() == child.get());
    REQUIRE(data.Node->FirstChild() == nullptr);

    // Not a child.
    auto again = data.Node->RemoveChild(*child);
    REQUIRE(again.HasException());
    CHECK(again.GetException().Code() == ExceptionCode::NotFoundError);
    auto text = data.Document->CreateTextNode(u8"text");
    auto fromText = text->RemoveChild(*child);
    REQUIRE(fromText.HasException());
    CHECK(fromText.GetException().Code() == ExceptionCode::NotFoundError);
  }

  TEST_CASE("Introduction to the DOM: markup goes in, a tree of nodes comes out",
            "[DOM][Node][dom:introduction-to-the-dom]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto result = Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html>\n"
                                                      u8"<html class=e>\n"
                                                      u8" <head><title>Aliens?</title></head>\n"
                                                      u8" <body>Why yes.</body>\n"
                                                      u8"</html>");
    REQUIRE_FALSE(result.HasException());

    // Each node on a line, indented by its depth: its name, then an element's class or a text's data.
    DOMString tree;
    Func<void(const Node &, uint32)> dump = [&](const Node &node, uint32 depth)
    {
      tree += DOMString(depth * 2, u8' ') + node.NodeName();
      if (auto *element = DynamicDowncast<Element>(node); element && element->HasAttribute(u8"class"))
      {
        tree += u8" class=\"" + *element->GetAttribute(u8"class") + u8"\"";
      }
      if (auto value = node.NodeValue())
      {
        tree += u8" " + *value;
      }
      tree += u8"|";
      for (auto *child = node.FirstChild(); child; child = child->NextSibling())
      {
        dump(*child, depth + 1);
      }
    };
    dump(*result.Value(), 0);

    CHECK(tree
          == u8"#document|"
             u8"  html|"
             u8"  HTML class=\"e\"|"
             u8"    HEAD|"
             u8"      TITLE|"
             u8"        #text Aliens?|"
             u8"    #text \n |"
             u8"    BODY|"
             u8"      #text Why yes.\n|");
  }
}
