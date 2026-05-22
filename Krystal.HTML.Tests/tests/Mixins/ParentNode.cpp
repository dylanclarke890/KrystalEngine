#include "Krystal.HTML/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("ParentNode::ConvertNodesIntoNode", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("returns the single node if the list contains only one node")
    {
      SECTION("element node")
      {
        List<NodeOrString> nodes;
        nodes.emplace_back(document->CreateElement(u8"div").Value());

        auto result = Mixins::ParentNode::ConvertNodesIntoNode(nodes, *document);
        REQUIRE_FALSE(result.HasException());

        REQUIRE(Is<Element>(result.Value()));
        auto &element = Downcast<Element>(*result.Value());
        REQUIRE(element.TagName() == u8"DIV");
      }

      SECTION("text node")
      {
        List<NodeOrString> nodes;
        nodes.emplace_back(u8"Hello");

        auto result = Mixins::ParentNode::ConvertNodesIntoNode(nodes, *document);
        REQUIRE_FALSE(result.HasException());

        auto &textNode = result.Value();
        REQUIRE(Is<HTML::Text>(textNode));
        REQUIRE(Downcast<HTML::Text>(*textNode).Data() == u8"Hello");
      }
    }

    SECTION("returns a DocumentFragment containing all nodes if the list contains more than one node")
    {
      List<NodeOrString> nodes;
      nodes.emplace_back(document->CreateElement(u8"div").Value());
      nodes.emplace_back(u8"Hello");

      auto result = Mixins::ParentNode::ConvertNodesIntoNode(nodes, *document);
      REQUIRE_FALSE(result.HasException());

      REQUIRE(Is<DocumentFragment>(result.Value()));
      auto &fragment = Downcast<DocumentFragment>(*result.Value());

      REQUIRE(Is<Element>(fragment.FirstChild()));
      auto &element = Downcast<Element>(*fragment.FirstChild());
      REQUIRE(element.TagName() == u8"DIV");

      REQUIRE(Is<HTML::Text>(fragment.LastChild()));
      auto &textNode = Downcast<HTML::Text>(*fragment.LastChild());
      REQUIRE(textNode.Data() == u8"Hello");

      REQUIRE_FALSE(element.Remove().HasException());
      REQUIRE_FALSE(textNode.Remove().HasException());
    }
  }

  TEST_CASE("ParentNode::FirstElementChild", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();

    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<HTML::Text> child1 = document->CreateTextNode(u8"Hello");
    Ref<Element> child2 = document->CreateElement(u8"span").Value();
    Ref<Element> child3 = document->CreateElement(u8"p").Value();

    REQUIRE(Mixins::ParentNode::FirstElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child1).HasException());
    REQUIRE(Mixins::ParentNode::FirstElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child2).HasException());
    REQUIRE(Mixins::ParentNode::FirstElementChild(*parent) == child2);

    REQUIRE_FALSE(parent->AppendChild(*child3).HasException());
    REQUIRE(Mixins::ParentNode::FirstElementChild(*parent) == child2);

    REQUIRE_FALSE(child1->Remove().HasException());
    REQUIRE_FALSE(child2->Remove().HasException());
    REQUIRE_FALSE(child3->Remove().HasException());
  }

  TEST_CASE("ParentNode::LastElementChild", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();

    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<HTML::Text> child1 = document->CreateTextNode(u8"Hello");
    Ref<Element> child2 = document->CreateElement(u8"span").Value();
    Ref<Element> child3 = document->CreateElement(u8"p").Value();

    REQUIRE(Mixins::ParentNode::LastElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child1).HasException());
    REQUIRE(Mixins::ParentNode::LastElementChild(*parent) == nullptr);

    REQUIRE_FALSE(parent->AppendChild(*child2).HasException());
    REQUIRE(Mixins::ParentNode::LastElementChild(*parent) == child2);

    REQUIRE_FALSE(parent->AppendChild(*child3).HasException());
    REQUIRE(Mixins::ParentNode::LastElementChild(*parent) == child3);

    REQUIRE_FALSE(child1->Remove().HasException());
    REQUIRE_FALSE(child2->Remove().HasException());
    REQUIRE_FALSE(child3->Remove().HasException());
  }

  TEST_CASE("ParentNode::ChildElementCount", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<HTML::Text> child1 = document->CreateTextNode(u8"Hello");
    Ref<Element> child2 = document->CreateElement(u8"span").Value();
    Ref<Element> child3 = document->CreateElement(u8"p").Value();

    REQUIRE(Mixins::ParentNode::ChildElementCount(*parent) == 0uz);

    REQUIRE_FALSE(parent->AppendChild(*child1).HasException());
    REQUIRE(Mixins::ParentNode::ChildElementCount(*parent) == 0uz);

    REQUIRE_FALSE(parent->AppendChild(*child2).HasException());
    REQUIRE(Mixins::ParentNode::ChildElementCount(*parent) == 1uz);

    REQUIRE_FALSE(parent->AppendChild(*child3).HasException());
    REQUIRE(Mixins::ParentNode::ChildElementCount(*parent) == 2uz);

    REQUIRE_FALSE(child1->Remove().HasException());
    REQUIRE_FALSE(child2->Remove().HasException());
    REQUIRE_FALSE(child3->Remove().HasException());
  }

  TEST_CASE("ParentNode::Prepend", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(Mixins::ParentNode::Prepend(*parent, {element, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(element->Remove().HasException());
  }

  TEST_CASE("ParentNode::Append", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(Mixins::ParentNode::Append(*parent, {element, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(element->Remove().HasException());
  }

  TEST_CASE("ParentNode::ReplaceChildren", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();

    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"p").Value();

    REQUIRE_FALSE(Mixins::ParentNode::ReplaceChildren(*parent, {element1, u8"text"}).HasException());

    REQUIRE(parent->FirstChild() == element1);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(Mixins::ParentNode::ReplaceChildren(*parent, {element2}).HasException());

    REQUIRE(parent->FirstChild() == element2);
    REQUIRE(parent->LastChild() == element2);

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  TEST_CASE("ParentNode::MoveBefore", "[Mixins][ParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"p").Value();

    REQUIRE_FALSE(parent->AppendChild(*element1).HasException());
    REQUIRE_FALSE(parent->AppendChild(*element2).HasException());

    REQUIRE(parent->FirstChild() == element1);
    REQUIRE(parent->LastChild() == element2);

    REQUIRE_FALSE(Mixins::ParentNode::MoveBefore(*parent, *element2, element1.get()).HasException());

    REQUIRE(parent->FirstChild() == element2);
    REQUIRE(parent->LastChild() == element1);

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  // TODO(test): TEST_CASE("ParentNode::QuerySelector", "[Mixins][ParentNode]")
  // TODO(test): TEST_CASE("ParentNode::QuerySelectorAll", "[Mixins][ParentNode]")
}