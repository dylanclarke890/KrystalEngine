#include "Krystal.Booey/DOM/Traversal/NodeIterator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Traversal/NodeFilter.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

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

  TEST_CASE("NodeIterator.root: its root",
            "[DOM][NodeIterator][dom:interface-nodeiterator][dom:dom-nodeiterator-root]")
  {
    CommonTestData data;

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&nodeIterator->Root() == data.Node);
  }

  TEST_CASE(
    "NodeIterator.referenceNode and pointerBeforeReferenceNode: its reference, initially before its root",
    "[DOM][NodeIterator][dom:dom-nodeiterator-referencenode][dom:dom-nodeiterator-pointerbeforereferencenode]"
    "[dom:nodeiterator-reference][dom:node-pointer][dom:node-pointer-node][dom:node-pointer-pointer-before]")
  {
    CommonTestData data;

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, nullptr);
    REQUIRE(&nodeIterator->ReferenceNode() == data.Node);
    REQUIRE(nodeIterator->PointerBeforeReferenceNode());
  }

  TEST_CASE("NodeIterator.nextNode() and previousNode(): traverse its iterator collection in tree order",
            "[DOM][NodeIterator][dom:dom-nodeiterator-nextnode][dom:dom-nodeiterator-previousnode]"
            "[dom:concept-nodeiterator-traverse][dom:iterator-collection]")
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
            "[DOM][NodeIterator][dom:concept-nodeiterator-traverse]")
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

  TEST_CASE("NodeIterator traversal: only nodes its filter accepts are returned",
            "[DOM][NodeIterator][dom:concept-nodeiterator-traverse][dom:dom-nodeiterator-filter]")
  {
    CommonTestData data;
    auto sibling1 = data.Document->CreateElement(u8"sibling1");
    REQUIRE_FALSE(sibling1.HasException());

    auto sibling2 = data.Document->CreateTextNode(u8"sibling2");
    REQUIRE_FALSE(data.Node->AppendChild(*sibling1.Value()).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*sibling2).HasException());

    auto nodeIterator =
      data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_ALL, CreateRefPtr<TextNodeFilter>());
    CHECK(dynamic_cast<TextNodeFilter *>(nodeIterator->Filter()) != nullptr);

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

  TEST_CASE("NodeIterator traversal: only nodes its whatToShow shows are returned",
            "[DOM][NodeIterator][dom:concept-nodeiterator-traverse][dom:dom-nodeiterator-whattoshow]")
  {
    CommonTestData data;
    auto sibling1 = data.Document->CreateElement(u8"sibling1");
    REQUIRE_FALSE(sibling1.HasException());

    auto sibling2 = data.Document->CreateTextNode(u8"sibling2");
    REQUIRE_FALSE(data.Node->AppendChild(*sibling1.Value()).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*sibling2).HasException());

    auto nodeIterator = data.Document->CreateNodeIterator(*data.Node, WhatToShow::SHOW_TEXT);
    CHECK(nodeIterator->WhatToShow() == WhatToShow::SHOW_TEXT);

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

  TEST_CASE(
    "NodeIterator pre-remove steps: run for every iterator of the document, following its root",
    "[DOM][NodeIterator][dom:nodeiterator-pre-removing-steps][dom:nodeiterator-adjust-a-node-pointer]")
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

  namespace
  {
    /// @brief body > div#d > (b#b > "bb", i#i > (em#em, "ii"), u#u).
    struct AdjustTestData
    {
      Ref<dom::Document> Document;
      Ref<Element> Body;
      Ref<Element> Div;
      Ref<Element> B;
      Ref<Element> I;
      Ref<Element> Em;
      Ref<Text> IText;
      Ref<Element> U;

      AdjustTestData()
          : Document(dom::Document::ParseHTMLUnsafe(
                       *CreateRef<HTMLDocument>(),
                       u8"<!DOCTYPE html><body><div id=d><b id=b>bb</b><i id=i><em id=em></em>"
                       u8"ii</i><u id=u></u></div>")
                       .ReleaseValue()),
            Body(ShareRef(*Document->Body())), Div(ShareRef(*Document->GetElementById(u8"d"))),
            B(ShareRef(*Document->GetElementById(u8"b"))), I(ShareRef(*Document->GetElementById(u8"i"))),
            Em(ShareRef(*Document->GetElementById(u8"em"))), IText(ShareRef(Downcast<Text>(*I->LastChild()))),
            U(ShareRef(*Document->GetElementById(u8"u")))
      {
      }
    };

    /// @brief Calls a function on each node it's given, then returns a fixed result.
    class CallbackNodeFilter : public NodeFilter
    {
    public:
      Func<void(const Node &)> Callback;
      FilterResult Result {FilterResult::FILTER_SKIP};

      KRYS_NODISCARD dom::ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept override
      {
        Callback(node);
        return Result;
      }
    };

    /// @brief Moves the iterator forward until its reference is the node, after it.
    void AdvanceTo(NodeIterator &iterator, const Node &node)
    {
      while (&iterator.ReferenceNode() != &node || iterator.PointerBeforeReferenceNode())
      {
        REQUIRE(iterator.NextNode().ReleaseValue() != nullptr);
      }
    }
  }

  TEST_CASE("Adjust a node pointer: nothing changes when the removed node contains the root",
            "[DOM][NodeIterator][dom:nodeiterator-adjust-a-node-pointer]")
  {
    AdjustTestData data;
    auto iterator = data.Document->CreateNodeIterator(*data.I);
    AdvanceTo(*iterator, *data.Em);

    SECTION("The root's ancestor")
    {
      REQUIRE_FALSE(data.Body->RemoveChild(*data.Div).HasException());
      CHECK(&iterator->ReferenceNode() == data.Em.get());
      CHECK_FALSE(iterator->PointerBeforeReferenceNode());
      CHECK(iterator->NextNode().ReleaseValue() == data.IText.get());
    }

    SECTION("The root")
    {
      REQUIRE_FALSE(data.Div->RemoveChild(*data.I).HasException());
      CHECK(&iterator->ReferenceNode() == data.Em.get());
      CHECK_FALSE(iterator->PointerBeforeReferenceNode());
    }

    SECTION("A node that doesn't contain its reference")
    {
      REQUIRE_FALSE(data.I->RemoveChild(*data.IText).HasException());
      CHECK(&iterator->ReferenceNode() == data.Em.get());
    }
  }

  TEST_CASE("Adjust a node pointer: a pointer after a removed node moves after the node before it",
            "[DOM][NodeIterator][dom:nodeiterator-adjust-a-node-pointer]")
  {
    AdjustTestData data;
    auto iterator = data.Document->CreateNodeIterator(*data.Div);

    SECTION("Its previous sibling's last inclusive descendant")
    {
      AdvanceTo(*iterator, *data.IText);
      REQUIRE_FALSE(data.Div->RemoveChild(*data.I).HasException());
      CHECK(&iterator->ReferenceNode() == data.B->FirstChild());
      CHECK_FALSE(iterator->PointerBeforeReferenceNode());
      CHECK(iterator->NextNode().ReleaseValue() == data.U.get());
    }

    SECTION("Its parent, when it has no previous sibling")
    {
      AdvanceTo(*iterator, *data.Em);
      REQUIRE_FALSE(data.I->RemoveChild(*data.Em).HasException());
      CHECK(&iterator->ReferenceNode() == data.I.get());
      CHECK_FALSE(iterator->PointerBeforeReferenceNode());
    }
  }

  TEST_CASE("Adjust a node pointer: a pointer before a removed node moves before the next node in the root",
            "[DOM][NodeIterator][dom:nodeiterator-adjust-a-node-pointer]")
  {
    AdjustTestData data;
    auto iterator = data.Document->CreateNodeIterator(*data.Div);
    AdvanceTo(*iterator, *data.Em);
    REQUIRE(iterator->PreviousNode().ReleaseValue() == data.Em.get());
    REQUIRE(iterator->PointerBeforeReferenceNode());

    SECTION("The node after it and its descendants")
    {
      REQUIRE_FALSE(data.Div->RemoveChild(*data.I).HasException());
      CHECK(&iterator->ReferenceNode() == data.U.get());
      CHECK(iterator->PointerBeforeReferenceNode());
    }

    SECTION("Or, with no node after it in the root, after the node before it")
    {
      REQUIRE_FALSE(data.Div->RemoveChild(*data.U).HasException());
      REQUIRE_FALSE(data.Div->RemoveChild(*data.I).HasException());
      CHECK(&iterator->ReferenceNode() == data.B->FirstChild());
      CHECK_FALSE(iterator->PointerBeforeReferenceNode());
    }
  }

  TEST_CASE(
    "NodeIterator traversal: its candidate reference moves when the filter removes the node it's given",
    "[DOM][NodeIterator][dom:concept-nodeiterator-traverse][dom:nodeiterator-candidate-reference]"
    "[dom:nodeiterator-pre-removing-steps]")
  {
    AdjustTestData data;
    auto filter = CreateRef<CallbackNodeFilter>();
    auto iterator = data.Document->CreateNodeIterator(*data.Div, WhatToShow::SHOW_ALL, filter);
    List<const Node *> seen;
    filter->Callback = [&](const Node &node)
    {
      seen.push_back(&node);
      if (&node == data.I.get())
      {
        REQUIRE_FALSE(data.Div->RemoveChild(*data.I).HasException());
      }
    };

    SECTION("Skipped, the traversal goes on from where it was, not from inside the removed node")
    {
      CHECK(iterator->NextNode().ReleaseValue() == nullptr);
      CHECK(seen
            == List<const Node *> {data.Div.get(), data.B.get(), data.B->FirstChild(), data.I.get(),
                                   data.U.get()});
    }

    SECTION("Accepted, it becomes the reference")
    {
      filter->Result = FilterResult::FILTER_ACCEPT;
      AdvanceTo(*iterator, *data.B->FirstChild());
      CHECK(iterator->NextNode().ReleaseValue() == data.I.get());
      CHECK(&iterator->ReferenceNode() == data.B->FirstChild());
      CHECK_FALSE(iterator->PointerBeforeReferenceNode());
      CHECK(iterator->NextNode().ReleaseValue() == data.U.get());
    }
  }

  TEST_CASE("NodeIterator traversal: from inside its own filter it throws, even at the end of its collection",
            "[DOM][NodeIterator][dom:concept-nodeiterator-traverse]")
  {
    AdjustTestData data;
    auto filter = CreateRef<CallbackNodeFilter>();
    filter->Result = FilterResult::FILTER_ACCEPT;
    auto iterator = data.Document->CreateNodeIterator(*data.U, WhatToShow::SHOW_ALL, filter);
    filter->Callback = [](const Node &) {};
    REQUIRE(iterator->NextNode().ReleaseValue() == data.U.get());

    // Its reference is after its last node, so a traversal forward would find nothing to filter.
    Maybe<ExceptionCode> reentry;
    filter->Callback = [&](const Node &)
    {
      auto result = iterator->NextNode();
      reentry = result.HasException() ? Maybe<ExceptionCode> {result.GetException().Code()} : null;
    };
    CHECK(iterator->PreviousNode().ReleaseValue() == data.U.get());
    CHECK(reentry == ExceptionCode::InvalidStateError);
    CHECK(&iterator->ReferenceNode() == data.U.get());
    CHECK(iterator->PointerBeforeReferenceNode());
  }
}
