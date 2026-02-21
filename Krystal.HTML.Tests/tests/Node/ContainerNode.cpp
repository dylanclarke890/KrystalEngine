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

      // Remove childB
      {
        auto result = parent->RemoveChild(*childB);
        REQUIRE(!result.HasException());

        REQUIRE(parent->FirstChild() == childA.get());
        REQUIRE(parent->LastChild() == childA.get());

        REQUIRE(childA->ParentNode() == parent.get());
        REQUIRE(!childA->PreviousSibling());
        REQUIRE(!childA->NextSibling());
      }

      // Remove childA
      {
        auto result = parent->RemoveChild(*childA);
        REQUIRE(!result.HasException());

        REQUIRE(!parent->FirstChild());
        REQUIRE(!parent->LastChild());

        REQUIRE(!childA->ParentNode());
        REQUIRE(!childA->PreviousSibling());
        REQUIRE(!childA->NextSibling());
      }
    }
  }
}