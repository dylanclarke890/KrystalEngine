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

#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentBreaker.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLine.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::layout
{
  struct LineInput
  {
    InlineItemRange NeedsLayoutRange;
    InlineRect InitialLogicalRect;
  };

  /// @brief Builds one line at a time out of a range of inline items.
  class AbstractLineBuilder
  {
    KRYS_NON_COPY_MOVABLE(AbstractLineBuilder);

  protected:
    layout::Line _line;
    InlineRect _lineLogicalRect;
    Span<const InlineItem> _inlineItemList;
    SmallList<const InlineItem *, 32> _wrapOpportunityList;
    Maybe<InlineTextItem> _partialLeadingTextItem;
    Maybe<PreviousLine> _previousLine {};
    bool _isFirstFormattedLineCandidate {false};

  private:
    InlineFormattingContext &_inlineFormattingContext;
    // Note that this is not necessarily a block container (see range builder).
    const ElementBox &_rootBox;
    HorizontalConstraints _rootHorizontalConstraints;

    InlineContentBreaker _inlineContentBreaker;
    Maybe<IntrinsicWidthMode> _intrinsicWidthMode;

  public:
    KRYS_NODISCARD virtual LineLayoutResult
      LayoutInlineContent(const LineInput &lineInput, const Maybe<PreviousLine> &previousLine,
                          bool isFirstFormattedLineCandidate) noexcept = 0;
    virtual ~AbstractLineBuilder() noexcept = default;

    void SetIntrinsicWidthMode(IntrinsicWidthMode intrinsicWidthMode) noexcept;

  protected:
    AbstractLineBuilder(InlineFormattingContext &inlineFormattingContext, const ElementBox &rootBox,
                        HorizontalConstraints rootHorizontalConstraints,
                        const InlineItemList &inlineItemList) noexcept;

    void Reset() noexcept;

    KRYS_NODISCARD Maybe<InlineLayoutUnit> OverflowWidthAsLeadingForNextLine(
      const InlineContentBreaker::ContinuousContent::RunList &candidateRuns,
      const InlineContentBreaker::Result &lineBreakingResult) const noexcept;

    KRYS_NODISCARD Maybe<IntrinsicWidthMode> GetIntrinsicWidthMode() const noexcept
    {
      return _intrinsicWidthMode;
    }
    KRYS_NODISCARD bool IsInIntrinsicWidthMode() const noexcept
    {
      return GetIntrinsicWidthMode().has_value();
    }

    KRYS_NODISCARD bool IsFirstFormattedLineCandidate() const noexcept
    {
      return _isFirstFormattedLineCandidate;
    }

    KRYS_NODISCARD InlineContentBreaker &GetInlineContentBreaker() noexcept
    {
      return _inlineContentBreaker;
    }

    KRYS_NODISCARD InlineFormattingContext &GetFormattingContext() noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const HorizontalConstraints &RootHorizontalConstraints() const noexcept
    {
      return _rootHorizontalConstraints;
    }
    KRYS_NODISCARD const InlineLayoutState &GetLayoutState() const noexcept;
    KRYS_NODISCARD InlineLayoutState &GetLayoutState() noexcept;
    KRYS_NODISCARD const BlockLayoutState &GetBlockLayoutState() const noexcept
    {
      return GetLayoutState().ParentBlockLayoutState();
    }
    KRYS_NODISCARD BlockLayoutState &GetBlockLayoutState() noexcept
    {
      return GetLayoutState().ParentBlockLayoutState();
    }
    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return _rootBox;
    }
    KRYS_NODISCARD const RenderStyle &RootStyle() const noexcept;
  };
}
