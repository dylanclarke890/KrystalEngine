#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML.Tests/TestContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("ContainerNode::HasChildNodes", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto container = CreateRef<TestContainerNode>(*doc);
    REQUIRE_FALSE(container->HasChildNodes());

    auto child = CreateRef<TestContainerNode>(*doc);
    auto appendResult = container->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());
    REQUIRE(container->HasChildNodes());

    container->RemoveChild(*child);
  }

  TEST_CASE("ContainerNode::ChildNodes", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto container = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    auto result = container->AppendChild(*childA);
    REQUIRE_FALSE(result.HasException());

    result = container->AppendChild(*childB);
    REQUIRE_FALSE(result.HasException());

    Ref<NodeList> childNodes = container->ChildNodes();
    REQUIRE(childNodes->Length() == 2uz);
    REQUIRE(childNodes->Item(0uz) == childA.get());
    REQUIRE(childNodes->Item(1uz) == childB.get());

    container->RemoveChild(*childA);
    container->RemoveChild(*childB);
  }

  TEST_CASE("ContainerNode::FirstChild", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto container = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    auto result = container->AppendChild(*childA);
    REQUIRE_FALSE(result.HasException());
    REQUIRE(container->FirstChild() == childA.get());

    result = container->InsertBefore(*childB, childA);
    REQUIRE_FALSE(result.HasException());
    REQUIRE(container->FirstChild() == childB.get());

    container->RemoveChild(*childA);
    container->RemoveChild(*childB);
  }

  TEST_CASE("ContainerNode::LastChild", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto container = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    auto result = container->AppendChild(*childA);
    REQUIRE_FALSE(result.HasException());
    REQUIRE(container->LastChild() == childA.get());

    result = container->AppendChild(*childB);
    REQUIRE_FALSE(result.HasException());
    REQUIRE(container->LastChild() == childB.get());

    container->RemoveChild(*childA);
    container->RemoveChild(*childB);
  }

  TEST_CASE("ContainerNode::AppendChild/RemoveChild", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    // Append childA
    {
      auto result = parent->AppendChild(*childA);
      REQUIRE(!result.HasException());

      REQUIRE(parent->FirstChild() == childA.get());
      REQUIRE(parent->LastChild() == childA.get());

      REQUIRE(childA->ParentNode() == parent.get());
      REQUIRE(!childA->PreviousSibling());
      REQUIRE(!childA->NextSibling());
    }

    // Append childB
    {
      auto result = parent->AppendChild(*childB);
      REQUIRE(!result.HasException());

      REQUIRE(parent->FirstChild() == childA.get());
      REQUIRE(parent->LastChild() == childB.get());

      REQUIRE(childB->ParentNode() == parent.get());
      REQUIRE(!childA->PreviousSibling());
      REQUIRE(childA->NextSibling() == childB.get());
      REQUIRE(childB->PreviousSibling() == childA.get());
      REQUIRE(!childB->NextSibling());
    }

    // Append childA again - moves to end
    {
      auto result = parent->AppendChild(*childA);
      REQUIRE(!result.HasException());

      REQUIRE(parent->FirstChild() == childB.get());
      REQUIRE(parent->LastChild() == childA.get());

      REQUIRE(childA->ParentNode() == parent.get());
      REQUIRE(!childB->PreviousSibling());
      REQUIRE(childB->NextSibling() == childA.get());
      REQUIRE(childA->PreviousSibling() == childB.get());
      REQUIRE(!childA->NextSibling());
    }

    // Remove childB using RemoveChild
    {
      auto result = parent->RemoveChild(*childB);
      REQUIRE(!result.HasException());

      REQUIRE(parent->FirstChild() == childA.get());
      REQUIRE(parent->LastChild() == childA.get());

      REQUIRE(childA->ParentNode() == parent.get());
      REQUIRE(!childA->PreviousSibling());
      REQUIRE(!childA->NextSibling());
    }

    parent->RemoveChild(*childA);
  }

  TEST_CASE("ContainerNode::InsertBefore", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);
    auto childC = CreateRef<TestContainerNode>(*doc);

    auto result = parent->AppendChild(*childA);
    REQUIRE(!result.HasException());

    result = parent->AppendChild(*childB);
    REQUIRE(!result.HasException());

    // Inserting child before itself does nothing
    {
      result = parent->InsertBefore(*childA, ShareRefPtr<Node>(childA.get()));
      REQUIRE(!result.HasException());

      REQUIRE(parent->FirstChild() == childA.get());
      REQUIRE(childA->ParentNode() == parent.get());

      REQUIRE(parent->LastChild() == childB.get());
      REQUIRE(childB->ParentNode() == parent.get());

      REQUIRE(!childA->PreviousSibling());
      REQUIRE(childA->NextSibling() == childB.get());

      REQUIRE(childB->PreviousSibling() == childA.get());
      REQUIRE(!childB->NextSibling());
    }

    // Insertion with new node
    {
      result = parent->InsertBefore(*childC, ShareRefPtr<Node>(childA.get()));
      REQUIRE(!result.HasException());

      REQUIRE(parent->FirstChild() == childC.get());
      REQUIRE(childC->ParentNode() == parent.get());

      REQUIRE(parent->LastChild() == childB.get());
      REQUIRE(childB->ParentNode() == parent.get());

      REQUIRE(!childC->PreviousSibling());
      REQUIRE(childC->NextSibling() == childA.get());

      REQUIRE(childA->PreviousSibling() == childC.get());
      REQUIRE(childA->NextSibling() == childB.get());
      REQUIRE(childB->PreviousSibling() == childA.get());

      REQUIRE(childC->NextSibling() == childA.get());
      REQUIRE(!childB->NextSibling());
    }

    parent->RemoveChild(*childA);
    parent->RemoveChild(*childB);
    parent->RemoveChild(*childC);
  }

  TEST_CASE("ContainerNode::ReplaceChild", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto parent = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    auto result = parent->AppendChild(*childA);
    REQUIRE(!result.HasException());

    result = parent->AppendChild(*childB);
    REQUIRE(!result.HasException());

    SECTION("Replace with same child does nothing")
    {
      result = parent->ReplaceChild(*childA, *childA);
      REQUIRE(!result.HasException());
      REQUIRE(parent->FirstChild() == childA.get());
      REQUIRE(childB->PreviousSibling() == childA.get());
    }

    parent->RemoveChild(*childA);
    parent->RemoveChild(*childB);
  }

  TEST_CASE("ContainerNode::Length", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto container = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    auto result = container->AppendChild(*childA);
    REQUIRE_FALSE(result.HasException());

    result = container->AppendChild(*childB);
    REQUIRE_FALSE(result.HasException());

    REQUIRE(container->Length() == 2uz);

    container->RemoveChild(*childA);
    container->RemoveChild(*childB);
  }

  TEST_CASE("ContainerNode::CountChildNodes", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    auto container = CreateRef<TestContainerNode>(*doc);
    auto childA = CreateRef<TestContainerNode>(*doc);
    auto childB = CreateRef<TestContainerNode>(*doc);

    auto result = container->AppendChild(*childA);
    REQUIRE_FALSE(result.HasException());

    result = container->AppendChild(*childB);
    REQUIRE_FALSE(result.HasException());

    REQUIRE(container->CountChildNodes() == 2uz);

    container->RemoveChild(*childA);
    container->RemoveChild(*childB);
  }
}