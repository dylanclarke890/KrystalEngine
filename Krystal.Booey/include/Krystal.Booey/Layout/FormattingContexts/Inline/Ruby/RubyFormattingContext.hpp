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
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentBreaker.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLevelBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  class InlineFormattingContext;
  class LineBox;
  class Rect;

  /// @brief Ruby layout inside an inline formatting context: the room annotations take on the line, ruby
  /// alignment, annotation placement and sizing, the annotations' contribution to the line box and overhang.
  class RubyFormattingContext
  {
  public:
    // Line building
    KRYS_NODISCARD static InlineLayoutUnit
      AnnotationBoxLogicalWidth(const Box &rubyBaseLayoutBox,
                                InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      BaseEndAdditionalLogicalWidth(const Box &rubyBaseLayoutBox, const Line::RunList &lineRuns,
                                    const InlineContentBreaker::ContinuousContent::RunList &candidateRuns,
                                    InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static HashMap<const Box *, InlineLayoutUnit>
      ApplyRubyAlign(Line &line, InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      ApplyRubyAlignOnAnnotationBox(Line &line, InlineLayoutUnit spaceToDistribute,
                                    InlineFormattingContext &inlineFormattingContext) noexcept;

    // Line box building
    static void ApplyAnnotationContributionToLayoutBounds(
      LineBox &lineBox, const InlineFormattingContext &inlineFormattingContext) noexcept;

    // Display content building
    KRYS_NODISCARD static InlineLayoutUnit
      BaseEndAdditionalLogicalWidth(const Box &rubyBaseLayoutBox, const inline_display::Box &baseDisplayBox,
                                    InlineLayoutUnit baseContentWidth,
                                    InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutPoint
      PlaceAnnotationBox(const Box &rubyBaseLayoutBox, const Rect &rubyBaseMarginBoxLogicalRect,
                         InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutSize
      SizeAnnotationBox(const Box &rubyBaseLayoutBox, const Rect &rubyBaseMarginBoxLogicalRect,
                        InlineFormattingContext &inlineFormattingContext) noexcept;

    static void ApplyRubyOverhang(InlineFormattingContext &parentFormattingContext,
                                  InlineLayoutUnit lineLogicalHeight, inline_display::Boxes &displayBoxes,
                                  const List<IndexRange> &interlinearRubyColumnRangeList) noexcept;

    enum class RubyBasesMayNeedResizing : bool
    {
      No,
      Yes
    };
    static void ApplyAlignmentOffsetList(inline_display::Boxes &displayBoxes,
                                         const HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                         RubyBasesMayNeedResizing rubyBasesMayNeedResizing,
                                         InlineFormattingContext &inlineFormattingContext) noexcept;
    static void ApplyAnnotationAlignmentOffset(inline_display::Boxes &displayBoxes,
                                               InlineLayoutUnit alignmentOffset,
                                               InlineFormattingContext &inlineFormattingContext) noexcept;

    // Miscellaneous helpers
    KRYS_NODISCARD static bool HasInterlinearAnnotation(const Box &rubyBaseLayoutBox) noexcept;
    KRYS_NODISCARD static bool HasInterCharacterAnnotation(const Box &rubyBaseLayoutBox) noexcept;

  private:
    using MaximumLayoutBoundsStretchMap = HashMap<const InlineLevelBox *, InlineLevelBox::AscentAndDescent>;
    static void AdjustLayoutBoundsAndStretchAncestorRubyBase(
      LineBox &lineBox, InlineLevelBox &rubyBaseInlineBox, MaximumLayoutBoundsStretchMap &descendantRubySet,
      const InlineFormattingContext &inlineFormattingContext) noexcept;

    KRYS_NODISCARD static size_t
      ApplyRubyAlignOnBaseContent(size_t rubyBaseStart, Line &line,
                                  HashMap<const Box *, InlineLayoutUnit> &alignmentOffsetList,
                                  InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      OverhangForAnnotationBefore(const Box &rubyBaseLayoutBox, size_t rubyBaseStart,
                                  const inline_display::Boxes &boxes, InlineLayoutUnit lineLogicalHeight,
                                  InlineFormattingContext &inlineFormattingContext) noexcept;
    KRYS_NODISCARD static InlineLayoutUnit
      OverhangForAnnotationAfter(const Box &rubyBaseLayoutBox, IndexRange rubyBaseRange,
                                 const inline_display::Boxes &boxes, InlineLayoutUnit lineLogicalHeight,
                                 InlineFormattingContext &inlineFormattingContext) noexcept;
  };
}
