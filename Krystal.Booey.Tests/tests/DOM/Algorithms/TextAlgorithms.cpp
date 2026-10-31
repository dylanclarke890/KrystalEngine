#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include <limits>

namespace krys::boo::dom::tests
{
  namespace
  {
    struct ReplaceTestData
    {
      Ref<dom::Document> Document;
      Ref<Text> Node;

      ReplaceTestData()
          : Document(CreateRef<dom::Document>()), Node(CreateRef<Text>(*Document, u8"0123456789"))
      {
      }

      /// @brief A collapsed live range at the offset in the node.
      Ref<Range> RangeAt(size_t offset) noexcept
      {
        auto range = Document->CreateRange();
        REQUIRE_FALSE(range->SetStart(*Node, offset).HasException());
        REQUIRE_FALSE(range->SetEnd(*Node, offset).HasException());
        return range;
      }
    };
  }

  TEST_CASE("Replace data: inserts the data at the offset and removes count code units after it",
            "[DOM][TextAlgorithms][dom:concept-cd-replace][dom:concept-cd-data]")
  {
    ReplaceTestData data;

    SECTION("Inside the data")
    {
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 2, 3, u8"ab").HasException());
      CHECK(data.Node->Data() == u8"01ab56789");
    }

    SECTION("A count past the end is clamped to the end")
    {
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 8, 5, u8"x").HasException());
      CHECK(data.Node->Data() == u8"01234567x");
    }

    SECTION("At the end")
    {
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 10, 0, u8"x").HasException());
      CHECK(data.Node->Data() == u8"0123456789x");
    }

    SECTION("An offset past the end throws an IndexSizeError, changing nothing")
    {
      auto result = TextAlgorithms::Replace(*data.Node, 11, 0, u8"x");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::IndexSizeError);
      CHECK(data.Node->Data() == u8"0123456789");
    }
  }

  TEST_CASE("Replace data: a count that overflows offset + count is clamped too",
            "[DOM][TextAlgorithms][dom:concept-cd-replace]")
  {
    ReplaceTestData data;
    auto range = data.RangeAt(5);

    REQUIRE_FALSE(
      TextAlgorithms::Replace(*data.Node, 1, std::numeric_limits<size_t>::max(), u8"").HasException());

    CHECK(data.Node->Data() == u8"0");
    CHECK(range->StartOffset() == 1uz);
    CHECK(range->EndOffset() == 1uz);
  }

  TEST_CASE("Replace data: queues a characterData record with the old data",
            "[DOM][TextAlgorithms][dom:concept-cd-replace]")
  {
    ReplaceTestData data;
    auto observer = CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    REQUIRE_FALSE(
      observer
        ->Observe(*data.Node, MutationObserverInit {.CharacterData = true, .CharacterDataOldValue = true})
        .HasException());

    REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 0, 1, u8"x").HasException());
    auto failed = TextAlgorithms::Replace(*data.Node, 20, 0, u8"y");
    REQUIRE(failed.HasException());

    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->Type() == u8"characterData");
    CHECK(records[0]->Target().get() == data.Node.get());
    CHECK(records[0]->OldValue() == u8"0123456789");
  }

  TEST_CASE("Replace data: moves live range boundaries in the node",
            "[DOM][TextAlgorithms][dom:concept-cd-replace]")
  {
    ReplaceTestData data;
    auto before = data.RangeAt(2);
    auto atOffset = data.RangeAt(3);
    auto inside = data.RangeAt(4);
    auto atEnd = data.RangeAt(6);
    auto after = data.RangeAt(8);

    // Replaces "345" with "abcdef".
    REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 3, 3, u8"abcdef").HasException());

    CHECK(before->StartOffset() == 2uz);
    CHECK(atOffset->StartOffset() == 3uz);
    // Greater than offset, up to offset + count: moved to the offset.
    CHECK(inside->StartOffset() == 3uz);
    CHECK(inside->EndOffset() == 3uz);
    CHECK(atEnd->StartOffset() == 3uz);
    // Past offset + count: shifted by the data's length less count.
    CHECK(after->StartOffset() == 11uz);
    CHECK(after->EndOffset() == 11uz);

    SECTION("Each boundary is moved on its own")
    {
      auto range = data.Document->CreateRange();
      REQUIRE_FALSE(range->SetStart(*data.Node, 1).HasException());
      REQUIRE_FALSE(range->SetEnd(*data.Node, 12).HasException());
      REQUIRE_FALSE(TextAlgorithms::Replace(*data.Node, 0, 2, u8"").HasException());
      CHECK(range->StartOffset() == 0uz);
      CHECK(range->EndOffset() == 10uz);
    }
  }

  TEST_CASE("Substring data: count code units from the offset, to the end at most",
            "[DOM][TextAlgorithms][dom:concept-cd-substring]")
  {
    ReplaceTestData data;
    CHECK(TextAlgorithms::Substring(*data.Node, 2, 3).Value() == u8"234");
    CHECK(TextAlgorithms::Substring(*data.Node, 7, 5).Value() == u8"789");
    CHECK(TextAlgorithms::Substring(*data.Node, 1, std::numeric_limits<size_t>::max()).Value()
          == u8"123456789");
    CHECK(TextAlgorithms::Substring(*data.Node, 10, 1).Value() == u8"");

    auto result = TextAlgorithms::Substring(*data.Node, 11, 0);
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::IndexSizeError);
  }

  namespace
  {
    /// @brief A parent of Text and CDATASection nodes, in an XML document so CDATA sections can be made.
    struct ContiguousTestData
    {
      Ref<XMLDocument> Document;
      Ref<Element> Parent;

      ContiguousTestData()
          : Document(CreateRef<XMLDocument>()),
            Parent(Document->CreateElementNS(u8"urn:x", u8"p").ReleaseValue())
      {
      }

      template <typename TNode>
      Ref<TNode> Append(Ref<TNode> node) noexcept
      {
        REQUIRE_FALSE(Parent->AppendChild(*node).HasException());
        return node;
      }

      Ref<Text> AppendText(DOMString &&data) noexcept
      {
        return Append(Document->CreateTextNode(krys::move(data)));
      }

      Ref<CDATASection> AppendCDATA(DOMString &&data) noexcept
      {
        return Append(Document->CreateCDATASection(krys::move(data)).ReleaseValue());
      }
    };

    template <typename TList>
    DOMString Joined(const TList &nodes)
    {
      DOMString result;
      for (auto &node : nodes)
      {
        result += node->Data();
      }
      return result;
    }
  }

  TEST_CASE("Contiguous Text nodes: the node and its Text node siblings on either side, in tree order",
            "[DOM][TextAlgorithms][dom:contiguous-text-nodes][dom:contiguous-exclusive-text-nodes]")
  {
    ContiguousTestData data;
    auto first = data.AppendText(u8"a");
    data.Append(data.Document->CreateComment(u8"stop"));
    auto second = data.AppendText(u8"b");
    auto cdata = data.AppendCDATA(u8"c");
    auto third = data.AppendText(u8"d");
    data.Append(data.Document->CreateElementNS(u8"urn:x", u8"stop").ReleaseValue());
    auto last = data.AppendText(u8"e");

    CHECK(Joined(TextAlgorithms::ContiguousTextNodes(*first)) == u8"a");
    CHECK(Joined(TextAlgorithms::ContiguousTextNodes(*second)) == u8"bcd");
    CHECK(Joined(TextAlgorithms::ContiguousTextNodes(*cdata)) == u8"bcd");
    CHECK(Joined(TextAlgorithms::ContiguousTextNodes(*third)) == u8"bcd");
    CHECK(Joined(TextAlgorithms::ContiguousTextNodes(*last)) == u8"e");

    // A CDATASection isn't an exclusive Text node, so it splits them.
    CHECK(Joined(TextAlgorithms::ContiguousExclusiveTextNodes(*second)) == u8"b");
    CHECK(Joined(TextAlgorithms::ContiguousExclusiveTextNodes(*third)) == u8"d");

    SECTION("A node without a parent is alone")
    {
      auto orphan = data.Document->CreateTextNode(u8"x");
      CHECK(Joined(TextAlgorithms::ContiguousTextNodes(*orphan)) == u8"x");
    }
  }

  TEST_CASE("Child and descendant text content: the data of the Text children, or descendants, in tree order",
            "[DOM][TextAlgorithms][dom:concept-child-text-content][dom:concept-descendant-text-content]")
  {
    ContiguousTestData data;
    data.AppendText(u8"a");
    data.Append(data.Document->CreateComment(u8"not text"));
    auto child = data.Append(data.Document->CreateElementNS(u8"urn:x", u8"c").ReleaseValue());
    REQUIRE_FALSE(child->AppendChild(*data.Document->CreateTextNode(u8"b")).HasException());
    data.AppendCDATA(u8"c");

    CHECK(TextAlgorithms::ChildTextContent(*data.Parent) == u8"ac");
    CHECK(TextAlgorithms::DescendantTextContent(*data.Parent) == u8"abc");
    CHECK(TextAlgorithms::ChildTextContent(*child) == u8"b");
  }

  TEST_CASE("Split a Text node: the data after the offset moves to a new node inserted after it",
            "[DOM][TextAlgorithms][dom:concept-text-split]")
  {
    ContiguousTestData data;
    auto text = data.AppendText(u8"Hello, world!");
    auto next = data.Append(data.Document->CreateComment(u8"next"));

    auto observer = CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    REQUIRE_FALSE(observer
                    ->Observe(*data.Parent, MutationObserverInit {.ChildList = true,
                                                                  .CharacterData = true,
                                                                  .Subtree = true,
                                                                  .CharacterDataOldValue = true})
                    .HasException());

    auto result = TextAlgorithms::Split(*text, 2);
    REQUIRE_FALSE(result.HasException());
    auto &newNode = result.Value();

    CHECK(text->Data() == u8"He");
    CHECK(newNode->Data() == u8"llo, world!");
    CHECK(&newNode->NodeDocument() == data.Document.get());
    CHECK(text->NextSibling() == newNode.get());
    CHECK(newNode->NextSibling() == next.get());

    // The insertion, then the data's replacement.
    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 2uz);
    CHECK(records[0]->Type() == u8"childList");
    CHECK(records[0]->AddedNodes()->Item(0) == newNode.get());
    CHECK(records[1]->Type() == u8"characterData");
    CHECK(records[1]->OldValue() == u8"Hello, world!");

    SECTION("A node without a parent keeps the new one apart")
    {
      auto orphan = data.Document->CreateTextNode(u8"abc");
      auto split = TextAlgorithms::Split(*orphan, 3);
      REQUIRE_FALSE(split.HasException());
      CHECK(orphan->Data() == u8"abc");
      CHECK(split.Value()->Data() == u8"");
      CHECK(split.Value()->ParentNode() == nullptr);
    }

    SECTION("A CDATASection splits into a Text node")
    {
      auto cdata = data.AppendCDATA(u8"abc");
      auto split = TextAlgorithms::Split(*cdata, 1);
      REQUIRE_FALSE(split.HasException());
      CHECK(split.Value()->NodeType() == NodeType::TEXT_NODE);
    }

    SECTION("An offset past the end throws an IndexSizeError")
    {
      auto split = TextAlgorithms::Split(*text, 3);
      REQUIRE(split.HasException());
      CHECK(split.GetException().Code() == ExceptionCode::IndexSizeError);
    }
  }

  TEST_CASE(
    "Split a Text node: live ranges past the offset move to the new node, and the gap after it widens",
    "[DOM][TextAlgorithms][dom:concept-text-split]")
  {
    ContiguousTestData data;
    auto text = data.AppendText(u8"Hello, world!");

    auto rangeOf = [&](Node &startNode, size_t start, Node &endNode, size_t end)
    {
      auto range = data.Document->CreateRange();
      REQUIRE_FALSE(range->SetStart(startNode, start).HasException());
      REQUIRE_FALSE(range->SetEnd(endNode, end).HasException());
      return range;
    };
    auto before = rangeOf(*text, 1, *text, 2);
    auto moved = rangeOf(*text, 5, *text, 6);
    auto movedEnd = rangeOf(*text, 1, *text, 8);
    auto atOffset = rangeOf(*text, 2, *text, 2);
    auto inParent = rangeOf(*data.Parent, 1, *data.Parent, 1);

    auto result = TextAlgorithms::Split(*text, 2);
    REQUIRE_FALSE(result.HasException());
    auto &newNode = result.Value();

    CHECK(before->StartContainer() == text.get());
    CHECK(before->StartOffset() == 1uz);
    CHECK(before->EndContainer() == text.get());
    CHECK(before->EndOffset() == 2uz);

    CHECK(moved->StartContainer() == newNode.get());
    CHECK(moved->StartOffset() == 3uz);
    CHECK(moved->EndContainer() == newNode.get());
    CHECK(moved->EndOffset() == 4uz);

    CHECK(movedEnd->StartContainer() == text.get());
    CHECK(movedEnd->StartOffset() == 1uz);
    CHECK(movedEnd->EndContainer() == newNode.get());
    CHECK(movedEnd->EndOffset() == 6uz);

    // At the offset, not greater: it stays.
    CHECK(atOffset->StartContainer() == text.get());
    CHECK(atOffset->StartOffset() == 2uz);

    // At the index of node plus 1: after the new node.
    CHECK(inParent->StartContainer() == data.Parent.get());
    CHECK(inParent->StartOffset() == 2uz);
    CHECK(inParent->EndOffset() == 2uz);
  }
}
