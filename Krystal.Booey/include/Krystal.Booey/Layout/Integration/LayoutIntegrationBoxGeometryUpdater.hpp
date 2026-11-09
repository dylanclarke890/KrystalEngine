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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBox;
  class RenderBoxModelObject;
  class RenderElement;
  class RenderInline;
  class RenderLineBreak;
  class RenderListMarker;
}

namespace krys::boo::layout
{
  class ElementBox;
  class LayoutState;
}

namespace krys::boo::layout_integration
{
  /// @brief Copies the geometry the render tree computed (margins, borders, padding, sizes and baselines) to
  /// the layout boxes of a formatting context before the layout formatting context runs.
  class BoxGeometryUpdater
  {
  private:
    WeakPtr<layout::LayoutState> _layoutState;
    CheckedPtr<const layout::ElementBox> _rootLayoutBox;
    HashMap<const layout::ElementBox *, LayoutUnit> _nestedListMarkerOffsets;

  public:
    BoxGeometryUpdater(layout::LayoutState &layoutState, const layout::ElementBox &rootLayoutBox) noexcept;

    void Clear() noexcept;

    void SetFormattingContextRootGeometry(LayoutUnit availableLogicalWidth) noexcept;
    void SetFormattingContextContentGeometry(Maybe<LayoutUnit> availableLogicalWidth,
                                             Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept;
    void UpdateBoxGeometryAfterIntegrationLayout(const layout::ElementBox &layoutBox,
                                                 LayoutUnit availableWidth) noexcept;
    KRYS_NODISCARD layout::ConstraintsForInlineContent
      FormattingContextConstraints(LayoutUnit availableWidth) noexcept;

    KRYS_NODISCARD HashMap<const layout::ElementBox *, LayoutUnit> TakeNestedListMarkerOffsets() noexcept
    {
      return krys::move(_nestedListMarkerOffsets);
    }

  private:
    void UpdateBoxGeometry(const RenderElement &renderer, Maybe<LayoutUnit> availableWidth,
                           Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode) noexcept;
    void UpdateLayoutBoxDimensions(const RenderBox &renderBox, Maybe<LayoutUnit> availableWidth,
                                   Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode = null) noexcept;
    void UpdateLineBreakBoxDimensions(const RenderLineBreak &lineBreakBox) noexcept;
    void UpdateInlineBoxDimensions(const RenderInline &renderInline, Maybe<LayoutUnit> availableWidth,
                                   Maybe<layout::IntrinsicWidthMode> intrinsicWidthMode = null) noexcept;
    void SetListMarkerOffsetForMarkerOutside(const RenderListMarker &listMarker) noexcept;

    KRYS_NODISCARD layout::BoxGeometry::HorizontalEdges
      HorizontalLogicalMargin(const RenderBoxModelObject &renderer, Maybe<LayoutUnit> availableWidth,
                              WritingMode writingMode, bool retainMarginStart = true,
                              bool retainMarginEnd = true) noexcept;
    KRYS_NODISCARD layout::BoxGeometry::VerticalEdges
      VerticalLogicalMargin(const RenderBoxModelObject &renderer, Maybe<LayoutUnit> availableWidth,
                            WritingMode writingMode) noexcept;
    KRYS_NODISCARD layout::BoxGeometry::Edges LogicalBorder(const RenderBoxModelObject &renderer,
                                                            WritingMode writingMode,
                                                            bool isIntrinsicWidthMode = false,
                                                            bool retainBorderStart = true,
                                                            bool retainBorderEnd = true) noexcept;
    KRYS_NODISCARD layout::BoxGeometry::Edges LogicalPadding(const RenderBoxModelObject &renderer,
                                                             Maybe<LayoutUnit> availableWidth,
                                                             WritingMode writingMode,
                                                             bool retainPaddingStart = true,
                                                             bool retainPaddingEnd = true) noexcept;

    KRYS_NODISCARD layout::LayoutState &GetLayoutState() noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }
    KRYS_NODISCARD const layout::ElementBox &RootLayoutBox() const noexcept
    {
      return *_rootLayoutBox;
    }
    KRYS_NODISCARD const RenderBlock &RootRenderer() const noexcept;
    KRYS_NODISCARD WritingMode GetWritingMode() const noexcept;
  };
}
