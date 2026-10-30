#pragma once

#include "Krystal.Booey/Editing/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderObject;

  enum class CaretRectMode : bool
  {
    Normal,
    ExpandToEndOfLine
  };

  KRYS_NODISCARD int32 CaretWidth() noexcept;

  /// @brief The caret's rect for a position in @p renderer, in its coordinates.
  KRYS_NODISCARD LayoutRect
    ComputeLocalCaretRect(const RenderObject &renderer, const InlineBoxAndOffset &boxAndOffset,
                          CaretRectMode caretRectMode = CaretRectMode::Normal) noexcept;
}
