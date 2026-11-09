/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderBlockFlow;
}

namespace krys::boo::layout
{
  class InlineDamage;
}

namespace krys::boo::layout_integration
{
  class InlineContent;

  /// @brief Moves a layout result's display content into a block container's inline content and computes the
  /// lines' overflow and the boxes' first and last flags.
  class InlineContentBuilder
  {
  private:
    const RenderBlockFlow &_blockFlow;

  public:
    explicit InlineContentBuilder(const RenderBlockFlow &blockFlow) noexcept;

    /// @return The rect the content covered before and after, to repaint.
    FloatRect Build(layout::InlineLayoutResult &&layoutResult, InlineContent &inlineContent,
                    const layout::InlineDamage *lineDamage) const noexcept;
    void UpdateLineOverflow(InlineContent &inlineContent) const noexcept;

  private:
    void AdjustDisplayLines(InlineContent &inlineContent, size_t startIndex) const noexcept;
    FloatRect HandlePartialDisplayContentUpdate(layout::InlineLayoutResult &&layoutResult,
                                                InlineContent &inlineContent,
                                                const layout::InlineDamage *lineDamage) const noexcept;
    void
      ComputeIsFirstIsLastBoxAndBidiReorderingForInlineContent(inline_display::Boxes &boxes) const noexcept;
  };
}
