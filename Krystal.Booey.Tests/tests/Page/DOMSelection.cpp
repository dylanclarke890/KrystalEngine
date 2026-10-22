#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Counts the events it hears.
    struct EventCounter : public dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(dom::Event &) noexcept override
      {
        ++Count;
      }
    };

    struct DOMSelectionFixture : PaintFixture
    {
      explicit DOMSelectionFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                           + utf8_string(markup),
                         {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      ~DOMSelectionFixture() noexcept
      {
        Document->Selection().DisassociateLiveRange();
        html::EventLoop::Current().RunUntilIdle();
      }

      KRYS_NODISCARD DOMSelection &Selection() noexcept
      {
        return *Document->GetSelection();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DOMSelection: attributes", "[Page][DOMSelection]")
  {
    DOMSelectionFixture fixture(u8"<p id=a>The quick brown fox</p><p id=b>jumps</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    auto &second = fixture.TextIn(u8"b");

    // The same object every time.
    CHECK(fixture.Document->GetSelection() == &selection);

    CHECK(selection.Type() == u8"None");
    CHECK(selection.Direction() == u8"none");
    CHECK(selection.RangeCount() == 0);
    CHECK(!selection.AnchorNode());
    CHECK(selection.IsCollapsed());
    CHECK(selection.ToString().empty());

    REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
    CHECK(selection.Type() == u8"Range");
    CHECK(selection.Direction() == u8"forward");
    CHECK(selection.AnchorNode().get() == &text);
    CHECK(selection.AnchorOffset() == 4);
    CHECK(selection.FocusNode().get() == &text);
    CHECK(selection.FocusOffset() == 9);
    CHECK(!selection.IsCollapsed());
    CHECK(selection.RangeCount() == 1);
    CHECK(selection.ToString() == u8"quick");
    CHECK(selection.ContainsNode(text, true));
    CHECK_FALSE(selection.ContainsNode(text));
    CHECK_FALSE(selection.ContainsNode(second, true));

    // Backwards: the anchor is after the focus.
    REQUIRE(!selection.SetBaseAndExtent(text, 9, text, 4).HasException());
    CHECK(selection.Direction() == u8"backward");
    CHECK(selection.AnchorOffset() == 9);
    CHECK(selection.FocusOffset() == 4);

    // Across paragraphs.
    REQUIRE(!selection.SetBaseAndExtent(text, 16, second, 2).HasException());
    // The paragraphs' margins add a blank line.
    CHECK(selection.ToString() == u8"fox\n\nju");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DOMSelection: collapsing and extending", "[Page][DOMSelection]")
  {
    DOMSelectionFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    auto &paragraph = fixture.ById(u8"a");

    SECTION("collapse, setPosition and extend")
    {
      REQUIRE(!selection.Collapse(&text, 4).HasException());
      CHECK(selection.Type() == u8"Caret");
      CHECK(selection.IsCollapsed());
      REQUIRE(!selection.Extend(text, 9).HasException());
      CHECK(selection.ToString() == u8"quick");
      REQUIRE(!selection.SetPosition(&text, 2).HasException());
      CHECK(selection.AnchorOffset() == 2);
      REQUIRE(!selection.Collapse(nullptr).HasException());
      CHECK(selection.Type() == u8"None");
    }

    SECTION("collapseToStart and collapseToEnd")
    {
      REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
      REQUIRE(!selection.CollapseToEnd().HasException());
      CHECK(selection.AnchorOffset() == 9);
      REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
      REQUIRE(!selection.CollapseToStart().HasException());
      CHECK(selection.AnchorOffset() == 4);
      selection.Empty();
      CHECK(selection.CollapseToStart() == dom::ExceptionCode::InvalidStateError);
    }

    SECTION("selectAllChildren and modify")
    {
      REQUIRE(!selection.SelectAllChildren(paragraph).HasException());
      CHECK(selection.AnchorNode().get() == &paragraph);
      CHECK(selection.ToString() == u8"The quick brown fox");

      REQUIRE(!selection.Collapse(&text, 4).HasException());
      selection.Modify(u8"extend", u8"forward", u8"word");
      CHECK(selection.ToString() == u8"quick ");
      selection.Modify(u8"EXTEND", u8"Backward", u8"Character");
      CHECK(selection.ToString() == u8"quick");
      selection.Modify(u8"move", u8"forward", u8"lineboundary");
      CHECK(selection.IsCollapsed());
      CHECK(selection.FocusOffset() == 19);
      // Unknown values do nothing.
      selection.Modify(u8"jump", u8"forward", u8"word");
      selection.Modify(u8"move", u8"up", u8"word");
      selection.Modify(u8"move", u8"forward", u8"page");
      CHECK(selection.FocusOffset() == 19);
    }

    SECTION("Errors")
    {
      CHECK(selection.Collapse(&text, 100) == dom::ExceptionCode::IndexSizeError);
      CHECK(selection.Extend(text, 1) == dom::ExceptionCode::InvalidStateError);
      CHECK(selection.GetRangeAt(0) == dom::ExceptionCode::IndexSizeError);
      auto other = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<dom::Node>(text), 0},
                                         dom::BoundaryPoint {ShareRef<dom::Node>(text), 1});
      CHECK(selection.RemoveRange(*other) == dom::ExceptionCode::NotFoundError);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DOMSelection: the live range", "[Page][DOMSelection]")
  {
    DOMSelectionFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");

    SECTION("getRangeAt gives the same range, and setting it sets the selection")
    {
      REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
      auto range = selection.GetRangeAt(0);
      REQUIRE(!range.HasException());
      Ref<dom::Range> liveRange = range.ReleaseValue();
      CHECK(liveRange->StartOffset() == 4);
      CHECK(liveRange->EndOffset() == 9);
      CHECK(selection.GetRangeAt(0).Value().get() == liveRange.get());
      CHECK(liveRange->IsAssociatedWithSelection());

      REQUIRE(!liveRange->SetEnd(text, 15).HasException());
      CHECK(selection.ToString() == u8"quick brown");
      liveRange->Collapse(true);
      CHECK(selection.Type() == u8"Caret");

      // Changing the selection another way leaves the range behind.
      REQUIRE(!selection.SetBaseAndExtent(text, 0, text, 3).HasException());
      CHECK_FALSE(liveRange->IsAssociatedWithSelection());
      REQUIRE(!liveRange->SetEnd(text, 19).HasException());
      CHECK(selection.ToString() == u8"The");
      CHECK(selection.GetRangeAt(0).Value().get() != liveRange.get());
    }

    SECTION("addRange, removeRange and deleteFromDocument")
    {
      auto liveRange = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<dom::Node>(text), 4},
                                             dom::BoundaryPoint {ShareRef<dom::Node>(text), 10});
      selection.AddRange(*liveRange);
      CHECK(selection.RangeCount() == 1);
      CHECK(selection.ToString() == u8"quick ");

      // A selection that's already there isn't replaced.
      auto second = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<dom::Node>(text), 0},
                                          dom::BoundaryPoint {ShareRef<dom::Node>(text), 3});
      selection.AddRange(*second);
      CHECK(selection.ToString() == u8"quick ");

      selection.DeleteFromDocument();
      CHECK(text.Data() == u8"The brown fox");
      CHECK(selection.IsCollapsed());

      REQUIRE(!selection.RemoveRange(*liveRange).HasException());
      CHECK(selection.RangeCount() == 0);
      CHECK_FALSE(liveRange->IsAssociatedWithSelection());
    }

    SECTION("getComposedRanges")
    {
      REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
      auto ranges = selection.GetComposedRanges();
      REQUIRE(ranges.size() == 1);
      CHECK(ranges[0]->StartContainer() == &text);
      CHECK(ranges[0]->StartOffset() == 4);
      CHECK(ranges[0]->EndOffset() == 9);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DOMSelection: selectionchange", "[Page][DOMSelection]")
  {
    DOMSelectionFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    auto counter = CreateRef<EventCounter>();
    fixture.Document->AddEventListener(dom::DOMStringAtom(u8"selectionchange"),
                                       Ref<dom::EventListenerCallback> {counter}, false);
    html::EventLoop::Current().RunUntilIdle();

    // Changes in a task fire one event, in a later task.
    REQUIRE(!selection.Collapse(&text, 4).HasException());
    REQUIRE(!selection.Extend(text, 9).HasException());
    CHECK(counter->Count == 0);
    html::EventLoop::Current().RunUntilIdle();
    CHECK(counter->Count == 1);

    // Setting the same selection again isn't a change.
    REQUIRE(!selection.SetBaseAndExtent(text, 4, text, 9).HasException());
    html::EventLoop::Current().RunUntilIdle();
    CHECK(counter->Count == 1);

    // The frame's own changes count too.
    fixture.Document->Selection().SelectAll();
    html::EventLoop::Current().RunUntilIdle();
    CHECK(counter->Count == 2);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "DOMSelection: a document without a frame has none",
                   "[Page][DOMSelection]")
  {
    auto document = CreateRef<dom::Document>();
    CHECK(document->GetSelection() == nullptr);
  }
}
