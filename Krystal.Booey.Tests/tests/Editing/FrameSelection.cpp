#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Rendering/CaretRectComputation.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct FrameSelectionFixture : PaintFixture
    {
      explicit FrameSelectionFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                           + utf8_string(markup),
                         {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }

      KRYS_NODISCARD FrameSelection &Selection() noexcept
      {
        return Document->Selection();
      }

      /// @brief What the rendering update does for the selection: lays out, then sets the render tree's.
      void UpdateRendering() noexcept
      {
        (void)Document->UpdateLayout();
        Selection().UpdateAppearanceAfterUpdatingRendering();
      }
    };

    KRYS_NODISCARD dom::Position At(dom::Node &node, uint32 offset) noexcept
    {
      return dom::MakeContainerOffsetPosition(ShareRefPtr(&node), offset);
    }
  }

  TEST_CASE("FrameSelection: setting the selection", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(u8"<p id=a>First paragraph.</p><p id=b>Second paragraph.</p>");
    auto &selection = fixture.Selection();
    auto &first = fixture.TextIn(u8"a");
    auto &second = fixture.TextIn(u8"b");

    CHECK(selection.IsNone());
    CHECK(selection.IsFocusedAndActive());

    selection.MoveTo(At(first, 6), At(second, 6), Affinity::Downstream);
    CHECK(selection.IsRange());
    CHECK(selection.Selection().Start().DeprecatedNode() == &first);
    CHECK(selection.Selection().End().DeprecatedNode() == &second);
    CHECK(first.ContainsSelectionEndPoint());
    CHECK(fixture.Document->Body()->ContainsSelectionEndPoint());

    // The render tree's selection follows at the rendering update.
    fixture.UpdateRendering();
    const auto &renderSelection = fixture.Document->GetRenderView()->Selection();
    CHECK(renderSelection.Start() == first.Renderer());
    CHECK(renderSelection.End() == second.Renderer());
    CHECK(renderSelection.StartOffset() == 6);
    CHECK(renderSelection.EndOffset() == 6);

    // Extending moves the focus.
    selection.SetExtent(VisiblePosition(At(second, 10)));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 10);
    selection.SetBase(VisiblePosition(At(second, 10)));
    CHECK(selection.IsCaret());

    // A caret has no render tree selection.
    fixture.UpdateRendering();
    CHECK(!renderSelection.Start());

    selection.Clear();
    CHECK(selection.IsNone());
    CHECK(!first.ContainsSelectionEndPoint());
    CHECK(!fixture.Document->Body()->ContainsSelectionEndPoint());
  }

  TEST_CASE("FrameSelection: selecting all and by the mouse", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(u8"<p id=a>First paragraph.</p><p id=b>Second paragraph.</p>");
    auto &selection = fixture.Selection();
    auto &first = fixture.TextIn(u8"a");
    auto &second = fixture.TextIn(u8"b");

    selection.SelectAll();
    CHECK(selection.IsRange());
    CHECK(selection.Selection().VisibleStart() == VisiblePosition(At(first, 0)));
    CHECK(selection.Selection().VisibleEnd() == VisiblePosition(At(second, 17)));
    CHECK(selection.IsAll(EditingBoundaryCrossingRule::CanCrossEditingBoundary));

    // Setting the same selection by the mouse changes nothing; a different one replaces it at its
    // granularity.
    VisibleSelection word(At(first, 2), Affinity::Downstream);
    word.ExpandUsingGranularity(TextGranularity::WordGranularity);
    selection.SetSelectionByMouseIfDifferent(word, TextGranularity::WordGranularity);
    CHECK(selection.Granularity() == TextGranularity::WordGranularity);
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 0);
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 5);
    // On Windows the selection is directional.
    CHECK(selection.Selection().GetDirectionality() == Directionality::Strong);

    // The point is over the selected word, and not over the rest.
    fixture.UpdateRendering();
    CHECK(selection.Contains(LayoutPoint(10_lu, 20_lu)));
    CHECK(!selection.Contains(LayoutPoint(10_lu, 70_lu)));
    CHECK(!selection.SelectionBounds().IsEmpty());
  }

  TEST_CASE("FrameSelection: DOM changes", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(u8"<p id=a>First <span id=s>middle</span> last.</p>");
    auto &selection = fixture.Selection();
    auto &first = fixture.TextIn(u8"a");
    auto &middle = fixture.TextIn(u8"s");

    SECTION("Replacing text before the selection moves it")
    {
      selection.MoveTo(At(first, 2), At(first, 4), Affinity::Downstream);
      REQUIRE_FALSE(first.ReplaceData(0, 1, u8"XYZ").HasException());
      CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 4);
      CHECK(selection.Selection().End().DeprecatedEditingOffset() == 6);
    }

    SECTION("Removing the node an end point is in moves it out")
    {
      selection.MoveTo(At(first, 2), At(middle, 3), Affinity::Downstream);
      fixture.UpdateRendering();
      // Removing the span drops the tree's references to it and its text.
      Ref<dom::Node> span = ShareRef<dom::Node>(fixture.ById(u8"s"));
      Ref<dom::Node> keepMiddle = ShareRef<dom::Node>(middle);
      REQUIRE_FALSE(Downcast<dom::Element>(*span).Remove().HasException());
      CHECK(selection.Selection().Start().DeprecatedNode() == &first);
      CHECK(!middle.ContainsSelectionEndPoint());
      // The render tree's selection is cleared, to be set again at the next update.
      CHECK(!fixture.Document->GetRenderView()->Selection().Start());
      fixture.UpdateRendering();
      CHECK(fixture.Document->GetRenderView()->Selection().Start() == first.Renderer());
    }

    SECTION("Removing everything selected collapses it where it was")
    {
      selection.MoveTo(At(middle, 1), At(middle, 3), Affinity::Downstream);
      Ref<dom::Node> paragraph = ShareRef<dom::Node>(fixture.ById(u8"a"));
      REQUIRE_FALSE(Downcast<dom::Element>(*paragraph).Remove().HasException());
      CHECK(selection.IsCaret());
      CHECK(selection.Selection().Start().DeprecatedNode() == fixture.Document->Body());
    }
  }

  TEST_CASE("FrameSelection: focus", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(u8"<p id=a>Text</p>");
    auto &selection = fixture.Selection();

    CHECK(selection.IsFocused());
    fixture.Document->SetFocusedAndActive(false);
    CHECK(!selection.IsFocusedAndActive());
    fixture.Document->SetFocusedAndActive(true);
    CHECK(selection.IsFocusedAndActive());
  }
}

