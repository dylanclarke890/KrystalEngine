/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/FormattingContexts/Grid/GridTypes.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MaximumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class UnplacedGridItem;

  /// @brief A grid item placed in the grid, with the computed sizes and alignment its layout needs.
  class PlacedGridItem
  {
  public:
    struct ComputedSizes
    {
      style::PreferredSize PreferredSize;
      style::MinimumSize MinimumSize;
      style::MaximumSize MaximumSize;
      style::MarginEdge MarginStart;
      style::MarginEdge MarginEnd;
    };

  private:
    const ElementBox *_layoutBox;
    ComputedSizes _inlineAxisSizes;
    ComputedSizes _blockAxisSizes;
    StyleSelfAlignmentData _inlineAxisAlignment;
    StyleSelfAlignmentData _blockAxisAlignment;
    style::ZoomFactor _usedZoom {1.0f, 1.0f};
    GridAreaLines _gridAreaLines;

  public:
    PlacedGridItem(const UnplacedGridItem &unplacedGridItem, GridAreaLines gridAreaLines,
                   const ComputedSizes &inlineAxisSizes, const ComputedSizes &blockAxisSizes,
                   const StyleSelfAlignmentData &inlineAxisAlignment,
                   const StyleSelfAlignmentData &blockAxisAlignment,
                   const style::ZoomFactor &usedZoom) noexcept;

    KRYS_NODISCARD const ComputedSizes &InlineAxisSizes() const noexcept
    {
      return _inlineAxisSizes;
    }
    KRYS_NODISCARD const ComputedSizes &BlockAxisSizes() const noexcept
    {
      return _blockAxisSizes;
    }

    KRYS_NODISCARD size_t ColumnStartLine() const noexcept
    {
      return _gridAreaLines.ColumnStartLine;
    }
    KRYS_NODISCARD size_t ColumnEndLine() const noexcept
    {
      return _gridAreaLines.ColumnEndLine;
    }
    KRYS_NODISCARD size_t RowStartLine() const noexcept
    {
      return _gridAreaLines.RowStartLine;
    }
    KRYS_NODISCARD size_t RowEndLine() const noexcept
    {
      return _gridAreaLines.RowEndLine;
    }

    KRYS_NODISCARD const ElementBox &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }

    KRYS_NODISCARD const StyleSelfAlignmentData &InlineAxisAlignment() const noexcept
    {
      return _inlineAxisAlignment;
    }
    KRYS_NODISCARD const StyleSelfAlignmentData &BlockAxisAlignment() const noexcept
    {
      return _blockAxisAlignment;
    }

    // FIXME: Add support for grid item's with preferred aspect ratios.
    KRYS_NODISCARD bool HasPreferredAspectRatio() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsReplacedElement() const noexcept
    {
      return _layoutBox->IsReplacedBox();
    }

    KRYS_NODISCARD const GridAreaLines &GetGridAreaLines() const noexcept
    {
      return _gridAreaLines;
    }
    KRYS_NODISCARD const style::ZoomFactor &UsedZoom() const noexcept
    {
      return _usedZoom;
    }
  };
}
