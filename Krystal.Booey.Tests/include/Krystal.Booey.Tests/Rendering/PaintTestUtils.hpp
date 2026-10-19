#pragma once

#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"

namespace krys::boo::tests
{
  /// @brief A parsed document shown in a frame with a small view, painted into a display list.
  struct PaintFixture : style::tests::ResolverFixture
  {
    IntSize ViewSize;

    explicit PaintFixture(utf8_string &&markup, IntSize viewSize = {200, 100}) noexcept
        : ResolverFixture(krys::move(markup), style::tests::WithView::No), ViewSize(viewSize)
    {
      Frame = LocalFrame::Create();
      Frame->CreateView(ViewSize);
      Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
    }

    /// @brief The display list of the whole view, laid out, as text.
    KRYS_NODISCARD utf8_string Painted() noexcept
    {
      (void)Document->UpdateLayout();
      display_list::RecorderImpl recorder(
        FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
      Frame->View()->PaintContents(recorder, IntRect(IntPoint(), ViewSize));
      return recorder.TakeDisplayList()->AsText({});
    }
  };

  /// @brief The display list of @p markup, after a doctype, painted in a 200x100 view, as text.
  KRYS_NODISCARD inline utf8_string Painted(utf8_stringview markup) noexcept
  {
    PaintFixture fixture(u8"<!DOCTYPE html>" + utf8_string(markup));
    return fixture.Painted();
  }

  /// @brief The start of a 200x100 view's display list: the view's base colour.
  KRYS_NODISCARD inline utf8_string BaseCanvas() noexcept
  {
    return u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode
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
    return BaseCanvas() + u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color )" + utf8_string(rootColour)
           + u8R"()
  (composite-operation copy)
  (blend-mode normal)))";
  }
}
