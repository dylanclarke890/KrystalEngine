#include "Krystal.HTML/Mixins/ChildNode.hpp"
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

  TEST_CASE("ChildNode::Before", "[Mixins][ChildNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(parent->AppendChild(*element1).HasException());

    REQUIRE_FALSE(Mixins::ChildNode::Before(*element1, {element2, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element2);
    REQUIRE(parent->LastChild() == element1);
    REQUIRE(parent->FirstChild()->NextSibling()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->FirstChild()->NextSibling()->NodeValue() == u8"text");

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  TEST_CASE("ChildNode::After", "[Mixins][ChildNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(parent->AppendChild(*element1).HasException());

    REQUIRE_FALSE(Mixins::ChildNode::After(*element1, {element2, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element1);
    REQUIRE(parent->FirstChild()->NextSibling() == element2);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  TEST_CASE("ChildNode::ReplaceWith", "[Mixins][ChildNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element1 = document->CreateElement(u8"span").Value();
    Ref<Element> element2 = document->CreateElement(u8"span").Value();

    REQUIRE_FALSE(parent->AppendChild(*element1).HasException());

    REQUIRE_FALSE(Mixins::ChildNode::ReplaceWith(*element1, {element2, u8"text"}).HasException());
    REQUIRE(parent->FirstChild() == element2);
    REQUIRE(parent->LastChild()->NodeType() == NodeType::TEXT_NODE);
    REQUIRE(parent->LastChild()->NodeValue() == u8"text");

    REQUIRE_FALSE(element1->Remove().HasException());
    REQUIRE_FALSE(element2->Remove().HasException());
  }

  TEST_CASE("ChildNode::Remove", "[Mixins][ChildNode]")
  {
    auto document = CreateRef<HTMLDocument>();
    Ref<Element> parent = document->CreateElement(u8"div").Value();
    Ref<Element> element = document->CreateElement(u8"span").Value();
    
    REQUIRE_FALSE(parent->AppendChild(*element).HasException());
    
    REQUIRE_FALSE(Mixins::ChildNode::Remove(*element).HasException());
 
    REQUIRE(parent->FirstChild() == nullptr);
  }
}