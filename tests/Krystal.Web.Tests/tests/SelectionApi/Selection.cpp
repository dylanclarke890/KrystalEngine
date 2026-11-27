#include "Krystal.Web/SelectionApi/Selection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Ranges/Range.hpp"
#include "Krystal.Web/Dom/Ranges/StaticRange.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"

namespace krys::web::tests
{
  namespace
  {
    /// @brief Counts the events it hears.
    struct EventCounter : public EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(Event &) noexcept override
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

      KRYS_NODISCARD Selection &Selection() noexcept
      {
        return *Document->GetSelection();
      }

      KRYS_NODISCARD Text &TextIn(DOMStringView id) noexcept
      {
        return Downcast<Text>(*ById(id).FirstChild());
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: attributes", "[SelectionApi][Selection]")
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: collapsing and extending", "[SelectionApi][Selection]")
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
      CHECK(selection.CollapseToStart() == ExceptionCode::InvalidStateError);
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
      CHECK(selection.Collapse(&text, 100) == ExceptionCode::IndexSizeError);
      CHECK(selection.Extend(text, 1) == ExceptionCode::InvalidStateError);
      CHECK(selection.GetRangeAt(0) == ExceptionCode::IndexSizeError);
      auto other = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<Node>(text), 0},
                                         dom::BoundaryPoint {ShareRef<Node>(text), 1});
      CHECK(selection.RemoveRange(*other) == ExceptionCode::NotFoundError);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: the live range", "[SelectionApi][Selection]")
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
      auto liveRange = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<Node>(text), 4},
                                             dom::BoundaryPoint {ShareRef<Node>(text), 10});
      selection.AddRange(*liveRange);
      CHECK(selection.RangeCount() == 1);
      CHECK(selection.ToString() == u8"quick ");

      // A selection that's already there isn't replaced.
      auto second = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<Node>(text), 0},
                                          dom::BoundaryPoint {ShareRef<Node>(text), 3});
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: selectionchange", "[SelectionApi][Selection]")
  {
    DOMSelectionFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    auto counter = CreateRef<EventCounter>();
    fixture.Document->AddEventListener(u8"selectionchange", counter, false);
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

  namespace
  {
    /// @brief A paragraph, and a host whose open shadow tree holds a text node, a span, and a host of its own
    /// whose shadow tree holds another text node.
    struct SelectionShadowFixture : DOMSelectionFixture
    {
      RefPtr<ShadowRoot> Outer;
      RefPtr<ShadowRoot> Inner;
      RefPtr<Text> OuterText;
      RefPtr<Element> OuterSpan;
      RefPtr<Text> InnerText;

      SelectionShadowFixture() noexcept
          : DOMSelectionFixture(u8"<p id=a>The quick brown fox</p><div id=host></div>")
      {
        auto &document = *Document;
        Outer = ById(u8"host").AttachShadow({}).ReleaseValue();
        OuterText = document.CreateTextNode(u8"outer text");
        OuterSpan = document.CreateElement(u8"span").ReleaseValue();
        auto innerHost = document.CreateElement(u8"div").ReleaseValue();
        (void)Outer->AppendChild(*OuterText);
        (void)Outer->AppendChild(*OuterSpan);
        (void)OuterSpan->AppendChild(*document.CreateTextNode(u8"span text"));
        (void)Outer->AppendChild(*innerHost);
        Inner = innerHost->AttachShadow({}).ReleaseValue();
        InnerText = document.CreateTextNode(u8"inner text");
        (void)Inner->AppendChild(*InnerText);
        (void)document.UpdateLayout();
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Selection: getComposedRanges keeps a shadow tree around a listed root",
                   "[SelectionApi][Selection][selection-api:dom-selection-getcomposedranges]")
  {
    SelectionShadowFixture fixture;
    auto &selection = fixture.Selection();
    REQUIRE(!selection.SetBaseAndExtent(*fixture.OuterText, 0, *fixture.OuterText, 5).HasException());
    REQUIRE(fixture.Document->Selection().Selection().Range()->Start.Container.get()
            == fixture.OuterText.get());

    // Unlisted, the outer tree is left for its host's place in the body.
    auto ranges = selection.GetComposedRanges();
    REQUIRE(ranges.size() == 1);
    CHECK(ranges[0]->StartContainer() == fixture.Document->Body());
    CHECK(ranges[0]->StartOffset() == 1);
    CHECK(ranges[0]->EndOffset() == 2);

    // The outer tree is a shadow-including ancestor of the listed inner one, so the range stays in it.
    ranges = selection.GetComposedRanges({ShareRef(*fixture.Inner)});
    REQUIRE(ranges.size() == 1);
    CHECK(ranges[0]->StartContainer() == fixture.OuterText.get());
    CHECK(ranges[0]->StartOffset() == 0);
    CHECK(ranges[0]->EndContainer() == fixture.OuterText.get());
    CHECK(ranges[0]->EndOffset() == 5);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: extend does nothing for a node outside the document",
                   "[SelectionApi][Selection][selection-api:dom-selection-extend]")
  {
    DOMSelectionFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto detached = fixture.Document->CreateTextNode(u8"detached");

    // The node is checked before the selection, so an empty selection doesn't throw.
    CHECK_FALSE(selection.Extend(*detached, 0).HasException());
    CHECK(selection.Type() == u8"None");
    CHECK(selection.Extend(fixture.TextIn(u8"a"), 0) == ExceptionCode::InvalidStateError);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: selectAllChildren needs a node in the document tree",
                   "[SelectionApi][Selection][selection-api:dom-selection-selectallchildren]")
  {
    SelectionShadowFixture fixture;
    auto &selection = fixture.Selection();

    REQUIRE(!selection.SelectAllChildren(*fixture.OuterSpan).HasException());
    CHECK(fixture.Document->Selection().IsNone());

    auto doctype = fixture.Document->DocType();
    REQUIRE(doctype);
    CHECK(selection.SelectAllChildren(*doctype) == ExceptionCode::InvalidNodeTypeError);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Selection: deleteFromDocument needs the selection in the document tree",
                   "[SelectionApi][Selection][selection-api:dom-selection-deletefromdocument]")
  {
    SelectionShadowFixture fixture;
    auto &selection = fixture.Selection();
    REQUIRE(!selection.SetBaseAndExtent(*fixture.OuterText, 0, *fixture.OuterText, 6).HasException());
    REQUIRE(!fixture.Document->Selection().IsNone());

    selection.DeleteFromDocument();
    CHECK(fixture.OuterText->Data() == u8"outer text");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: addRange needs a range in the document",
                   "[SelectionApi][Selection][selection-api:dom-selection-addrange]")
  {
    SelectionShadowFixture fixture;
    auto &selection = fixture.Selection();

    SECTION("In a shadow tree")
    {
      auto range = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<Node>(*fixture.OuterText), 0},
                                         dom::BoundaryPoint {ShareRef<Node>(*fixture.OuterText), 5});
      selection.AddRange(*range);
      CHECK_FALSE(range->IsAssociatedWithSelection());
      CHECK(fixture.Document->Selection().IsNone());
    }

    SECTION("In another document")
    {
      auto other = CreateRef<Document>();
      auto text = other->CreateTextNode(u8"text");
      auto range = CreateRef<dom::Range>(dom::BoundaryPoint {ShareRef<Node>(*text), 0},
                                         dom::BoundaryPoint {ShareRef<Node>(*text), 2});
      selection.AddRange(*range);
      CHECK_FALSE(range->IsAssociatedWithSelection());
      CHECK(fixture.Document->Selection().IsNone());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Selection: a document without a frame has none",
                   "[SelectionApi][Selection]")
  {
    auto document = CreateRef<Document>();
    CHECK(document->GetSelection() == nullptr);
  }
}
