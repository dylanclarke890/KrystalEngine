#pragma once

#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Graphics/DisplayLists/DisplayList.hpp"

namespace krys::web
{
  class Frame;
}

namespace krys::web::tests
{
  /// @brief A parsed document shown in a frame with a small view, painted into a display list.
  struct PaintFixture : style::tests::ResolverFixture
  {
    SizeI ViewSize;
    explicit PaintFixture(utf8_string &&markup, SizeI viewSize = {200, 100}) noexcept;

    /// @brief The display list of the whole view, laid out.
    KRYS_NODISCARD Ref<const display_list::DisplayList> PaintedDisplayList() noexcept;

    /// @brief The display list of the whole view, laid out, as text.
    KRYS_NODISCARD utf8_string Painted() noexcept;
  };

  /// @brief The display list of @p markup, after a doctype, painted in a 200x100 view, as text.
  KRYS_NODISCARD utf8_string Painted(utf8_stringview markup) noexcept;

  /// @brief The start of a 200x100 view's display list: the view's base colour.
  /// @note The dumps put a space after `(composite-mode`, kept outside the raw strings so tools that strip
  /// trailing whitespace can't drop it.
  KRYS_NODISCARD utf8_string BaseCanvas() noexcept;

  /// @brief The start of a 200x100 view's display list: the view's base colour, then the root's background of
  /// @p rootColour over the canvas.
  KRYS_NODISCARD utf8_string Canvas(utf8_stringview rootColour = u8"#FFFFFF") noexcept;

  /// @brief The text @p displayList's glyph runs draw, in drawing order, read back through each run's font as
  /// characters of @p alphabet; runs are separated by `|`. A glyph that's none of @p alphabet's characters
  /// reads as `?`, and runs with none of them are left out.
  /// @note Glyphs are drawn left to right, so right-to-left text drawn in visual order reads reversed.
  KRYS_NODISCARD utf8_string DrawnText(const display_list::DisplayList &displayList,
                                       utf8_stringview alphabet) noexcept;
}
