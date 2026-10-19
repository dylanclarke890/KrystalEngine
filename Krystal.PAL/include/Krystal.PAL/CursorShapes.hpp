#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::pal
{
  /// @brief The system cursors a window can show over its client area.
  enum class CursorShape : uint8
  {
    /// @brief The normal arrow.
    Arrow,
    /// @brief The text selection I-beam.
    IBeam,
    /// @brief The pointing hand, over links and buttons.
    Hand,
    Crosshair,
    /// @brief Busy: the application can't take input.
    Wait,
    /// @brief Working in the background: the arrow with a busy indicator.
    Progress,
    /// @brief The arrow with a question mark.
    Help,
    NotAllowed,
    /// @brief Arrows in four directions.
    Move,
    /// @brief Resizing up or down.
    ResizeNS,
    /// @brief Resizing left or right.
    ResizeEW,
    /// @brief Resizing towards the top right or the bottom left.
    ResizeNESW,
    /// @brief Resizing towards the top left or the bottom right.
    ResizeNWSE,
    /// @brief No cursor.
    Hidden,
  };
}
