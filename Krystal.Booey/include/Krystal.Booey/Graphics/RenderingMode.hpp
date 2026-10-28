#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief What an image buffer is made for.
  enum class RenderingPurpose : uint8
  {
    Unspecified,
    Canvas,
    DOM,
    LayerBacking,
    Snapshot,
    ShareableSnapshot,
    ShareableLocalSnapshot,
    MediaPainting,
  };

  /// @brief Where an image buffer's drawing goes: pixels in memory, a GPU surface, or a display list that a
  /// drawing backend renders when it draws the buffer.
  enum class RenderingMode : uint8
  {
    Unaccelerated,
    Accelerated,
    DisplayList,
  };

  TextStream &operator<<(TextStream &, RenderingPurpose) noexcept;
  TextStream &operator<<(TextStream &, RenderingMode) noexcept;
}
