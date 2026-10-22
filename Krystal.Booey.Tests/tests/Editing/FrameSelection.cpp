#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
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
