#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"

#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListItems.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/Fonts/Font.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo::tests
{
  PaintFixture::PaintFixture(utf8_string &&markup, IntSize viewSize, bool inPage) noexcept
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

  Ref<const display_list::DisplayList> PaintFixture::PaintedDisplayList() noexcept
  {
    (void)Document->UpdateLayout();
    display_list::RecorderImpl recorder(
      FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
    Frame->View()->PaintContents(recorder, IntRect(IntPoint(), ViewSize));
    return recorder.TakeDisplayList();
  }

  utf8_string PaintFixture::Painted() noexcept
  {
    return PaintedDisplayList()->AsText({});
  }

  utf8_string Painted(utf8_stringview markup) noexcept
  {
    PaintFixture fixture(u8"<!DOCTYPE html>" + utf8_string(markup));
    return fixture.Painted();
  }

  utf8_string BaseCanvas() noexcept
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

  utf8_string Canvas(utf8_stringview rootColour) noexcept
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

  utf8_string DrawnText(const display_list::DisplayList &displayList, utf8_stringview alphabet) noexcept
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
