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
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineDamage.hpp"
#include "Krystal.Booey/Style/Difference.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::layout
{
  class Box;
  class ElementBox;
  class InlineTextBox;
  struct InvalidatedLine;

  /// @brief Records changes to an inline formatting context's content and style in its damage: whether the
  /// inline item list needs rebuilding, and where a partial layout can start (or that it needs a full one).
  class InlineInvalidation
  {
  private:
    enum class ShouldApplyRangeLayout : bool
    {
      No,
      Yes
    };

    InlineDamage &_inlineDamage;

    const InlineItemList &_inlineItemList;
    const inline_display::Content &_displayContent;

  public:
    InlineInvalidation(InlineDamage &inlineDamage, const InlineItemList &inlineItemList,
                       const inline_display::Content &displayContent) noexcept;

    bool RootStyleWillChange(const ElementBox &formattingContextRoot, const RenderStyle &newStyle) noexcept;
    bool StyleWillChange(const Box &layoutBox, const RenderStyle &newStyle, style::Difference diff) noexcept;

    bool TextInserted(const InlineTextBox &newOrDamagedInlineTextBox, Maybe<size_t> offset = {}) noexcept;
    bool TextWillBeRemoved(const InlineTextBox &damagedInlineTextBox, Maybe<size_t> offset = {}) noexcept;

    bool InlineLevelBoxInserted(const Box &layoutBox) noexcept;
    bool InlineLevelBoxWillBeRemoved(const Box &layoutBox) noexcept;
    bool InlineLevelBoxContentWillChange(const Box &layoutBox) noexcept;

    bool RestartForPagination(size_t lineIndex, LayoutUnit pageTopAdjustment) noexcept;

    KRYS_NODISCARD static bool MayOnlyNeedPartialLayout(const InlineDamage *inlineDamage) noexcept
    {
      return inlineDamage && inlineDamage->LayoutStartPosition();
    }
    static void ResetInlineDamage(InlineDamage &inlineDamage) noexcept;

  private:
    bool UpdateInlineDamage(const InvalidatedLine &invalidatedLine, InlineDamage::Reason reason,
                            ShouldApplyRangeLayout shouldApplyRangeLayout = ShouldApplyRangeLayout::No,
                            LayoutUnit restartPaginationAdjustment = 0_lu) noexcept;
    bool SetFullLayoutIfNeeded(const Box &layoutBox) noexcept;
    KRYS_NODISCARD const inline_display::Boxes &DisplayBoxes() const noexcept
    {
      return _displayContent.BoxList;
    }
    KRYS_NODISCARD const inline_display::Lines &DisplayLines() const noexcept
    {
      return _displayContent.LineList;
    }
  };
}
