/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <algorithm>
#include <span>
#include <utility>

namespace krys::boo
{
  class RenderText;
}

namespace krys::boo::inline_iterator
{
  /// @brief A text renderer's boxes in logical (text) order, when bidi reordering moved them out of it.
  struct TextLogicalOrderCacheData
  {
    List<TextBoxIterator> Boxes;
    size_t Index {0};
  };
  using TextLogicalOrderCache = UniquePtr<TextLogicalOrderCacheData>;

  /// @brief The first of a text renderer's boxes in logical order, and the cache to walk the rest with.
  KRYS_NODISCARD std::pair<TextBoxIterator, TextLogicalOrderCache>
    FirstTextBoxInLogicalOrderFor(const RenderText &text) noexcept;
  KRYS_NODISCARD TextBoxIterator NextTextBoxInLogicalOrder(const TextBoxIterator &textBox,
                                                           TextLogicalOrderCache &cache) noexcept;

  /// @brief A line's leaf boxes in logical order.
  struct LineLogicalOrderCacheData
  {
    LineBoxIterator LineBox;
    List<LeafBoxIterator> Boxes;
    size_t Index {0};
  };
  using LineLogicalOrderCache = UniquePtr<LineLogicalOrderCacheData>;

  KRYS_NODISCARD LeafBoxIterator FirstLeafOnLineInLogicalOrder(const LineBoxIterator &lineBox,
                                                               LineLogicalOrderCache &cache) noexcept;
  KRYS_NODISCARD LeafBoxIterator LastLeafOnLineInLogicalOrder(const LineBoxIterator &lineBox,
                                                              LineLogicalOrderCache &cache) noexcept;
  KRYS_NODISCARD LeafBoxIterator NextLeafOnLineInLogicalOrder(const LeafBoxIterator &box,
                                                              LineLogicalOrderCache &cache) noexcept;
  KRYS_NODISCARD LeafBoxIterator PreviousLeafOnLineInLogicalOrder(const LeafBoxIterator &box,
                                                                  LineLogicalOrderCache &cache) noexcept;

  KRYS_NODISCARD LeafBoxIterator FirstLeafOnLineInLogicalOrderWithNode(const LineBoxIterator &lineBox,
                                                                       LineLogicalOrderCache &cache) noexcept;
  KRYS_NODISCARD LeafBoxIterator LastLeafOnLineInLogicalOrderWithNode(const LineBoxIterator &lineBox,
                                                                      LineLogicalOrderCache &cache) noexcept;

  /// @brief A line's leaf boxes, put back in logical order by undoing the bidi reordering with
  /// @p reverseFunction (given each run of boxes to reverse).
  template <typename TReverseFunction>
  KRYS_NODISCARD List<LeafBoxIterator> LeafBoxesInLogicalOrder(const LineBoxIterator &lineBox,
                                                               TReverseFunction &&reverseFunction) noexcept
  {
    List<LeafBoxIterator> boxes;

    uint8 minLevel = 128;
    uint8 maxLevel = 0;

    for (auto box = lineBox->LineLeftmostLeafBox(); box; box = box.TraverseLineRightwardOnLine())
    {
      minLevel = std::min<uint8>(minLevel, box->BidiLevel());
      maxLevel = std::max<uint8>(maxLevel, box->BidiLevel());
      boxes.push_back(box);
    }

    // Reverse of reordering of the line (L2 according to Bidi spec):
    // L2. From the highest level found in the text to the lowest odd level on each line,
    // reverse any contiguous sequence of characters that are at that level or higher.

    // Reversing the reordering of the line is only done up to the lowest odd level.
    if (!(minLevel % 2))
    {
      ++minLevel;
    }

    auto boxCount = boxes.size();
    for (; minLevel <= maxLevel; ++minLevel)
    {
      size_t boxIndex = 0;
      while (boxIndex < boxCount)
      {
        while (boxIndex < boxCount && boxes[boxIndex]->BidiLevel() < minLevel)
        {
          ++boxIndex;
        }

        auto first = boxIndex;
        while (boxIndex < boxCount && boxes[boxIndex]->BidiLevel() >= minLevel)
        {
          ++boxIndex;
        }

        reverseFunction(std::span(boxes).subspan(first, boxIndex - first));
      }
    }

    return boxes;
  }
}
