#pragma once

#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  struct PaintInfo;

  /// @brief Paints a line's ellipsis (`text-overflow: ellipsis`, `line-clamp`) in the line's text colour and
  /// first text shadow.
  /// FIXME: The selection's background and colour (paintSelection, ellipsisSelectionState) come with
  /// selection.
  class EllipsisBoxPainter
  {
    KRYS_NON_COPY_MOVABLE(EllipsisBoxPainter);

  private:
    const inline_iterator::LineBox &_lineBox;
    PaintInfo &_paintInfo;
    LayoutPoint _paintOffset;

  public:
    EllipsisBoxPainter(const inline_iterator::LineBox &lineBox, PaintInfo &paintInfo,
                       const LayoutPoint &paintOffset) noexcept;

    void Paint() noexcept;
  };
}