namespace krys::boo::tests
{
  namespace
  {
    using Alteration = FrameSelection::Alteration;

    /// @brief Counts `selectstart` events, cancelling them if asked.
    struct ModifySelectStartListener : public dom::EventListenerCallback
    {
      int32 Count {0};
      bool Cancel {false};

      void HandleEvent(dom::Event &event) noexcept override
      {
        ++Count;
        if (Cancel)
        {
          event.PreventDefault();
        }
      }
    };
  }

  TEST_CASE("FrameSelection: modifying by character and word", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(u8"<p id=a>The quick brown fox</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");

    selection.MoveTo(At(text, 4), Affinity::Downstream);
    REQUIRE(selection.IsCaret());

    // Extending keeps the base and moves the extent.
    CHECK(
      selection.Modify(Alteration::Extend, SelectionDirection::Right, TextGranularity::CharacterGranularity));
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 4);
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 5);
    CHECK(
      selection.Modify(Alteration::Extend, SelectionDirection::Left, TextGranularity::CharacterGranularity));
    CHECK(
      selection.Modify(Alteration::Extend, SelectionDirection::Left, TextGranularity::CharacterGranularity));
    CHECK(!selection.Selection().IsBaseFirst());
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 3);
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 4);

    // Words: moving right skips the spaces after a word, as Windows does.
    selection.MoveTo(At(text, 0), Affinity::Downstream);
    CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Right, TextGranularity::WordGranularity));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 4);
    CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Right, TextGranularity::WordGranularity));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 10);
    CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Left, TextGranularity::WordGranularity));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 4);

    // Moving collapses a range to its end in the direction moved.
    selection.MoveTo(At(text, 4), At(text, 9), Affinity::Downstream);
    CHECK(
      selection.Modify(Alteration::Move, SelectionDirection::Right, TextGranularity::CharacterGranularity));
    CHECK(selection.IsCaret());
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 9);
    selection.MoveTo(At(text, 4), At(text, 9), Affinity::Downstream);
    CHECK(
      selection.Modify(Alteration::Move, SelectionDirection::Left, TextGranularity::CharacterGranularity));
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 4);
  }

  TEST_CASE("FrameSelection: modifying by line, paragraph and document", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(
      u8"<p id=a style=margin:0>The quick brown fox</p><p id=b style=margin:0>ab</p>"
      u8"<p id=c style=margin:0>jumps over the lazy dog</p>");
    auto &selection = fixture.Selection();
    auto &first = fixture.TextIn(u8"a");
    auto &second = fixture.TextIn(u8"b");
    auto &third = fixture.TextIn(u8"c");

    SECTION("Line boundaries")
    {
      selection.MoveTo(At(first, 4), Affinity::Downstream);
      CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Forward, TextGranularity::LineBoundary));
      CHECK(selection.Selection().End().DeprecatedEditingOffset() == 19);
      CHECK(
        selection.Modify(Alteration::Extend, SelectionDirection::Backward, TextGranularity::LineBoundary));
      CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 0);
    }

    SECTION("Lines keep their horizontal position across a shorter line")
    {
      selection.MoveTo(At(first, 10), Affinity::Downstream);
      CHECK(
        selection.Modify(Alteration::Extend, SelectionDirection::Forward, TextGranularity::LineGranularity));
      CHECK(selection.Selection().End().DeprecatedNode() == &second);
      CHECK(selection.Selection().End().DeprecatedEditingOffset() == 2);
      CHECK(
        selection.Modify(Alteration::Extend, SelectionDirection::Forward, TextGranularity::LineGranularity));
      CHECK(selection.Selection().End().DeprecatedNode() == &third);
      auto offset = selection.Selection().End().DeprecatedEditingOffset();
      CHECK(offset >= 9);
      CHECK(offset <= 11);
      CHECK(
        selection.Modify(Alteration::Extend, SelectionDirection::Backward, TextGranularity::LineGranularity));
      CHECK(selection.Selection().End().DeprecatedNode() == &second);
    }

    SECTION("Paragraphs")
    {
      selection.MoveTo(At(first, 2), Affinity::Downstream);
      CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Forward,
                             TextGranularity::ParagraphBoundary));
      CHECK(selection.Selection().End().DeprecatedNode() == &first);
      CHECK(selection.Selection().End().DeprecatedEditingOffset() == 19);
    }

    SECTION("The document")
    {
      selection.MoveTo(At(second, 1), Affinity::Downstream);
      CHECK(
        selection.Modify(Alteration::Extend, SelectionDirection::Forward, TextGranularity::DocumentBoundary));
      CHECK(selection.Selection().End().DeprecatedNode() == &third);
      CHECK(selection.Selection().End().DeprecatedEditingOffset() == 23);
      CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Backward,
                             TextGranularity::DocumentBoundary));
      CHECK(selection.Selection().Start().DeprecatedNode() == &first);
      CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 0);
    }

    SECTION("A vertical distance")
    {
      selection.MoveTo(At(first, 1), Affinity::Downstream);
      CHECK(selection.Modify(Alteration::Extend, 40, FrameSelection::VerticalDirection::Down));
      CHECK(selection.Selection().End().DeprecatedNode() == &third);
      CHECK(selection.Modify(Alteration::Extend, 20, FrameSelection::VerticalDirection::Up));
      CHECK(selection.Selection().End().DeprecatedNode() == &second);
      CHECK_FALSE(selection.Modify(Alteration::Extend, 0, FrameSelection::VerticalDirection::Up));
    }
  }

  TEST_CASE("FrameSelection: modifying as the user", "[Editing][FrameSelection]")
  {
    FrameSelectionFixture fixture(
      u8"<p id=a>The quick brown fox</p><div style=height:2000px></div><p id=z>The end</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"a");
    auto listener = CreateRef<ModifySelectStartListener>();
    fixture.Document->AddEventListener(dom::DOMStringAtom(u8"selectstart"),
                                       Ref<dom::EventListenerCallback> {listener}, true);

    SECTION("Extending a caret fires selectstart, which can cancel it")
    {
      selection.MoveTo(At(text, 4), Affinity::Downstream);
      listener->Cancel = true;
      CHECK_FALSE(selection.Modify(Alteration::Extend, SelectionDirection::Right,
                                   TextGranularity::CharacterGranularity, UserTriggered::Yes));
      CHECK(listener->Count == 1);
      CHECK(selection.IsCaret());

      listener->Cancel = false;
      CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Right,
                             TextGranularity::CharacterGranularity, UserTriggered::Yes));
      CHECK(listener->Count == 2);
      CHECK(selection.IsRange());

      // Extending a range doesn't fire it again.
      CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Right,
                             TextGranularity::CharacterGranularity, UserTriggered::Yes));
      CHECK(listener->Count == 2);
    }

    SECTION("The extent is revealed")
    {
      selection.MoveTo(At(text, 0), Affinity::Downstream);
      auto *view = fixture.Document->View();
      CHECK(view->GetScrollPosition().Y() == 0);
      CHECK(selection.Modify(Alteration::Extend, SelectionDirection::Forward,
                             TextGranularity::DocumentBoundary, UserTriggered::Yes));
      fixture.UpdateRendering();
      CHECK(selection.Selection().End().DeprecatedNode() == &fixture.TextIn(u8"z"));
      CHECK(view->GetScrollPosition().Y() > 1'500);
    }
  }
}

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A page showing @p markup (10px text on 20px lines, no body margin) in a 400x300 view.
    struct CaretFixture : PaintFixture
    {
      explicit CaretFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                           + utf8_string(markup),
                         {400, 300}, true)
      {
        TestPage->UpdateRendering();
      }

      KRYS_NODISCARD FrameSelection &Selection() noexcept
      {
        return Document->Selection();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }

      /// @brief Whether the view paints a caret: a rect 1px wide, filled with @p colour.
      KRYS_NODISCARD bool PaintsCaret(utf8_stringview colour) noexcept
      {
        auto painted = Painted();
        auto colourText = utf8_string(u8"(color ") + utf8_string(colour) + u8")";
        for (auto at = painted.find(u8"(fill-rect-with-color"); at != utf8_string::npos;
             at = painted.find(u8"(fill-rect-with-color", at + 1))
        {
          auto end = painted.find(u8"(fill-rect", at + 1);
          auto fill = painted.substr(at, end == utf8_string::npos ? utf8_string::npos : end - at);
          if (fill.find(u8" size 1x") != utf8_string::npos && fill.find(colourText) != utf8_string::npos)
          {
            return true;
          }
        }
        return false;
      }

      /// @brief Runs the event loop's timers up to @p delay from now, then the rendering update.
      void Advance(MonotonicTime::duration delay) noexcept
      {
        auto &loop = html::EventLoop::Current();
        loop.Run(loop.Now() + delay);
        TestPage->UpdateRendering();
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameSelection: the caret in editable content",
                   "[Editing][FrameSelection][Caret]")
  {
    CaretFixture fixture(u8"<div id=e contenteditable style=caret-color:#FF0000>The quick brown fox</div>"
                         u8"<p id=p>Not editable</p>");
    auto &selection = fixture.Selection();
    auto &text = fixture.TextIn(u8"e");

    SECTION("A caret in editable text has a rect, and paints in the caret colour")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 0), Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      CHECK(selection.IsCaretVisible());
      auto bounds = selection.AbsoluteCaretBounds();
      CHECK(bounds.X() == 0);
      // As tall as the text, not the line.
      CHECK(bounds.Height() == 11);
      CHECK(bounds.Width() == 1);
      CHECK(fixture.PaintsCaret(u8"#FF0000"));

      // Moving the caret moves its rect.
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 3), Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      CHECK(selection.AbsoluteCaretBounds().X() > 10);
    }

    SECTION("A caret outside editable text isn't painted")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&fixture.TextIn(u8"p")), 2),
                       Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      CHECK_FALSE(fixture.PaintsCaret(u8"#000000"));
    }

    SECTION("The caret blinks")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 2), Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      CHECK(selection.GetCaretAnimator().IsActive());
      CHECK(fixture.PaintsCaret(u8"#FF0000"));

      fixture.Advance(std::chrono::milliseconds(550));
      CHECK_FALSE(fixture.PaintsCaret(u8"#FF0000"));
      fixture.Advance(std::chrono::milliseconds(550));
      CHECK(fixture.PaintsCaret(u8"#FF0000"));

      // Moving the caret shows it again, and restarts the blinking.
      fixture.Advance(std::chrono::milliseconds(550));
      CHECK_FALSE(fixture.PaintsCaret(u8"#FF0000"));
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 4), Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      CHECK(fixture.PaintsCaret(u8"#FF0000"));

      // The event handler suspends the blinking while the mouse is down; the next blink, which takes its
      // suspension from the theme's interval, resumes it.
      selection.SetCaretBlinkingSuspended(true);
      CHECK(selection.IsCaretBlinkingSuspended());
      fixture.Advance(std::chrono::milliseconds(550));
      CHECK_FALSE(selection.IsCaretBlinkingSuspended());
    }

    SECTION("An unfocused document hides the caret")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 2), Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      fixture.Document->SetFocusedAndActive(false);
      fixture.TestPage->UpdateRendering();
      CHECK_FALSE(selection.IsCaretVisible());
      CHECK_FALSE(fixture.PaintsCaret(u8"#FF0000"));
      fixture.Document->SetFocusedAndActive(true);
      fixture.TestPage->UpdateRendering();
      CHECK(selection.IsCaretVisible());
      CHECK(fixture.PaintsCaret(u8"#FF0000"));
    }

    SECTION("The arrows move a caret in editable text")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 2), Affinity::Downstream);
      auto &editor = fixture.Document->GetEditor();
      CHECK(editor.GetCommand(u8"MoveRight").Execute());
      CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 3);
      CHECK(editor.GetCommand(u8"MoveWordRight").Execute());
      CHECK(editor.GetCommand(u8"MoveToEndOfLine").Execute());
      CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 19);
      CHECK(editor.GetCommand(u8"MoveToBeginningOfDocument").Execute());
      CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 0);
    }

    SECTION("Removing the text the caret is in")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&text), 2), Affinity::Downstream);
      fixture.TestPage->UpdateRendering();
      (void)fixture.ById(u8"e").RemoveChild(text);
      fixture.TestPage->UpdateRendering();
      CHECK(selection.IsCaret());
      CHECK(selection.Selection().Start().DeprecatedNode() == &fixture.ById(u8"e"));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameSelection: focusing an editing host",
                   "[Editing][FrameSelection][Caret]")
  {
    CaretFixture fixture(u8"<div id=e contenteditable>Editable</div><button id=b>Button</button>");
    auto &selection = fixture.Selection();
    auto &host = fixture.ById(u8"e");

    SECTION("focus() places a caret at its start")
    {
      Downcast<html::HTMLElement>(host).Focus();
      CHECK(fixture.Document->FocusedElement() == &host);
      CHECK(selection.IsCaret());
      CHECK(selection.Selection().RootEditableElement() == &host);
      CHECK(selection.Selection().VisibleStart() == VisiblePosition(FirstPositionInOrBeforeNode(&host)));
    }

    SECTION("Focusing something else leaves the selection")
    {
      Downcast<html::HTMLElement>(fixture.ById(u8"b")).Focus();
      CHECK(selection.IsNone());
    }

    SECTION("Placing the selection in it focuses it")
    {
      selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(fixture.ById(u8"e").FirstChild()), 3),
                       Affinity::Downstream);
      CHECK(fixture.Document->FocusedElement() == &host);
    }

    SECTION("Sequential navigation places a caret in it")
    {
      CHECK(html::FocusAlgorithms::SequentialNavigation(*fixture.Document, dom::FocusDirection::Forward));
      CHECK(fixture.Document->FocusedElement() == &host);
      CHECK(selection.IsCaret());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBlockFlow: room for the caret at the end of lines",
                   "[Editing][Caret]")
  {
    CaretFixture fixture(
      u8"<div id=clipped contenteditable style=overflow:hidden>Text</div>"
      u8"<div id=padded contenteditable style='overflow:hidden;padding-right:4px'>Text</div>"
      u8"<div id=visible contenteditable>Text</div><div id=plain style=overflow:hidden>Text</div>");
    auto padding = [&](dom::DOMStringView id)
    {
      return Downcast<RenderBlockFlow>(*fixture.ById(id).Renderer()).EndPaddingWidthForCaret();
    };
    CHECK(padding(u8"clipped") == LayoutUnit(CaretWidth()));
    CHECK(padding(u8"padded") == LayoutUnit(0));
    CHECK(padding(u8"visible") == LayoutUnit(0));
    CHECK(padding(u8"plain") == LayoutUnit(0));
  }
}
