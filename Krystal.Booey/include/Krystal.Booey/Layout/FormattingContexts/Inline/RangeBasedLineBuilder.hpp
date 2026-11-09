/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Layout/FormattingContexts/Inline/TextOnlySimpleLineBuilder.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class PlacedFloats;

  /// @brief A line builder for text-only content wrapped in a single inline box (or inline boxes alone): it
  /// runs the text-only builder on the content and adds the inline box runs around it.
  class RangeBasedLineBuilder final : public AbstractLineBuilder
  {
  private:
    TextOnlySimpleLineBuilder _textOnlySimpleLineBuilder;
    size_t _inlineBoxCount {0};

  public:
    RangeBasedLineBuilder(InlineFormattingContext &inlineFormattingContext,
                          HorizontalConstraints rootHorizontalConstraints,
                          const InlineContentCache::InlineItems &inlineItems) noexcept;
    KRYS_NODISCARD LineLayoutResult LayoutInlineContent(const LineInput &lineInput,
                                                        const Maybe<PreviousLine> &previousLine,
                                                        bool isFirstFormattedLineCandidate) noexcept final;

    KRYS_NODISCARD static bool IsEligibleForRangeInlineLayout(
      const InlineFormattingContext &inlineFormattingContext, InlineItemRange needsLayoutRange,
      const InlineContentCache::InlineItems &inlineItems, const PlacedFloats &placedFloats) noexcept;
  };
}
