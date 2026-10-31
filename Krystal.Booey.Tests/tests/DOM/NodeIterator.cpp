#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Callbacks/NodeFilter.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Node;

      CommonTestData() : Document(CreateRef<HTMLDocument>()), Node(CreateRef<TestElement>(*Document))
      {
      }
    };

    class TextNodeFilter : public NodeFilter
    {
    public:
      KRYS_NODISCARD dom::ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept override
      {
        return node.NodeType() == NodeType::TEXT_NODE ? FilterResult::FILTER_ACCEPT
                                                      : FilterResult::FILTER_REJECT;
      }
    };
  }

  TEST_CASE("NodeIterator::Root", "[HTML][NodeIterator]")
  {
    CommonTestData data;

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&nodeIterator->Root() == data.Node);
  }

  TEST_CASE("NodeIterator::ReferenceNode and NodeIterator::PointerBeforeReferenceNode",
            "[HTML][NodeIterator]")
  {
    CommonTestData data;

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&nodeIterator->ReferenceNode() == data.Node);
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
  }

  TEST_CASE("NodeIterator::Detach", "[HTML][NodeIterator]")
  {
    CommonTestData data;
    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    nodeIterator->Detach();

    // Detach does nothing as per the spec, so the state should be unchanged.
    REQUIRE(&nodeIterator->ReferenceNode() == data.Node);
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
  }

  TEST_CASE("NodeIterator::NextNode and NodeIterator::PreviousNode", "[HTML][NodeIterator]")
  {
    CommonTestData data;

    SECTION("root with no children")
    {
      auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);

      auto nextNodeResult = nodeIterator->NextNode();
      REQUIRE_FALSE(nextNodeResult.HasException());
      REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(nextNodeResult.Value() == data.Node);

      nextNodeResult = nodeIterator->NextNode();
      REQUIRE_FALSE(nextNodeResult.HasException());
      REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(nextNodeResult.Value() == nullptr);

      auto previousNodeResult = nodeIterator->PreviousNode();
      REQUIRE_FALSE(previousNodeResult.HasException());
      REQUIRE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(previousNodeResult.Value() == data.Node);
    }

    SECTION("root with children")
    {
      auto sibling1 = data.Document->CreateElement(u8"sibling1");
      REQUIRE_FALSE(sibling1.HasException());

      auto sibling2 = data.Document->CreateElement(u8"sibling2");
      REQUIRE_FALSE(sibling2.HasException());

      REQUIRE_FALSE(data.Node->AppendChild(*sibling1.Value()).HasException());
      REQUIRE_FALSE(data.Node->AppendChild(*sibling2.Value()).HasException());

      auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);

      auto nextNodeResult = nodeIterator->NextNode();
      REQUIRE_FALSE(nextNodeResult.HasException());
      REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(nextNodeResult.Value() == data.Node);

      nextNodeResult = nodeIterator->NextNode();
      REQUIRE_FALSE(nextNodeResult.HasException());
      REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(nextNodeResult.Value() == sibling1.Value());

      nextNodeResult = nodeIterator->NextNode();
      REQUIRE_FALSE(nextNodeResult.HasException());
      REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(nextNodeResult.Value() == sibling2.Value());

      auto previousNodeResult = nodeIterator->PreviousNode();
      REQUIRE_FALSE(previousNodeResult.HasException());
      REQUIRE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(previousNodeResult.Value() == sibling2.Value());

      previousNodeResult = nodeIterator->PreviousNode();
      REQUIRE_FALSE(previousNodeResult.HasException());
      REQUIRE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(previousNodeResult.Value() == sibling1.Value());

      previousNodeResult = nodeIterator->PreviousNode();
      REQUIRE_FALSE(previousNodeResult.HasException());
      REQUIRE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(previousNodeResult.Value() == data.Node);

      previousNodeResult = nodeIterator->PreviousNode();
      REQUIRE_FALSE(previousNodeResult.HasException());
      REQUIRE(nodeIterator->PointerBeforeReferenceNode());
      REQUIRE(previousNodeResult.Value() == nullptr);

      REQUIRE_FALSE(data.Node->RemoveChild(*sibling1.Value()).HasException());
      REQUIRE_FALSE(data.Node->RemoveChild(*sibling2.Value()).HasException());
    }
  }

  TEST_CASE("NodeIterator traversal: nothing precedes the root, even when it has a previous sibling",
            "[HTML][NodeIterator][dom:concept-nodeiterator-traverse]")
  {
    CommonTestData data;
    auto parent = CreateRef<TestElement>(*data.Document);
    auto sibling = CreateRef<TestElement>(*data.Document);
    REQUIRE_FALSE(parent->AppendChild(*sibling).HasException());
    REQUIRE_FALSE(parent->AppendChild(*data.Node).HasException());

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    auto previousNodeResult = nodeIterator->PreviousNode();
    REQUIRE_FALSE(previousNodeResult.HasException());
    CHECK(previousNodeResult.Value() == nullptr);
    CHECK(&nodeIterator->ReferenceNode() == data.Node.get());
  }

  TEST_CASE("NodeIterator::Filter", "[HTML][NodeIterator]")
  {
    CommonTestData data;
    auto sibling1 = data.Document->CreateElement(u8"sibling1");
    REQUIRE_FALSE(sibling1.HasException());

    auto sibling2 = data.Document->CreateTextNode(u8"sibling2");
    REQUIRE_FALSE(data.Node->AppendChild(*sibling1.Value()).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*sibling2).HasException());

    auto nodeIterator =
      data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, CreateRefPtr<TextNodeFilter>());

    auto nextNodeResult = nodeIterator->NextNode();
    REQUIRE_FALSE(nextNodeResult.HasException());
    REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(nextNodeResult.Value() == sibling2);

    nextNodeResult = nodeIterator->NextNode();
    REQUIRE_FALSE(nextNodeResult.HasException());
    REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(nextNodeResult.Value() == nullptr);

    auto previousNodeResult = nodeIterator->PreviousNode();
    REQUIRE_FALSE(previousNodeResult.HasException());
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(previousNodeResult.Value() == sibling2);

    previousNodeResult = nodeIterator->PreviousNode();
    REQUIRE_FALSE(previousNodeResult.HasException());
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(previousNodeResult.Value() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*sibling1.Value()).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*sibling2).HasException());
  }

  TEST_CASE("NodeIterator::WhatToShow", "[HTML][NodeIterator]")
  {
    CommonTestData data;
    auto sibling1 = data.Document->CreateElement(u8"sibling1");
    REQUIRE_FALSE(sibling1.HasException());

    auto sibling2 = data.Document->CreateTextNode(u8"sibling2");
    REQUIRE_FALSE(data.Node->AppendChild(*sibling1.Value()).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*sibling2).HasException());

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_TEXT);

    auto nextNodeResult = nodeIterator->NextNode();
    REQUIRE_FALSE(nextNodeResult.HasException());
    REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(nextNodeResult.Value() == sibling2);

    nextNodeResult = nodeIterator->NextNode();
    REQUIRE_FALSE(nextNodeResult.HasException());
    REQUIRE_FALSE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(nextNodeResult.Value() == nullptr);

    auto previousNodeResult = nodeIterator->PreviousNode();
    REQUIRE_FALSE(previousNodeResult.HasException());
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(previousNodeResult.Value() == sibling2);

    previousNodeResult = nodeIterator->PreviousNode();
    REQUIRE_FALSE(previousNodeResult.HasException());
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
    REQUIRE(previousNodeResult.Value() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*sibling1.Value()).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*sibling2).HasException());
  }

  TEST_CASE("NodeIterator pre-remove steps: run for every iterator of the document, following its root",
            "[DOM][NodeIterator][dom:nodeiterator-pre-removing-steps]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    auto first = document->CreateElement(u8"span").ReleaseValue();
    auto second = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(root->AppendChild(*first).HasException());
    REQUIRE_FALSE(root->AppendChild(*second).HasException());
    auto iterator = document->CreateNodeIterator(*root);

    SECTION("With the pointer before the reference, it moves to the next node that isn't removed")
    {
      REQUIRE(iterator->NextNode().Value().get() == root.get());
      REQUIRE(iterator->NextNode().Value().get() == first.get());
      REQUIRE(iterator->PreviousNode().Value().get() == first.get());
      REQUIRE(iterator->PointerBeforeReferenceNode());

      REQUIRE_FALSE(first->Remove().HasException());
      CHECK(&iterator->ReferenceNode() == second.get());
      CHECK(iterator->PointerBeforeReferenceNode());
      CHECK(iterator->NextNode().Value().get() == second.get());
    }

    SECTION("After its root is adopted into another document")
    {
      auto other = CreateRef<HTMLDocument>();
      REQUIRE_FALSE(other->AdoptNode(*root).HasException());
      REQUIRE(iterator->NextNode().Value().get() == root.get());
      REQUIRE(iterator->NextNode().Value().get() == first.get());

      REQUIRE_FALSE(first->Remove().HasException());
      CHECK(&iterator->ReferenceNode() == root.get());
      CHECK(iterator->NextNode().Value().get() == second.get());
    }
  }
}
