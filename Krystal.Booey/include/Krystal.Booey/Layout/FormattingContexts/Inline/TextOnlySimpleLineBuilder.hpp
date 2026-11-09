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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/AbstractLineBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class PlacedFloats;
  struct CandidateTextContent;
  struct TextOnlyLineBreakResult;

  /// @brief A faster line builder for content that is only text and line breaks with simple style.
  class TextOnlySimpleLineBuilder final : public AbstractLineBuilder
  {
  private:
    InlineLayoutUnit _trimmedTrailingWhitespaceWidth {0.f};
    Maybe<InlineLayoutUnit> _overflowContentLogicalWidth {};

  public:
    TextOnlySimpleLineBuilder(InlineFormattingContext &inlineFormattingContext, const ElementBox &rootBox,
                              HorizontalConstraints rootHorizontalConstraints,
                              const InlineItemList &inlineItemList) noexcept;
    KRYS_NODISCARD LineLayoutResult LayoutInlineContent(const LineInput &lineInput,
                                                        const Maybe<PreviousLine> &previousLine,
                                                        bool isFirstFormattedLineCandidate) noexcept final;

    KRYS_NODISCARD static bool
      IsEligibleForSimplifiedTextOnlyInlineLayoutByContent(const InlineContentCache::InlineItems &inlineItems,
                                                           const PlacedFloats &placedFloats) noexcept;
    KRYS_NODISCARD static bool IsEligibleForSimplifiedInlineLayoutByStyle(const Box &box) noexcept;

  private:
    KRYS_NODISCARD InlineItemPosition PlaceInlineTextContent(const RenderStyle &rootStyle,
                                                             const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD InlineItemPosition PlaceNonWrappingInlineTextContent(
      const RenderStyle &rootStyle, const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD Maybe<LineLayoutResult>
      PlaceSingleCharacterContentIfApplicable(const LineInput &lineInput,
                                              bool isFirstFormattedLineCandidate) noexcept;
    KRYS_NODISCARD TextOnlyLineBreakResult HandleOverflowingTextContent(
      const RenderStyle &rootStyle, const InlineContentBreaker::ContinuousContent &candidateContent,
      const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD TextOnlyLineBreakResult
      CommitCandidateContent(const RenderStyle &rootStyle, const CandidateTextContent &candidateContent,
                             const InlineItemRange &layoutRange) noexcept;
    void Initialize(const InlineItemRange &layoutRange, const InlineRect &initialLogicalRect,
                    const Maybe<PreviousLine> &previousLine, bool isFirstFormattedLineCandidate) noexcept;
    void HandleLineEnding(const RenderStyle &rootStyle, InlineItemPosition placedContentEnd,
                          size_t layoutRangeEndIndex) noexcept;
    KRYS_NODISCARD size_t RevertToTrailingItem(const RenderStyle &rootStyle,
                                               const InlineItemRange &layoutRange,
                                               const InlineTextItem &trailingInlineItem) noexcept;
    KRYS_NODISCARD size_t RevertToLastNonOverflowingItem(const RenderStyle &rootStyle,
                                                         const InlineItemRange &layoutRange) noexcept;
    KRYS_NODISCARD InlineLayoutUnit AvailableWidth() const noexcept;
  };
}
