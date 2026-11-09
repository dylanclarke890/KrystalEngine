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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayContent.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class Box;
  class InlineFormattingContext;
  class InlineLayoutState;
  class LineBox;

  /// @brief Builds the display line for a line box, and applies line ending truncation (ellipsis) to display
  /// lines.
  class InlineDisplayLineBuilder
  {
    struct EnclosingLineGeometry
    {
      inline_display::Line::EnclosingTopAndBottom EnclosingTopAndBottom;
      InlineRect ContentOverflowRect;
    };

  private:
    InlineFormattingContext &_inlineFormattingContext;
    const ConstraintsForInlineContent &_constraints;

  public:
    InlineDisplayLineBuilder(InlineFormattingContext &inlineFormattingContext,
                             const ConstraintsForInlineContent &constraints) noexcept;

    KRYS_NODISCARD inline_display::Line Build(const LineLayoutResult &lineLayoutResult,
                                              const LineBox &lineBox,
                                              bool lineIsFullyTruncatedInBlockDirection) const noexcept;

    static void ApplyEllipsisIfNeeded(LineEndingTruncationPolicy truncationPolicy,
                                      inline_display::Line &displayLine,
                                      inline_display::Boxes &displayBoxes) noexcept;
    KRYS_NODISCARD static bool
      HasTrailingLineWithBlockContent(const inline_display::Lines &displayLines) noexcept;

  private:
    KRYS_NODISCARD EnclosingLineGeometry
      CollectEnclosingLineGeometry(const LineLayoutResult &lineLayoutResult, const LineBox &lineBox,
                                   const InlineRect &lineBoxRect) const noexcept;

    KRYS_NODISCARD const ConstraintsForInlineContent &GetConstraints() const noexcept
    {
      return _constraints;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD InlineFormattingContext &GetFormattingContext() noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const Box &Root() const noexcept;
  };
}
