#include "Krystal.Booey/DOM/Traversal/TreeWalker.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Traversal/NodeFilter.hpp"

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

  TEST_CASE(
    "TreeWalker.root and currentNode: its root, where it starts",
    "[DOM][TreeWalker][dom:interface-treewalker][dom:dom-treewalker-root][dom:dom-treewalker-currentnode]")
  {
    CommonTestData data;
    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&treeWalker->Root() == data.Node);
    REQUIRE(&treeWalker->CurrentNode() == data.Node);
  }

  TEST_CASE("TreeWalker: moving to children, siblings and parents with no filter",
            "[DOM][TreeWalker][dom:dom-treewalker-firstchild][dom:dom-treewalker-lastchild][dom:dom-"
            "treewalker-nextsibling]"
            "[dom:dom-treewalker-previoussibling][dom:dom-treewalker-parentnode]")
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

  TEST_CASE("TreeWalker traversal: only nodes its filter accepts are returned",
            "[DOM][TreeWalker][dom:dom-treewalker-filter][dom:concept-traverse-children][dom:concept-"
            "traverse-siblings]")
  {
    CommonTestData data;

    auto textNode = CreateRef<krys::boo::dom::Text>(*data.Document, u8"Hello, World!");

    REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

    auto filter = CreateRef<TextNodeFilter>();
    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ALL, filter);
    CHECK(treeWalker->Filter() == filter.get());

    auto firstChild = treeWalker->FirstChild();
    REQUIRE(firstChild.HasValue());
    REQUIRE(firstChild.Value() == textNode);

    auto nextSibling = treeWalker->NextSibling();
    REQUIRE(nextSibling.HasValue());
    REQUIRE(nextSibling.Value() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*textNode).HasException());
  }

  TEST_CASE("TreeWalker traversal: only nodes its whatToShow shows are returned",
            "[DOM][TreeWalker][dom:dom-treewalker-whattoshow][dom:concept-traverse-children]")
  {
    CommonTestData data;

    auto elementNode = CreateRef<TestElement>(*data.Document);
    auto textNode = CreateRef<krys::boo::dom::Text>(*data.Document, u8"Hello, World!");

    REQUIRE_FALSE(data.Node->AppendChild(*elementNode).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*textNode).HasException());

    auto treeWalker = data.Document->CreateTreeWalker(*data.Node, WhatToShow::SHOW_ELEMENT, nullptr);
    CHECK(treeWalker->WhatToShow() == WhatToShow::SHOW_ELEMENT);

    auto firstChild = treeWalker->FirstChild();
    REQUIRE(firstChild.HasValue());
    REQUIRE(firstChild.Value() == elementNode);

    auto nextSibling = treeWalker->NextSibling();
    REQUIRE(nextSibling.HasValue());
    REQUIRE(nextSibling.Value() == nullptr);

    REQUIRE_FALSE(data.Node->RemoveChild(*elementNode).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*textNode).HasException());
  }

  namespace
  {
    /// @brief div#d > (b#b > "bb", i#i > (em#em > "e", "ii"), u#u), in a body.
    struct WalkTestData
    {
      Ref<dom::Document> Document;
      Ref<Element> Div;
      Ref<Element> B;
      Ref<Element> I;
      Ref<Element> Em;
      Ref<Element> U;
      Node *BText;
      Node *EmText;
      Node *IText;

      WalkTestData()
          : Document(dom::Document::ParseHTMLUnsafe(
                       *CreateRef<HTMLDocument>(),
                       u8"<!DOCTYPE html><body><div id=d><b id=b>bb</b><i id=i><em id=em>e</em>"
                       u8"ii</i><u id=u></u></div>")
                       .ReleaseValue()),
            Div(ShareRef(*Document->GetElementById(u8"d"))), B(ShareRef(*Document->GetElementById(u8"b"))),
            I(ShareRef(*Document->GetElementById(u8"i"))), Em(ShareRef(*Document->GetElementById(u8"em"))),
            U(ShareRef(*Document->GetElementById(u8"u"))), BText(B->FirstChild()), EmText(Em->FirstChild()),
            IText(I->LastChild())
      {
      }
    };

    /// @brief Returns what its function returns for each node.
    class FunctionNodeFilter : public NodeFilter
    {
    public:
      Func<FilterResult(const Node &)> Function;

      explicit FunctionNodeFilter(Func<FilterResult(const Node &)> function) noexcept
          : Function(krys::move(function))
      {
      }

      KRYS_NODISCARD dom::ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept override
      {
        return Function(node);
      }
    };

    Ref<TreeWalker> CreateWalker(WalkTestData &data, Node &root, Func<FilterResult(const Node &)> function)
    {
      return data.Document->CreateTreeWalker(root, WhatToShow::SHOW_ALL,
                                             CreateRefPtr<FunctionNodeFilter>(krys::move(function)));
    }
  }

  TEST_CASE("TreeWalker.currentNode: setting it moves the walker to any node, even outside its root",
            "[DOM][TreeWalker][dom:dom-treewalker-currentnode][dom:treewalker-current]")
  {
    WalkTestData data;
    auto walker = data.Document->CreateTreeWalker(*data.I);
    walker->CurrentNode(*data.B);
    CHECK(&walker->CurrentNode() == data.B.get());
    CHECK(walker->NextNode().ReleaseValue() == data.BText);
  }

  TEST_CASE("TreeWalker.parentNode(): the closest ancestor the filter accepts, up to its root",
            "[DOM][TreeWalker][dom:dom-treewalker-parentnode]")
  {
    WalkTestData data;
    auto walker = CreateWalker(
      data, *data.Div, [&](const Node &node)
      { return &node == data.Em.get() ? FilterResult::FILTER_REJECT : FilterResult::FILTER_ACCEPT; });
    walker->CurrentNode(*data.EmText);
    CHECK(walker->ParentNode().ReleaseValue() == data.I.get());
    CHECK(walker->ParentNode().ReleaseValue() == data.Div.get());
    CHECK(walker->ParentNode().ReleaseValue() == nullptr);
    CHECK(&walker->CurrentNode() == data.Div.get());

    // Not past its root, even when the filter rejects it.
    auto rejectingRoot = CreateWalker(
      data, *data.I, [&](const Node &node)
      { return &node == data.I.get() ? FilterResult::FILTER_REJECT : FilterResult::FILTER_ACCEPT; });
    rejectingRoot->CurrentNode(*data.Em);
    CHECK(rejectingRoot->ParentNode().ReleaseValue() == nullptr);
    CHECK(&rejectingRoot->CurrentNode() == data.Em.get());
  }

  TEST_CASE(
    "TreeWalker.firstChild() and lastChild(): skipped nodes' children are searched, rejected ones' aren't",
    "[DOM][TreeWalker][dom:concept-traverse-children][dom:dom-treewalker-firstchild]"
    "[dom:dom-treewalker-lastchild]")
  {
    WalkTestData data;

    SECTION("firstChild()")
    {
      auto walker =
        CreateWalker(data, *data.Div,
                     [&](const Node &node)
                     {
                       if (&node == data.B.get())
                       {
                         return FilterResult::FILTER_REJECT;
                       }
                       return &node == data.I.get() ? FilterResult::FILTER_SKIP : FilterResult::FILTER_ACCEPT;
                     });
      CHECK(walker->FirstChild().ReleaseValue() == data.Em.get());
      CHECK(&walker->CurrentNode() == data.Em.get());
    }

    SECTION("lastChild()")
    {
      auto walker = CreateWalker(data, *data.Div,
                                 [](const Node &node)
                                 {
                                   return (node.NodeType() == NodeType::ELEMENT_NODE)
                                            ? FilterResult::FILTER_SKIP
                                            : FilterResult::FILTER_ACCEPT;
                                 });
      CHECK(walker->LastChild().ReleaseValue() == data.IText);
    }

    SECTION("Nothing accepted: it stops at the current node, leaving it unchanged")
    {
      auto walker = CreateWalker(data, *data.Div,
                                 [](const Node &node)
                                 {
                                   return (node.NodeType() == NodeType::ELEMENT_NODE)
                                            ? FilterResult::FILTER_SKIP
                                            : FilterResult::FILTER_REJECT;
                                 });
      walker->CurrentNode(*data.I);
      CHECK(walker->FirstChild().ReleaseValue() == nullptr);
      CHECK(&walker->CurrentNode() == data.I.get());
    }
  }

  TEST_CASE(
    "TreeWalker.nextSibling() and previousSibling(): into skipped siblings, and out of skipped parents",
    "[DOM][TreeWalker][dom:concept-traverse-siblings][dom:dom-treewalker-nextsibling]"
    "[dom:dom-treewalker-previoussibling]")
  {
    WalkTestData data;
    FilterResult forI = FilterResult::FILTER_SKIP;
    auto walker = CreateWalker(data, *data.Div,
                               [&](const Node &node)
                               {
                                 if (&node == data.I.get())
                                 {
                                   return forI;
                                 }
                                 return (node.NodeType() == NodeType::ELEMENT_NODE)
                                          ? FilterResult::FILTER_ACCEPT
                                          : FilterResult::FILTER_REJECT;
                               });

    SECTION("A skipped sibling's children are its siblings")
    {
      walker->CurrentNode(*data.B);
      CHECK(walker->NextSibling().ReleaseValue() == data.Em.get());
      walker->CurrentNode(*data.U);
      CHECK(walker->PreviousSibling().ReleaseValue() == data.Em.get());
    }

    SECTION("A rejected sibling's aren't")
    {
      forI = FilterResult::FILTER_REJECT;
      walker->CurrentNode(*data.B);
      CHECK(walker->NextSibling().ReleaseValue() == data.U.get());
    }

    SECTION("Past the last child of a skipped parent, its parent's siblings")
    {
      walker->CurrentNode(*data.Em);
      CHECK(walker->NextSibling().ReleaseValue() == data.U.get());
    }

    SECTION("But not of an accepted parent")
    {
      forI = FilterResult::FILTER_ACCEPT;
      walker->CurrentNode(*data.Em);
      CHECK(walker->NextSibling().ReleaseValue() == nullptr);
      CHECK(&walker->CurrentNode() == data.Em.get());
    }

    SECTION("The root has none")
    {
      CHECK(walker->NextSibling().ReleaseValue() == nullptr);
    }
  }

  TEST_CASE("TreeWalker.previousNode() and nextNode(): in tree order within its root",
            "[DOM][TreeWalker][dom:dom-treewalker-previousnode][dom:dom-treewalker-nextnode]")
  {
    WalkTestData data;
    auto walker = data.Document->CreateTreeWalker(*data.Div);
    List<Node *> order {data.B.get(), data.BText, data.I.get(), data.Em.get(),
                        data.EmText,  data.IText, data.U.get()};
    for (auto *node : order)
    {
      CHECK(walker->NextNode().ReleaseValue() == node);
    }
    CHECK(walker->NextNode().ReleaseValue() == nullptr);

    for (auto it = order.rbegin() + 1; it != order.rend(); ++it)
    {
      CHECK(walker->PreviousNode().ReleaseValue() == *it);
    }
    CHECK(walker->PreviousNode().ReleaseValue() == data.Div.get());
    CHECK(walker->PreviousNode().ReleaseValue() == nullptr);

    SECTION("Rejected nodes' descendants are passed over")
    {
      auto rejecting = CreateWalker(
        data, *data.Div, [&](const Node &node)
        { return &node == data.I.get() ? FilterResult::FILTER_REJECT : FilterResult::FILTER_ACCEPT; });
      rejecting->CurrentNode(*data.BText);
      CHECK(rejecting->NextNode().ReleaseValue() == data.U.get());
      CHECK(rejecting->PreviousNode().ReleaseValue() == data.BText);
    }

    SECTION("Skipped nodes' descendants aren't")
    {
      auto skipping = CreateWalker(
        data, *data.Div, [&](const Node &node)
        { return &node == data.I.get() ? FilterResult::FILTER_SKIP : FilterResult::FILTER_ACCEPT; });
      skipping->CurrentNode(*data.BText);
      CHECK(skipping->NextNode().ReleaseValue() == data.Em.get());
      skipping->CurrentNode(*data.Em);
      CHECK(skipping->PreviousNode().ReleaseValue() == data.BText);
    }
  }

  TEST_CASE("TreeWalker.nextNode(): from a node outside its root with nothing after it, null",
            "[DOM][TreeWalker][dom:dom-treewalker-nextnode]")
  {
    WalkTestData data;
    auto walker = data.Document->CreateTreeWalker(*data.B);
    // <u> has no children and neither it nor its ancestors have a next sibling, and <b> isn't one of them.
    walker->CurrentNode(*data.U);
    CHECK(walker->NextNode().ReleaseValue() == nullptr);
    CHECK(&walker->CurrentNode() == data.U.get());
  }
}