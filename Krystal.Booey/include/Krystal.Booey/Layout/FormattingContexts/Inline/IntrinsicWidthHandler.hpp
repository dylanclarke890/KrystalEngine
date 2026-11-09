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
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class AbstractLineBuilder;
  class InlineFormattingContext;

  /// @brief Computes the minimum and maximum content widths of inline content by laying it out with the line
  /// builders in intrinsic width mode, or with shortcuts for simple content.
  class IntrinsicWidthHandler
  {
    enum class MayCacheLayoutResult : bool
    {
      No,
      Yes
    };

  private:
    InlineFormattingContext &_inlineFormattingContext;
    const InlineContentCache::InlineItems &_inlineItems;
    InlineItemRange _inlineItemRange;
    bool _mayUseSimplifiedTextOnlyInlineLayoutInRange {false};

    Maybe<InlineLayoutUnit> _maximumContentWidthBetweenLineBreaks {};
    Maybe<LineLayoutResult> _maximumIntrinsicWidthResultForSingleLine {};

  public:
    IntrinsicWidthHandler(InlineFormattingContext &inlineFormattingContext,
                          const InlineContentCache::InlineItems &inlineItems) noexcept;

    KRYS_NODISCARD InlineLayoutUnit MinimumContentSize() noexcept;
    KRYS_NODISCARD InlineLayoutUnit MaximumContentSize() noexcept;

    KRYS_NODISCARD Maybe<LineLayoutResult> &MaximumIntrinsicWidthLineContent() noexcept
    {
      return _maximumIntrinsicWidthResultForSingleLine;
    }

  private:
    KRYS_NODISCARD InlineLayoutUnit ComputedIntrinsicWidthForConstraint(
      IntrinsicWidthMode intrinsicWidthMode, AbstractLineBuilder &lineBuilder,
      MayCacheLayoutResult mayCacheLayoutResult = MayCacheLayoutResult::No) noexcept;
    KRYS_NODISCARD InlineLayoutUnit SimplifiedMinimumWidth(const ElementBox &root) const noexcept;
    KRYS_NODISCARD InlineLayoutUnit
      SimplifiedMaximumWidth(MayCacheLayoutResult mayCacheLayoutResult = MayCacheLayoutResult::No) noexcept;

    KRYS_NODISCARD InlineFormattingContext &GetFormattingContext() noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const InlineFormattingContext &GetFormattingContext() const noexcept
    {
      return _inlineFormattingContext;
    }
    KRYS_NODISCARD const ElementBox &FormattingContextRoot() const noexcept;
    KRYS_NODISCARD const ElementBox &LineBuilerRoot() const noexcept;
    KRYS_NODISCARD const InlineItemList &GetInlineItemList() const noexcept
    {
      return _inlineItems.Content();
    }
  };
}
