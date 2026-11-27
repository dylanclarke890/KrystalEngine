#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Web.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Ranges/Range.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/SelectionApi/Selection.hpp"

// The selection's range is a live range, so the DOM's mutation algorithms move it, whether or not script has
// asked for it: each test runs both ways (askedForRange).

namespace krys::web::tests
{
  namespace
  {
    /// @brief Counts the `selectionchange` events it hears.
    struct SelectionChangeCounter : public EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(Event &) noexcept override
      {
        ++Count;
      }
    };

    struct SelectionMutationFixture : PaintFixture
    {
      explicit SelectionMutationFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                           + utf8_string(markup),
                         {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      ~SelectionMutationFixture() noexcept
      {
        Document->Selection().DisassociateLiveRange();
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD Selection &Selection() noexcept
      {
        return *Document->GetSelection();
      }

      KRYS_NODISCARD Text &TextIn(DOMStringView id) noexcept
      {
        return Downcast<Text>(*ById(id).FirstChild());
      }

      /// @brief The selection's range, when the test asks for it before mutating; null otherwise.
      KRYS_NODISCARD RefPtr<dom::Range> RangeIfAsked(bool askedForRange) noexcept
      {
        if (!askedForRange)
        {
          return nullptr;
        }
        auto range = Selection().GetRangeAt(0);
        REQUIRE(!range.HasException());
        return range.ReleaseValue();
      }

      /// @brief Checks the selection's anchor and focus, and that a range asked for is still its range.
      void CheckEnds(const Node &anchor, uint32 anchorOffset, const Node &focus, uint32 focusOffset,
                     const RefPtr<dom::Range> &askedRange) noexcept
      {
        auto &selection = Selection();
        CHECK(selection.AnchorNode().get() == &anchor);
        CHECK(selection.AnchorOffset() == anchorOffset);
        CHECK(selection.FocusNode().get() == &focus);
        CHECK(selection.FocusOffset() == focusOffset);
        if (askedRange)
        {
          auto range = selection.GetRangeAt(0);
          REQUIRE(!range.HasException());
          CHECK(range.Value().get() == askedRange.get());
        }
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Selection: removing the node holding an end moves it to the parent",
                   "[SelectionApi][Selection][selection-api:responding-to-dom-mutations]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionMutationFixture fixture(u8"<div id=d><p id=a>one</p><p id=b>two</p><p id=c>three</p></div>");
    auto &selection = fixture.Selection();
    auto &div = fixture.ById(u8"d");
    auto &three = fixture.TextIn(u8"c");
    Ref<Element> removed = ShareRef(fixture.ById(u8"b"));

    SECTION("Forwards")
    {
      REQUIRE(!selection.SetBaseAndExtent(fixture.TextIn(u8"b"), 1, three, 2).HasException());
      auto range = fixture.RangeIfAsked(askedForRange);
      REQUIRE(!div.RemoveChild(*removed).HasException());
      fixture.CheckEnds(div, 1, three, 2, range);
      CHECK(selection.Direction() == u8"forward");
    }

    SECTION("Backwards: the direction is kept")
    {
      REQUIRE(!selection.SetBaseAndExtent(three, 2, fixture.TextIn(u8"b"), 1).HasException());
      auto range = fixture.RangeIfAsked(askedForRange);
      REQUIRE(!div.RemoveChild(*removed).HasException());
      fixture.CheckEnds(three, 2, div, 1, range);
      CHECK(selection.Direction() == u8"backward");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: removing a node between the ends leaves them",
                   "[SelectionApi][Selection][selection-api:responding-to-dom-mutations]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionMutationFixture fixture(u8"<div id=d><p id=a>one</p><p id=b>two</p><p id=c>three</p></div>");
    auto &selection = fixture.Selection();
    auto &one = fixture.TextIn(u8"a");
    auto &three = fixture.TextIn(u8"c");
    Ref<Element> removed = ShareRef(fixture.ById(u8"b"));
    REQUIRE(!selection.SetBaseAndExtent(one, 1, three, 2).HasException());
    auto range = fixture.RangeIfAsked(askedForRange);

    REQUIRE(!fixture.ById(u8"d").RemoveChild(*removed).HasException());
    fixture.CheckEnds(one, 1, three, 2, range);
    CHECK(selection.ToString() == u8"ne\n\nth");
  }

  // FIXME: The selection's ends don't move when a child is inserted before them, even with its live range
  // asked for: FrameSelection has no insertion steps, and the live range's update doesn't reach it.
  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Selection: inserting a child before the ends moves their offsets",
                   "[SelectionApi][Selection][selection-api:responding-to-dom-mutations][!shouldfail]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionMutationFixture fixture(u8"<div id=d><p id=a>one</p><p id=b>two</p><p id=c>three</p></div>");
    auto &selection = fixture.Selection();
    auto &div = fixture.ById(u8"d");
    // The second paragraph, as the div's children.
    REQUIRE(!selection.SetBaseAndExtent(div, 1, div, 2).HasException());
    auto range = fixture.RangeIfAsked(askedForRange);

    auto inserted = fixture.Document->CreateElement(u8"p").ReleaseValue();
    REQUIRE(!div.InsertBefore(*inserted, div.FirstChild()).HasException());
    fixture.CheckEnds(div, 2, div, 3, range);
    CHECK(selection.ToString() == u8"two");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: replacing text moves the ends after it",
                   "[SelectionApi][Selection][selection-api:responding-to-dom-mutations]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionMutationFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    // "quick"
    REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
    auto range = fixture.RangeIfAsked(askedForRange);

    SECTION("Before the selection")
    {
      REQUIRE(!text.ReplaceData(0, 3, u8"A").HasException());
      fixture.CheckEnds(text, 2, text, 7, range);
      CHECK(selection.ToString() == u8"quick");
    }

    SECTION("Inside the selection: the end moves by the difference")
    {
      REQUIRE(!text.ReplaceData(5, 2, u8"X").HasException());
      fixture.CheckEnds(text, 4, text, 8, range);
      CHECK(selection.ToString() == u8"qXck");
    }

    SECTION("After the selection")
    {
      REQUIRE(!text.ReplaceData(10, 5, u8"red").HasException());
      fixture.CheckEnds(text, 4, text, 9, range);
      CHECK(selection.ToString() == u8"quick");
    }
  }

  // FIXME: The selection's ends stay in the split text node, clamped to its new length, even with its live
  // range asked for.
  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: splitting text moves the ends after the split",
                   "[SelectionApi][Selection][selection-api:responding-to-dom-mutations][!shouldfail]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionMutationFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    // "brown"
    REQUIRE(!selection.SetBaseAndExtent(text, 10, text, 15).HasException());
    auto range = fixture.RangeIfAsked(askedForRange);

    auto split = text.SplitText(4);
    REQUIRE(!split.HasException());
    Ref<Text> after = split.ReleaseValue();
    fixture.CheckEnds(*after, 6, *after, 11, range);
    CHECK(selection.ToString() == u8"brown");
  }

  // FIXME: The selection's ends stay in the text node normalize() removes, even with its live range asked
  // for.
  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: normalizing moves the ends into the merged text",
                   "[SelectionApi][Selection][selection-api:responding-to-dom-mutations][!shouldfail]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionMutationFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &paragraph = fixture.ById(u8"a");
    auto &text = fixture.TextIn(u8"a");
    Ref<Text> second = text.SplitText(10).ReleaseValue();
    (void)fixture.Document->UpdateLayout();
    // "brown", in the second text node.
    REQUIRE(!selection.SetBaseAndExtent(*second, 0, *second, 5).HasException());
    auto range = fixture.RangeIfAsked(askedForRange);

    REQUIRE(!paragraph.Normalize().HasException());
    REQUIRE(paragraph.FirstChild() == &text);
    fixture.CheckEnds(text, 10, text, 15, range);
    CHECK(selection.ToString() == u8"brown");
  }

