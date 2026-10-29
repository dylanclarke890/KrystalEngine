#pragma once

#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo::tests
{
  /// @brief A parsed document shown in a frame with a small view, painted into a display list.
  struct PaintFixture : style::tests::ResolverFixture
  {
    IntSize ViewSize;
    /// @brief The page whose main frame shows the document, if it has one (filters paint only in a page).
    RefPtr<Page> TestPage;

    explicit PaintFixture(utf8_string &&markup, IntSize viewSize = {200, 100}, bool inPage = false) noexcept
        : ResolverFixture(krys::move(markup), style::tests::WithView::No), ViewSize(viewSize)
    {
      if (inPage)
      {
        TestPage = Page::Create();
        Frame = ShareRefPtr(&TestPage->MainFrame());
      }
      else
      {
        Frame = LocalFrame::Create();
      }
      Frame->CreateView(ViewSize);
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
    }

    /// @brief The display list of the whole view, laid out.
    KRYS_NODISCARD Ref<const display_list::DisplayList> PaintedDisplayList() noexcept
    {
      (void)Document->UpdateLayout();
      display_list::RecorderImpl recorder(
        FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
      Frame->View()->PaintContents(recorder, IntRect(IntPoint(), ViewSize));
      return recorder.TakeDisplayList();
    }

    /// @brief The display list of the whole view, laid out, as text.
    KRYS_NODISCARD utf8_string Painted() noexcept
    {
      return PaintedDisplayList()->AsText({});
    }
  };

  /// @brief The display list of @p markup, after a doctype, painted in a 200x100 view, as text.
  KRYS_NODISCARD inline utf8_string Painted(utf8_stringview markup) noexcept
  {
    PaintFixture fixture(u8"<!DOCTYPE html>" + utf8_string(markup));
    return fixture.Painted();
  }

  /// @brief The start of a 200x100 view's display list: the view's base colour.
  /// @note The dumps put a space after `(composite-mode`, kept outside the raw strings so tools that strip
  /// trailing whitespace can't drop it.
  KRYS_NODISCARD inline utf8_string BaseCanvas() noexcept
  {
    return u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode)"
           u8" "
           u8R"(
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF)))";
  }

  /// @brief The start of a 200x100 view's display list: the view's base colour, then the root's background of
  /// @p rootColour over the canvas.
  KRYS_NODISCARD inline utf8_string Canvas(utf8_stringview rootColour = u8"#FFFFFF") noexcept
  {
    return BaseCanvas()
           + u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode)"
             u8" "
             u8R"(
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color )" + utf8_string(rootColour)
           + u8R"()
  (composite-operation copy)
  (blend-mode normal)))";
  }
  /// @brief The text @p displayList's glyph runs draw, in drawing order, read back through each run's font as
  /// characters of @p alphabet; runs are separated by `|`. A glyph that's none of @p alphabet's characters
  /// reads as `?`, and runs with none of them are left out.
  /// @note Glyphs are drawn left to right, so right-to-left text drawn in visual order reads reversed.
  KRYS_NODISCARD inline utf8_string DrawnText(const display_list::DisplayList &displayList,
                                              utf8_stringview alphabet) noexcept
  {
    utf8_string text;
    for (const auto &item : displayList.Items())
    {
      auto *glyphs = std::get_if<display_list::DrawGlyphs>(&item);
      if (!glyphs)
      {
        continue;
      }
      auto font = glyphs->GetFont();
      utf8_string run;
      bool known = false;
      for (auto glyph : glyphs->Glyphs())
      {
        utf8_stringview character = u8"?";
        for (text::CodePointIterator it {alphabet}; !it.AtEnd(); ++it)
        {
          if (font->GlyphForCharacter(*it) == glyph)
          {
            auto next = it;
            ++next;
            character = alphabet.substr(it.CodeUnitsSince(alphabet.data()), next.CodeUnitsSince(it));
            known = true;
            break;
          }
        }
        run += character;
      }
      if (known)
      {
        text += (text.empty() ? u8"" : u8"|") + run;
      }
    }
    return text;
  }
}
