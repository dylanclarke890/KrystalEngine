#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Convert nodes into a node: strings become text, and more than one node a fragment",
            "[DOM][ParentNode][dom:interface-parentnode][dom:convert-nodes-into-a-node]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("returns the single node if the list contains only one node")
    {
      SECTION("element node")
      {
        List<NodeOrString> nodes;
        nodes.emplace_back(document->CreateElement(u8"div").Value());

        auto result = mixins::ParentNode::ConvertNodesIntoNode(nodes, *document);
        REQUIRE_FALSE(result.HasException());

        REQUIRE(Is<Element>(result.Value()));
        auto &element = Downcast<Element>(*result.Value());
        REQUIRE(element.TagName() == u8"DIV");
      }

      SECTION("text node")
      {
        List<NodeOrString> nodes;
        nodes.emplace_back(u8"Hello");

        auto result = mixins::ParentNode::ConvertNodesIntoNode(nodes, *document);
        REQUIRE_FALSE(result.HasException());

        auto &textNode = result.Value();
        REQUIRE(Is<krys::boo::dom::Text>(textNode));
        REQUIRE(Downcast<krys::boo::dom::Text>(*textNode).Data() == u8"Hello");
      }
    }

    SECTION("returns a DocumentFragment containing all nodes if the list contains more than one node")
    {
      List<NodeOrString> nodes;
      nodes.emplace_back(document->CreateElement(u8"div").Value());
      nodes.emplace_back(u8"Hello");

      auto result = mixins::ParentNode::ConvertNodesIntoNode(nodes, *document);
      REQUIRE_FALSE(result.HasException());

      REQUIRE(Is<DocumentFragment>(result.Value()));
      auto &fragment = Downcast<DocumentFragment>(*result.Value());

      REQUIRE(Is<Element>(fragment.FirstChild()));
      auto &element = Downcast<Element>(*fragment.FirstChild());
      REQUIRE(element.TagName() == u8"DIV");

      REQUIRE(Is<krys::boo::dom::Text>(fragment.LastChild()));
      auto &textNode = Downcast<krys::boo::dom::Text>(*fragment.LastChild());
      REQUIRE(textNode.Data() == u8"Hello");

      REQUIRE_FALSE(element.Remove().HasException());
      REQUIRE_FALSE(textNode.Remove().HasException());
    }
  }

  TEST_CASE("firstElementChild: the first child that is an element",
            "[DOM][ParentNode][dom:dom-parentnode-firstelementchild]")
  {
    auto document = CreateRef<HTMLDocument>();

    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<krys::boo::dom::Text> child1 = document->CreateTextNode(u8"Hello");
    Ref<Element> child2 = document->CreateElement(u8"span").Value();
    Ref<Element> child3 = document->CreateElement(u8"p").Value();

    REQUIRE(mixins::ParentNode::FirstElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child1).HasException());
    REQUIRE(mixins::ParentNode::FirstElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child2).HasException());
    REQUIRE(mixins::ParentNode::FirstElementChild(*parent) == child2);

    REQUIRE_FALSE(parent->AppendChild(*child3).HasException());
    REQUIRE(mixins::ParentNode::FirstElementChild(*parent) == child2);

    REQUIRE_FALSE(child1->Remove().HasException());
    REQUIRE_FALSE(child2->Remove().HasException());
    REQUIRE_FALSE(child3->Remove().HasException());
  }

  TEST_CASE("lastElementChild: the last child that is an element",
            "[DOM][ParentNode][dom:dom-parentnode-lastelementchild]")
  {
    auto document = CreateRef<HTMLDocument>();

    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<krys::boo::dom::Text> child1 = document->CreateTextNode(u8"Hello");
    Ref<Element> child2 = document->CreateElement(u8"span").Value();
    Ref<Element> child3 = document->CreateElement(u8"p").Value();

    REQUIRE(mixins::ParentNode::LastElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child1).HasException());
    REQUIRE(mixins::ParentNode::LastElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child2).HasException());
    REQUIRE(mixins::ParentNode::LastElementChild(*parent) == child2);

    REQUIRE_FALSE(parent->AppendChild(*child3).HasException());
    REQUIRE(mixins::ParentNode::LastElementChild(*parent) == child3);

    REQUIRE_FALSE(child1->Remove().HasException());
    REQUIRE_FALSE(child2->Remove().HasException());
    REQUIRE_FALSE(child3->Remove().HasException());
  }

  TEST_CASE("childElementCount: the number of children that are elements",
            "[DOM][ParentNode][dom:dom-parentnode-childelementcount]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<krys::boo::dom::Text> child1 = document->CreateTextNode(u8"Hello");
    Ref<Element> child2 = document->CreateElement(u8"span").Value();
    Ref<Element> child3 = document->CreateElement(u8"p").Value();

    REQUIRE(mixins::ParentNode::ChildElementCount(*parent) == 0uz);

    REQUIRE_FALSE(parent->AppendChild(*child1).HasException());
    REQUIRE(mixins::ParentNode::ChildElementCount(*parent) == 0uz);

    REQUIRE_FALSE(parent->AppendChild(*child2).HasException());
    REQUIRE(mixins::ParentNode::ChildElementCount(*parent) == 1uz);

    REQUIRE_FALSE(parent->AppendChild(*child3).HasException());
    REQUIRE(mixins::ParentNode::ChildElementCount(*parent) == 2uz);

    REQUIRE_FALSE(child1->Remove().HasException());
    REQUIRE_FALSE(child2->Remove().HasException());
    REQUIRE_FALSE(child3->Remove().HasException());
  }

  TEST_CASE("prepend(): the nodes, with strings as text, before the first child",
            "[DOM][ParentNode][dom:dom-parentnode-prepend]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(mixins::ParentNode::Prepend(*parent, {element, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(element->Remove().HasException());
  }

  TEST_CASE("append(): the nodes, with strings as text, after the last child",
            "[DOM][ParentNode][dom:dom-parentnode-append]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(mixins::ParentNode::Append(*parent, {element, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(element->Remove().HasException());
  }

  TEST_CASE("replaceChildren(): the nodes, with strings as text, in place of all the children",
            "[DOM][ParentNode][dom:dom-parentnode-replacechildren]")
  {
    auto document = CreateRef<HTMLDocument>();

    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"p").Value();

    REQUIRE_FALSE(mixins::ParentNode::ReplaceChildren(*parent, {element1, u8"text"}).HasException());

    REQUIRE(parent->FirstChild() == element1);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(mixins::ParentNode::ReplaceChildren(*parent, {element2}).HasException());

    REQUIRE(parent->FirstChild() == element2);
    REQUIRE(parent->LastChild() == element2);

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  TEST_CASE("moveBefore(): moves the node before the child, or after it if it's the node",
            "[DOM][ParentNode][dom:dom-parentnode-movebefore]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"p").Value();

    REQUIRE_FALSE(parent->AppendChild(*element1).HasException());
    REQUIRE_FALSE(parent->AppendChild(*element2).HasException());

    REQUIRE(parent->FirstChild() == element1);
    REQUIRE(parent->LastChild() == element2);

    REQUIRE_FALSE(mixins::ParentNode::MoveBefore(*parent, *element2, element1.get()).HasException());

    REQUIRE(parent->FirstChild() == element2);
    REQUIRE(parent->LastChild() == element1);

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  TEST_CASE("children: a live collection of the element children",
            "[DOM][ParentNode][dom:dom-parentnode-children]")
  {
    // Rare data is created on first use.
    auto document = CreateRef<HTMLDocument>();
    auto div = document->CreateElement(u8"div").Value();
    auto span = document->CreateElement(u8"span").Value();
    REQUIRE_FALSE(document->AppendChild(*div).HasException());
    REQUIRE_FALSE(div->AppendChild(*span).HasException());
    CHECK(document->Children()->Length() == 1u);
    CHECK(span->Children()->Length() == 0u);
    CHECK(document->CreateDocumentFragment()->Children()->Length() == 0u);

    auto children = div->Children();
    REQUIRE(children->Length() == 1u);
    REQUIRE_FALSE(div->AppendChild(*document->CreateTextNode(u8"text")).HasException());
    REQUIRE_FALSE(div->AppendChild(*document->CreateElement(u8"b").Value()).HasException());
    CHECK(children->Length() == 2u);
    CHECK(children->Item(0) == span.get());
  }

  TEST_CASE("replaceChildren(): the children being replaced don't count against the document's constraints",
            "[DOM][ParentNode][dom:dom-parentnode-replacechildren]")
  {
    auto document = CreateRef<HTMLDocument>();
    REQUIRE_FALSE(
      document->AppendChild(*document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value())
        .HasException());
    REQUIRE_FALSE(document->AppendChild(*document->CreateElement(u8"html").Value()).HasException());

    SECTION("A new document element replaces them")
    {
      auto html = document->CreateElement(u8"html").Value();
      REQUIRE_FALSE(document->ReplaceChildren({Ref<Node>(html)}).HasException());
      CHECK(document->FirstChild() == html.get());
      CHECK(document->LastChild() == html.get());
    }

    SECTION("A doctype can't come with it, as converting the nodes puts them in a fragment")
    {
      auto doctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"").Value();
      auto html = document->CreateElement(u8"html").Value();
      CHECK(document->ReplaceChildren({Ref<Node>(doctype), Ref<Node>(html)}).HasException());
    }

    SECTION("Two elements are still refused, leaving the children as they were")
    {
      auto first = document->CreateElement(u8"html").Value();
      auto second = document->CreateElement(u8"html").Value();
      CHECK(document->ReplaceChildren({Ref<Node>(first), Ref<Node>(second)}).HasException());
      CHECK(document->ChildElementCount() == 1u);
      CHECK(document->FirstChild()->NodeType() == NodeType::DOCUMENT_TYPE_NODE);
    }
  }

  TEST_CASE(
    "querySelector() and querySelectorAll(): the first or all descendants that match, as a static list",
    "[DOM][ParentNode][dom:dom-parentnode-queryselector][dom:dom-parentnode-queryselectorall]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").Value();
    auto first = document->CreateElement(u8"p").Value();
    auto second = document->CreateElement(u8"p").Value();
    REQUIRE_FALSE(root->AppendChild(*first).HasException());
    REQUIRE_FALSE(root->AppendChild(*second).HasException());

    CHECK(root->QuerySelector(u8"p").Value() == first);
    CHECK(root->QuerySelector(u8"span").Value() == nullptr);
    CHECK(root->QuerySelector(u8"div").Value() == nullptr);
    CHECK(root->QuerySelector(u8"(").HasException());

    auto all = root->QuerySelectorAll(u8"p").ReleaseValue();
    REQUIRE(all->Length() == 2u);
    CHECK(all->Item(1) == second);

    // A static list doesn't change with the tree.
    REQUIRE_FALSE(root->AppendChild(*document->CreateElement(u8"p").Value()).HasException());
    CHECK(all->Length() == 2u);
  }
}