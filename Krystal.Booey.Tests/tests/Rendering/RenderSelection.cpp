#include "Krystal.Booey/Rendering/RenderSelection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using HighlightState = RenderObject::HighlightState;

    struct RenderSelectionFixture : PaintFixture
    {
      explicit RenderSelectionFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}p{margin:0}</style>"
                           + utf8_string(markup),
                         {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }

      KRYS_NODISCARD RenderSelection &Selection() noexcept
      {
        return Document->GetRenderView()->Selection();
      }

      void Select(dom::Node &start, uint32 startOffset, dom::Node &end, uint32 endOffset) noexcept
      {
        Document->Selection().MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&start), startOffset),
                                     dom::MakeContainerOffsetPosition(ShareRefPtr(&end), endOffset),
                                     Affinity::Downstream);
        (void)Document->UpdateLayout();
        Document->Selection().UpdateAppearanceAfterUpdatingRendering();
      }
    };
  }

  TEST_CASE("RenderSelection: selection states", "[Rendering][RenderSelection]")
  {
    RenderSelectionFixture fixture(
      u8"<p id=a>First line</p><p id=m>Middle</p><p id=b style='padding-left:20px'>Last line</p>");
    auto &first = *fixture.TextIn(u8"a").Renderer();
    auto &middle = *fixture.TextIn(u8"m").Renderer();
    auto &last = *fixture.TextIn(u8"b").Renderer();

    fixture.Select(fixture.TextIn(u8"a"), 6, fixture.TextIn(u8"b"), 4);
    CHECK(first.SelectionState() == HighlightState::Start);
    CHECK(middle.SelectionState() == HighlightState::Inside);
    CHECK(last.SelectionState() == HighlightState::End);

    // Blocks take their descendants' states: the paragraphs theirs, the body both ends.
    CHECK(fixture.ById(u8"a").Renderer()->SelectionState() == HighlightState::Start);
    CHECK(fixture.ById(u8"m").Renderer()->SelectionState() == HighlightState::Inside);
    CHECK(fixture.ById(u8"b").Renderer()->SelectionState() == HighlightState::End);
    CHECK(fixture.Document->Body()->Renderer()->SelectionState() == HighlightState::Both);
    CHECK(first.IsSelectionBorder());
    CHECK(!middle.IsSelectionBorder());

    // Text boxes: the part of the renderer's selection in each.
    auto firstBox = *inline_iterator::LineLeftmostTextBoxFor(first);
    CHECK(fixture.Selection().HighlightStateForTextBox(first, firstBox.SelectableRange())
          == HighlightState::Start);
    auto [start, end] = fixture.Selection().RangeForTextBox(first, firstBox.SelectableRange());
    CHECK(start == 6);
    CHECK(end == 10);

    // The selection's bounds span the three lines, from the first's selected part to the last's: a line's
    // selection runs from its text's top (or the line above's bottom) to its text's bottom.
    auto bounds = fixture.Selection().Bounds();
    CHECK(bounds.Y() > 0);
    CHECK(bounds.Y() < 10);
    CHECK(bounds.MaxY() > 50);
    CHECK(bounds.MaxY() <= 60);

    // The body, a selection root, fills the gaps: right of the first line, left of the last.
    auto *body = Downcast<RenderBlock>(fixture.Document->Body()->Renderer());
    CHECK(body->IsSelectionRoot());
    CHECK(body->ShouldPaintSelectionGaps());
    auto gaps = body->SelectionGapRectsForRepaint(nullptr);
    CHECK(!gaps.Right().IsEmpty());
    CHECK(gaps.Right().MaxX() == 400);
    CHECK(!gaps.Left().IsEmpty());
    CHECK(gaps.Left().X() == 0);
    CHECK(gaps.Left().MaxX() == 20);
    CHECK(!fixture.ById(u8"m").Renderer()->ShouldPaintSelectionGaps());

    fixture.Document->Selection().Clear();
    fixture.Document->Selection().UpdateAppearanceAfterUpdatingRendering();
    CHECK(first.SelectionState() == HighlightState::None);
    CHECK(middle.SelectionState() == HighlightState::None);
    CHECK(fixture.Document->Body()->Renderer()->SelectionState() == HighlightState::None);
    CHECK(fixture.Selection().Bounds().IsEmpty());
  }

  TEST_CASE("RenderSelection: within one text", "[Rendering][RenderSelection]")
  {
    RenderSelectionFixture fixture(u8"<p id=a>Some words here</p>");
    auto &text = *fixture.TextIn(u8"a").Renderer();

    fixture.Select(fixture.TextIn(u8"a"), 5, fixture.TextIn(u8"a"), 10);
    CHECK(text.SelectionState() == HighlightState::Both);
    auto box = *inline_iterator::LineLeftmostTextBoxFor(text);
    CHECK(fixture.Selection().HighlightStateForTextBox(text, box.SelectableRange()) == HighlightState::Both);
    auto [start, end] = fixture.Selection().RangeForTextBox(text, box.SelectableRange());
    CHECK(start == 5);
    CHECK(end == 10);

    // The selected words' rect: within the line, not from its start.
    auto rect = text.SelectionRectForRepaint(nullptr, false);
    CHECK(rect.X() > 0);
    CHECK(rect.Y() > 0);
    CHECK(rect.MaxY() <= 20);
    CHECK(rect.Width() > 0);
    CHECK(rect.MaxX() < 100);
  }

  TEST_CASE("RenderSelection: replaced elements", "[Rendering][RenderSelection]")
  {
    RenderSelectionFixture fixture(u8"<p id=a>Before <img id=i style='width:30px;height:20px'> after</p>");
    auto &text = fixture.TextIn(u8"a");
    auto *image = Downcast<RenderReplaced>(fixture.ById(u8"i").Renderer());
    REQUIRE(image);

    auto &after = Downcast<dom::Text>(*fixture.ById(u8"a").LastChild());
    fixture.Select(text, 2, after, 3);
    CHECK(image->SelectionState() == HighlightState::Inside);
    CHECK(image->IsSelected());
    CHECK(image->LocalSelectionRect() == LayoutRect(0_lu, 0_lu, 30_lu, 20_lu));

    fixture.Select(text, 2, text, 4);
    CHECK(!image->IsSelected());
    CHECK(image->LocalSelectionRect().IsEmpty());
  }
}

