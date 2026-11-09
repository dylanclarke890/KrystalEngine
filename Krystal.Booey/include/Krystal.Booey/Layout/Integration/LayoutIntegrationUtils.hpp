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

#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::layout
{
  class BoxGeometry;
  class ElementBox;
  class InlineLayoutState;
  class LayoutState;

  /// @brief How the layout formatting contexts reach back into render tree layout: laying out and measuring
  /// the formatting context roots they contain, and converting block layout state between the two.
  class IntegrationUtils
  {
  private:
    const CheckedRef<const LayoutState> _globalLayoutState;

  public:
    explicit IntegrationUtils(const LayoutState &globalLayoutState) noexcept;

    void LayoutWithFormattingContextForBox(const ElementBox &box, Maybe<LayoutUnit> widthConstraint = null,
                                           Maybe<LayoutUnit> heightConstraint = null) const noexcept;
    KRYS_NODISCARD LayoutUnit MaxContentWidth(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit MinContentWidth(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit MinContentHeight(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit PreferredMinWidth(const ElementBox &box) const noexcept;
    KRYS_NODISCARD LayoutUnit PreferredMaxWidth(const ElementBox &box) const noexcept;

    void
      LayoutWithFormattingContextForBlockInInline(const ElementBox &block,
                                                  LayoutPoint blockLineLogicalTopLeft,
                                                  const InlineLayoutState &inlineLayoutState) const noexcept;

    KRYS_NODISCARD static BlockLayoutState::MarginState
      ToMarginState(const RenderBlockFlow::MarginInfo &marginInfo,
                    LayoutUnit contentOffsetAfterSelfCollapsingBlock) noexcept;
    KRYS_NODISCARD static RenderBlockFlow::MarginInfo
      ToMarginInfo(const BlockLayoutState::MarginState &marginState) noexcept;
    KRYS_NODISCARD static Pair<LayoutRect, LayoutRect>
      ToMarginAndBorderBoxVisualRect(const BoxGeometry &logicalGeometry, const LayoutSize &containerSize,
                                     WritingMode writingMode) noexcept;
  };
}
