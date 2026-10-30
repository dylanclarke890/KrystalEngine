#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo
{
  /// @brief The part of a text renderer's content a text box shows, for clamping offsets (selection, marked
  /// text) to the box: byte offsets into the renderer's text.
  struct TextBoxSelectableRange
  {
    const uint32 Start;
    const uint32 Length;
    const uint32 AdditionalLengthAtEnd {0};
    const bool IsLineBreak {false};
    // FIXME: Consider holding onto the truncation position instead. See webkit.org/b/164999
    const Maybe<uint32> Truncation {};
    // where containing block's bidi direction != text box's direction.
    const bool HasMismatchingContentDirection {false};

    /// @brief @p offset in the renderer's text, as an offset into the box's text.
    KRYS_NODISCARD uint32 Clamp(uint32 offset) const noexcept
    {
      auto clampedOffset = std::clamp(offset, Start, Start + Length) - Start;

      if (HasMismatchingContentDirection && Truncation && *Truncation)
      {
        return std::max<uint32>(clampedOffset, Length - *Truncation);
      }

      // FIXME: For some reason we allow the caret move to (invisible) fully truncated text. The zero test is
      // to keep that behavior.
      if (Truncation && *Truncation)
      {
        return std::min<uint32>(clampedOffset, *Truncation);
      }

      if (clampedOffset == Length)
      {
        clampedOffset += AdditionalLengthAtEnd;
      }

      return clampedOffset;
    }

    KRYS_NODISCARD std::pair<uint32, uint32> Clamp(uint32 startOffset, uint32 endOffset) const noexcept
    {
      return {Clamp(startOffset), Clamp(endOffset)};
    }

    KRYS_NODISCARD bool Intersects(uint32 startOffset, uint32 endOffset) const noexcept
    {
      return Clamp(startOffset) < Clamp(endOffset);
    }
  };
}