namespace krys::boo::tests
{
  TEST_CASE("RenderSelection: painting", "[Rendering][RenderSelection]")
  {
    RenderSelectionFixture fixture(u8"<style>::selection{background:#00FF00;color:#FF0000}</style>"
                                   u8"<p id=a>First line</p><p id=b>Last line</p>");
    auto unselected = fixture.Painted();
    CHECK(unselected.find(u8"#00FF00") == utf8_string::npos);
    CHECK(unselected.find(u8"#FF0000") == utf8_string::npos);

    fixture.Select(fixture.TextIn(u8"a"), 6, fixture.TextIn(u8"b"), 4);
    auto selected = fixture.Painted();
    // The selected text's background and the body's gaps are filled with the selection's background (made
    // translucent, so that it looks the same over white), and the selected text is painted in its colour.
    CHECK(selected.find(u8"(color #00FF00CC)") != utf8_string::npos);
    CHECK(selected.find(u8"#FF0000") != utf8_string::npos);

    // Without focus, the theme's selection colours give way to the inactive ones; ::selection's stay.
    fixture.Document->SetFocusedAndActive(false);
    CHECK(fixture.Painted().find(u8"(color #00FF00CC)") != utf8_string::npos);
  }

  TEST_CASE("RenderSelection: the theme's colours", "[Rendering][RenderSelection]")
  {
    RenderSelectionFixture fixture(u8"<p id=a>Some words</p>");
    fixture.Select(fixture.TextIn(u8"a"), 0, fixture.TextIn(u8"a"), 4);
    auto active = fixture.Painted();
    // Active: the accent colour at 30%, made translucent so that it looks the same over white; the text keeps
    // its colour (Adwaita has no selection foreground colours).
    auto &theme = RenderTheme::Singleton();
    auto activeBackground =
      SerialisationForRenderTreeAsText(theme.ActiveSelectionBackgroundColor(style::ColorOptions::None));
    auto overWhite =
      BlendSourceOver(Color::white, theme.ActiveSelectionBackgroundColor(style::ColorOptions::None))
        .ToColorTypeLossy<SRGBA<uint8>>()
        .Resolved();
    CHECK(std::abs(static_cast<int32>(overWhite.Blue) - 247) <= 2);
    CHECK(std::abs(static_cast<int32>(overWhite.Red) - 194) <= 2);
    CHECK(active.find(u8"(color " + activeBackground + u8")") != utf8_string::npos);

    fixture.Document->SetFocusedAndActive(false);
    auto inactive = fixture.Painted();
    auto inactiveBackground =
      SerialisationForRenderTreeAsText(theme.InactiveSelectionBackgroundColor(style::ColorOptions::None));
    CHECK(inactive.find(activeBackground) == utf8_string::npos);
    CHECK(inactive.find(u8"(color " + inactiveBackground + u8")") != utf8_string::npos);
  }
}
