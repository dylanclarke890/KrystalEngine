#include "Krystal.HTML/Iterator/TreeWalker.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Iterator/NodeFilter.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
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

  TEST_CASE("TreeWalker::Root", "[HTML][TreeWalker]")
  {
    CommonTestData data;
    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&treeWalker->Root() == data.Node);
    REQUIRE(&treeWalker->CurrentNode() == data.Node);
  }

  TEST_CASE("TreeWalker::Basic operations", "[HTML][TreeWalker]")
  {
    CommonTestData data;
    auto child1 = CreateRef<TestElement>(*data.Document);
    auto child2 = CreateRef<TestElement>(*data.Document);

    REQUIRE_FALSE(data.Node->AppendChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child2).HasException());

    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&treeWalker->CurrentNode() == data.Node);

    auto firstChild = treeWalker->FirstChild();
    REQUIRE(firstChild.HasValue());
    REQUIRE(firstChild.Value() == child1);

    REQUIRE(&treeWalker->CurrentNode() == child1);

    auto nextSibling = treeWalker->NextSibling();
    REQUIRE(nextSibling.HasValue());
    REQUIRE(nextSibling.Value() == child2);

    auto previousSibling = treeWalker->PreviousSibling();
    REQUIRE(previousSibling.HasValue());
    REQUIRE(previousSibling.Value() == child1);

    auto parentNode = treeWalker->ParentNode();
    REQUIRE(parentNode.HasValue());
    REQUIRE(parentNode.Value() == data.Node);

    auto lastChild = treeWalker->LastChild();
    REQUIRE(lastChild.HasValue());
    REQUIRE(lastChild.Value() == child2);

    REQUIRE_FALSE(data.Node->RemoveChild(*child1).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*child2).HasException());
  }

  TEST_CASE("TreeWalker::Filter", "[HTML][TreeWalker]")
  {
    CommonTestData data;

    auto textNode = CreateRef<HTML::Text>(*data.Document, u8"Hello, World!");

    REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

    auto filter = CreateRef<TextNodeFilter>();
    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ALL, filter);

    auto firstChild = treeWalker->FirstChild();
    REQUIRE(firstChild.HasValue());
    REQUIRE(firstChild.Value() == textNode);

    auto nextSibling = treeWalker->NextSibling();
    REQUIRE(nextSibling.HasValue());
    REQUIRE(nextSibling.Value() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*textNode).HasException());
  }

  TEST_CASE("TreeWalker::WhatToShow", "[HTML][TreeWalker]")
  {
    CommonTestData data;

    auto elementNode = CreateRef<TestElement>(*data.Document);
    auto textNode = CreateRef<HTML::Text>(*data.Document, u8"Hello, World!");

    REQUIRE_FALSE(data.Node->AppendChild(*elementNode).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ELEMENT, nullptr);

    auto firstChild = treeWalker->FirstChild();
    REQUIRE(firstChild.HasValue());
    REQUIRE(firstChild.Value() == elementNode);

    auto nextSibling = treeWalker->NextSibling();
    REQUIRE(nextSibling.HasValue());
    REQUIRE(nextSibling.Value() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*elementNode).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*textNode).HasException());
  }
}