#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NodeFilter.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief Records the nodes it's called with, and returns a fixed result or an exception.
    class RecordingNodeFilter : public NodeFilter
    {
    public:
      mutable List<const Node *> Calls;
      Maybe<ExceptionCode> Throws;
      FilterResult Result {FilterResult::FILTER_ACCEPT};
      // When set, the filter calls it from inside acceptNode() and records what it returns.
      Func<ExceptionOr<RefPtr<Node>>()> Reenter;
      mutable List<ExceptionOr<RefPtr<Node>>> ReentryResults;

      KRYS_NODISCARD ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept override
      {
        Calls.push_back(&node);
        if (Reenter)
        {
          ReentryResults.push_back(Reenter());
        }
        if (Throws)
        {
          return Exception {*Throws};
        }
        return Result;
      }
    };

    /// @brief div > (b > "text", <!--comment-->).
    struct FilterTestData
    {
      Ref<Document> Document;
      Ref<Element> Div;
      Ref<Element> B;
      Ref<Text> BText;
      Ref<Comment> Comment;

      FilterTestData()
          : Document(CreateRef<HTMLDocument>()), Div(Document->CreateElement(u8"div").ReleaseValue()),
            B(Document->CreateElement(u8"b").ReleaseValue()), BText(Document->CreateTextNode(u8"text")),
            Comment(Document->CreateComment(u8"comment"))
      {
        REQUIRE_FALSE(B->AppendChild(*BText).HasException());
        REQUIRE_FALSE(Div->AppendChild(*B).HasException());
        REQUIRE_FALSE(Div->AppendChild(*Comment).HasException());
      }
    };
  }

  TEST_CASE(
    "Filter: a node whose type isn't in whatToShow is skipped without calling the filter",
    "[DOM][IteratorAlgorithms][dom:concept-node-filter][dom:traversal][dom:concept-traversal-whattoshow]")
  {
    FilterTestData data;
    auto filter = CreateRef<RecordingNodeFilter>();
    // Only comments: the nth bit is the node type minus one.
    auto iterator =
      data.Document->CreateNodeIterator(*data.Div, WhatToShow::SHOW_COMMENT, RefPtr<NodeFilter> {filter});
    CHECK(iterator->NextNode().ReleaseValue() == data.Comment.get());
    CHECK(iterator->NextNode().ReleaseValue() == nullptr);
    REQUIRE(filter->Calls.size() == 1uz);
    CHECK(filter->Calls[0] == data.Comment.get());

    // Elements and text, through a TreeWalker.
    auto walker =
      data.Document->CreateTreeWalker(*data.Div, WhatToShow::SHOW_ELEMENT | WhatToShow::SHOW_TEXT);
    CHECK(walker->NextNode().ReleaseValue() == data.B.get());
    CHECK(walker->NextNode().ReleaseValue() == data.BText.get());
    CHECK(walker->NextNode().ReleaseValue() == nullptr);
  }

  TEST_CASE("Filter: with no filter, every shown node is accepted",
            "[DOM][IteratorAlgorithms][dom:concept-node-filter][dom:concept-traversal-filter]"
            "[dom:concept-traversal-root]")
  {
    FilterTestData data;
    auto iterator = data.Document->CreateNodeIterator(*data.Div, WhatToShow::SHOW_ALL);
    // Only the root and its descendants.
    CHECK(&iterator->Root() == data.Div.get());
    CHECK(iterator->Filter() == nullptr);
    CHECK(iterator->NextNode().ReleaseValue() == data.Div.get());
    CHECK(iterator->NextNode().ReleaseValue() == data.B.get());
    CHECK(iterator->NextNode().ReleaseValue() == data.BText.get());
    CHECK(iterator->NextNode().ReleaseValue() == data.Comment.get());
    CHECK(iterator->NextNode().ReleaseValue() == nullptr);
  }

  TEST_CASE("Filter: the filter's result decides, and it's called with the node",
            "[DOM][IteratorAlgorithms][dom:concept-node-filter][dom:concept-traversal-filter]")
  {
    FilterTestData data;
    auto filter = CreateRef<RecordingNodeFilter>();
    filter->Result = FilterResult::FILTER_SKIP;
    auto iterator =
      data.Document->CreateNodeIterator(*data.Div, WhatToShow::SHOW_ALL, RefPtr<NodeFilter> {filter});
    CHECK(iterator->NextNode().ReleaseValue() == nullptr);
    REQUIRE(filter->Calls.size() == 4uz);
    CHECK(filter->Calls[0] == data.Div.get());
    CHECK(filter->Calls[3] == data.Comment.get());
  }

  TEST_CASE("Filter: calling the traverser from its own filter throws an InvalidStateError",
            "[DOM][IteratorAlgorithms][dom:concept-node-filter][dom:concept-traversal-active]")
  {
    FilterTestData data;
    auto filter = CreateRef<RecordingNodeFilter>();

    SECTION("NodeIterator")
    {
      auto iterator =
        data.Document->CreateNodeIterator(*data.Div, WhatToShow::SHOW_ALL, RefPtr<NodeFilter> {filter});
      filter->Reenter = [&]
      {
        return iterator->NextNode();
      };
      CHECK(iterator->NextNode().ReleaseValue() == data.Div.get());
      REQUIRE(filter->ReentryResults.size() == 1uz);
      REQUIRE(filter->ReentryResults[0].HasException());
      CHECK(filter->ReentryResults[0].GetException().Code() == ExceptionCode::InvalidStateError);

      // It's no longer active once the filter returns.
      filter->Reenter = nullptr;
      CHECK(iterator->NextNode().ReleaseValue() == data.B.get());
    }

    SECTION("TreeWalker")
    {
      auto walker =
        data.Document->CreateTreeWalker(*data.Div, WhatToShow::SHOW_ALL, RefPtr<NodeFilter> {filter});
      filter->Reenter = [&]() -> ExceptionOr<RefPtr<Node>>
      {
        auto result = walker->FirstChild();
        if (result.HasException())
        {
          return result.ReleaseException();
        }
        return ShareRefPtr(result.ReleaseValue());
      };
      CHECK(walker->FirstChild().ReleaseValue() == data.B.get());
      REQUIRE(filter->ReentryResults.size() == 1uz);
      REQUIRE(filter->ReentryResults[0].HasException());
      CHECK(filter->ReentryResults[0].GetException().Code() == ExceptionCode::InvalidStateError);
    }
  }

  TEST_CASE("Filter: an exception from the filter is rethrown, and the traverser is no longer active",
            "[DOM][IteratorAlgorithms][dom:concept-node-filter][dom:concept-traversal-active]")
  {
    FilterTestData data;
    auto filter = CreateRef<RecordingNodeFilter>();
    filter->Throws = ExceptionCode::NotSupportedError;
    auto iterator =
      data.Document->CreateNodeIterator(*data.Div, WhatToShow::SHOW_ALL, RefPtr<NodeFilter> {filter});
    auto result = iterator->NextNode();
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);

    filter->Throws = std::nullopt;
    CHECK(iterator->NextNode().ReleaseValue() == data.Div.get());
  }
}
