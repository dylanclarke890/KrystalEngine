/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo::layout
{
  class Box;
  class InlineFormattingContext;

  /// @brief Distributes the free space of a line between its runs: `text-align: justify` and `ruby-align`,
  /// and moves display boxes by the resulting alignment offsets.
  class InlineContentAligner
  {
  public:
    KRYS_NODISCARD static InlineLayoutUnit
      ApplyTextAlignJustify(Line::RunList &runs, InlineLayoutUnit spaceToDistribute,
                            size_t hangingTrailingWhitespaceLength) noexcept;

    KRYS_NODISCARD static InlineLayoutUnit ApplyRubyAlign(RubyAlign rubyAlign, Line::RunList &runs,
                                                          IndexRange range,
                                                          InlineLayoutUnit spaceToDistribute) noexcept;

    enum class AdjustContentOnlyInsideRubyBase : bool
    {
      No,
      Yes
    };
    static void
      ApplyRubyBaseAlignmentOffset(inline_display::Boxes &displayBoxes,
                                   const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                   AdjustContentOnlyInsideRubyBase adjustContentOnlyInsideRubyBase,
                                   InlineFormattingContext &inlineFormattingContext) noexcept;
    static void ApplyRubyAnnotationAlignmentOffset(inline_display::Boxes &displayBoxes,
                                                   InlineLayoutUnit alignmentOffset,
                                                   InlineFormattingContext &inlineFormattingContext) noexcept;

  private:
    KRYS_NODISCARD static InlineLayoutUnit ApplyExpansionOnRange(Line::RunList &runs, IndexRange range,
                                                                 const ExpansionInfo &expansion,
                                                                 InlineLayoutUnit spaceToDistribute) noexcept;
  };
}
