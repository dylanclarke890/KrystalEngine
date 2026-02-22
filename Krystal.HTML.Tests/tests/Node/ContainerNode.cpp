#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Element/Element.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestNode : public Element
  {
  public:
    TestNode(Document &document) noexcept : Element(document, NodeFlag::None)
    {
    }
  };

  TEST_CASE("ContainerNode - Basic node operations", "[ContainerNode]")
  {
    Ref<Document> doc = CreateRef<Document>();
    Ref<TestNode> parent = CreateRef<TestNode>(*doc);
    Ref<TestNode> childA = CreateRef<TestNode>(*doc);
    Ref<TestNode> childB = CreateRef<TestNode>(*doc);

    SECTION("Append/remove child")
    {
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

      // Remove childA using Remove
      {
        auto result = childA->Remove();
        REQUIRE(!result.HasException());

        REQUIRE(!parent->FirstChild());
        REQUIRE(!parent->LastChild());

        REQUIRE(!childA->ParentNode());
        REQUIRE(!childA->PreviousSibling());
        REQUIRE(!childA->NextSibling());
      }
    }

    SECTION("InsertBefore")
    {
      Ref<TestNode> childC = CreateRef<TestNode>(*doc);

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

        REQUIRE(childC->NextSibling() == childA.get());
        REQUIRE(!childB->NextSibling());
      }

      childC->Remove();
    }

    SECTION("Replace child")
    {
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
    }

    childA->Remove();
    childB->Remove();
  }
}