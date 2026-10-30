#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayLine.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::inline_display
{
  using Boxes = List<Box>;
  using Lines = List<Line>;

  /// @brief The lines and boxes of laid out inline content.
  struct Content
  {
    void Clear() noexcept;
    void Set(Content &&newContent) noexcept;
    void Append(Content &&newContent) noexcept;
    void Insert(Content &&newContent, size_t lineIndex, size_t boxIndex) noexcept;
    void Remove(size_t firstLineIndex, size_t numberOfLines, size_t firstBoxIndex,
                size_t numberOfBoxes) noexcept;

    Lines LineList;
    Boxes BoxList;
  };
}
