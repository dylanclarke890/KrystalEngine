#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Iterator/NodeFilter.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/Node.hpp"
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
      Ref<TestElement> Node;

      CommonTestData() : Document(CreateRef<HTMLDocument>()), Node(CreateRef<TestElement>(*Document))
      {
      }
    };

    class TextNodeFilter : public NodeFilter
    {
    public:
      KRYS_NODISCARD ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept override
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
}