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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineBox.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::layout
{
  class BlockLayoutState;
  class ElementBox;
  class InlineFormattingContext;
  class InlineLayoutState;

  /// @brief Builds the line box for a laid out line: constructs its inline level boxes, sets their vertical
  /// metrics and computes the line box geometry.
  class LineBoxBuilder
  {
  private:
    const InlineFormattingContext &_inlineFormattingContext;
    LineLayoutResult &_lineLayoutResult;
    bool _fallbackFontRequiresIdeographicBaseline {false};
    bool _lineHasNonLineSpanningRubyContent {false};
    HashMap<const InlineLevelBox *, TextUtil::FallbackFontList> _fallbackFontsForInlineBoxes;
    List<size_t> _outsideListMarkers;

  public:
    LineBoxBuilder(const InlineFormattingContext &inlineFormattingContext,
                   LineLayoutResult &lineLayoutResult) noexcept;

    KRYS_NODISCARD LineBox Build(size_t lineIndex) noexcept;

  private:
    void SetVerticalPropertiesForInlineLevelBox(const LineBox &lineBox,
                                                InlineLevelBox &inlineLevelBox) const noexcept;
    void SetLayoutBoundsForInlineBox(InlineLevelBox &inlineBox, FontBaseline fontBaseline) const noexcept;
    void AdjustInlineBoxHeightsForLineBoxContainIfApplicable(LineBox &lineBox) noexcept;
    void ComputeLineBoxGeometry(LineBox &lineBox) const noexcept;
    KRYS_NODISCARD InlineLevelBox::AscentAndDescent
      EnclosingAscentDescentWithFallbackFonts(const InlineLevelBox &inlineBox,
                                              const TextUtil::FallbackFontList &fallbackFontsForContent,
                                              FontBaseline fontBaseline) const noexcept;
    KRYS_NODISCARD TextUtil::FallbackFontList CollectFallbackFonts(const InlineLevelBox &parentInlineBox,
                                                                   const Line::Run &run,
                                                                   const RenderStyle &style) noexcept;
    void AdjustMarginStartForListMarker(const ElementBox &listMarkerBox,
                                        LayoutUnit nestedListMarkerMarginStart,
                                        InlineLayoutUnit rootInlineBoxOffset) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit ApplyTextBoxTrimOnLineBoxIfNeeded(InlineLayoutUnit lineBoxLogicalHeight,
                                                                      LineBox &lineBox) const noexcept;
    void ConstructInlineLevelBoxes(LineBox &lineBox) noexcept;
    void ConstructBlockContent(LineBox &lineBox) noexcept;
    void AdjustIdeographicBaselineIfApplicable(LineBox &lineBox) noexcept;
    void AdjustOutsideListMarkersPosition(LineBox &lineBox) noexcept;

    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return GetLineLayoutResult().FirstLast.FirstFormattedLine == IsFirstFormattedLine::Yes;
    }
    KRYS_NODISCARD bool IsLastLine() const noexcept
    {
      return GetLineLayoutResult().FirstLast.IsLastLineWithInlineContent;
    }

    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const LineLayoutResult &GetLineLayoutResult() const noexcept
    {
      return _lineLayoutResult;
    }
    KRYS_NODISCARD const ElementBox &RootBox() const noexcept;
    KRYS_NODISCARD const RenderStyle &RootStyle() const noexcept;
    KRYS_NODISCARD const InlineLayoutState &GetLayoutState() const noexcept;
    KRYS_NODISCARD const BlockLayoutState &GetBlockLayoutState() const noexcept;
  };
}