  namespace
  {
    /// @brief A selection in three paragraphs, and a count of the `selectionchange` events at the document.
    struct SelectionChangeFixture : SelectionMutationFixture
    {
      Ref<SelectionChangeCounter> Counter = CreateRef<SelectionChangeCounter>();
      bool AskedForRange;

      explicit SelectionChangeFixture(bool askedForRange) noexcept
          : SelectionMutationFixture(
              u8"<div id=d><p id=a>The quick brown fox</p><p id=b>two</p><p id=c>three</p></div>"),
            AskedForRange(askedForRange)
      {
        Document->AddEventListener(u8"selectionchange", Counter, false);
      }

      /// @brief Selects, then lets the event for that fire and starts counting.
      void Select(Node &anchor, uint32 anchorOffset, Node &focus, uint32 focusOffset) noexcept
      {
        REQUIRE(!Selection().SetBaseAndExtent(anchor, anchorOffset, focus, focusOffset).HasException());
        (void)RangeIfAsked(AskedForRange);
        html::EventLoop::Current().RunUntilIdle();
        Counter->Count = 0;
      }

      KRYS_NODISCARD int32 EventsAfterTasks() noexcept
      {
        html::EventLoop::Current().RunUntilIdle();
        return Counter->Count;
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Selection: changing text that moves the selection schedules selectionchange",
                   "[SelectionApi][Selection][selection-api:selectionchange-event]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionChangeFixture fixture(askedForRange);
    auto &text = fixture.TextIn(u8"a");

    SECTION("Replacing text before the ends")
    {
      fixture.Select(text, 4, text, 9);
      REQUIRE(!text.ReplaceData(0, 3, u8"A").HasException());
      CHECK(fixture.EventsAfterTasks() == 1);
    }

    SECTION("Splitting text before the ends")
    {
      fixture.Select(text, 10, text, 15);
      REQUIRE(!text.SplitText(4).HasException());
      CHECK(fixture.EventsAfterTasks() == 1);
    }
  }

  // FIXME: The selection doesn't schedule selectionchange when a removal or insertion moves its ends:
  // FrameSelection adjusts them without going through SetSelection, and the live range's updates don't
  // reach it.
  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Selection: changing the tree around the selection schedules selectionchange",
                   "[SelectionApi][Selection][selection-api:selectionchange-event][!shouldfail]")
  {
    bool askedForRange = GENERATE(false, true);
    CAPTURE(askedForRange);
    SelectionChangeFixture fixture(askedForRange);
    auto &div = fixture.ById(u8"d");

    SECTION("Removing the node holding an end")
    {
      fixture.Select(fixture.TextIn(u8"b"), 1, fixture.TextIn(u8"c"), 2);
      Ref<Element> removed = ShareRef(fixture.ById(u8"b"));
      REQUIRE(!div.RemoveChild(*removed).HasException());
      CHECK(fixture.EventsAfterTasks() == 1);
    }

    SECTION("Inserting a child before the ends")
    {
      fixture.Select(div, 1, div, 2);
      auto inserted = fixture.Document->CreateElement(u8"p").ReleaseValue();
      REQUIRE(!div.InsertBefore(*inserted, div.FirstChild()).HasException());
      CHECK(fixture.EventsAfterTasks() == 1);
    }
  }
}
