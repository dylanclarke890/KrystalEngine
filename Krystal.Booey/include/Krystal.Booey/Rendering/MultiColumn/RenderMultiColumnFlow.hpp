/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentedFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderMultiColumnSet;
  class RenderMultiColumnSpannerPlaceholder;

  /// @brief The content of a multi-column container, laid out as one tall column that its column sets cut
  /// into columns. It's the container's first child; the column sets and spanners (`column-span: all`
  /// boxes) follow it as the container's other children.
  // Columns always progress in the inline direction: the non-standard column axis and progression aren't
  // supported.
  class RenderMultiColumnFlow final : public RenderFragmentedFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderMultiColumnFlow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderMultiColumnFlow);

  public:
    // Spanners leave the map when their placeholder is removed
    // (RenderTreeBuilder::MultiColumn::MultiColumnRelativeWillBeRemoved).
    using SpannerMap = HashMap<const RenderBox *, WeakPtr<RenderMultiColumnSpannerPlaceholder>>;

  private:
    SpannerMap _spannerMap;

    // The last set we worked on. It's not to be used as the "current set". The concept of a
    // "current set" is difficult, since layout may jump back and forth in the tree, due to wrong
    // top location estimates (due to e.g. margin collapsing), and possibly for other reasons.
    mutable WeakPtr<RenderMultiColumnSet> _lastSetWorkedOn;

    // The default column count and width, based on the containing block's width.
    uint32 _columnCount {1};
    LayoutUnit _columnWidth;

    LayoutUnit _columnHeightAvailable; // Total height available to columns, or 0 if auto.
    bool _inLayout {
      false}; // Set while we're laying out the flow, during which column set heights are unknown.
    bool _inBalancingPass {false}; // Guard to avoid re-entering column balancing.
    bool _needsHeightsRecalculation {false};

  public:
    RenderMultiColumnFlow(dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderMultiColumnFlow() noexcept override;

    /// @brief The multi-column container.
    KRYS_NODISCARD RenderBlockFlow *MultiColumnBlockFlow() const noexcept;

    KRYS_NODISCARD RenderMultiColumnSet *FirstMultiColumnSet() const noexcept;
    KRYS_NODISCARD RenderMultiColumnSet *LastMultiColumnSet() const noexcept;
    KRYS_NODISCARD RenderBox *FirstColumnSetOrSpanner() const noexcept;
    KRYS_NODISCARD bool HasColumnSpanner() const noexcept
    {
      return !_spannerMap.empty();
    }
    KRYS_NODISCARD static RenderBox *NextColumnSetOrSpannerSiblingOf(const RenderBox *child) noexcept;
    KRYS_NODISCARD static RenderBox *PreviousColumnSetOrSpannerSiblingOf(const RenderBox *child) noexcept;

    KRYS_NODISCARD RenderMultiColumnSpannerPlaceholder *
      FindColumnSpannerPlaceholder(const RenderBox &spanner) const noexcept;

    void Layout() noexcept override;

    KRYS_NODISCARD uint32 ColumnCount() const noexcept
    {
      return _columnCount;
    }
    KRYS_NODISCARD LayoutUnit ColumnWidth() const noexcept
    {
      return _columnWidth;
    }
    KRYS_NODISCARD LayoutUnit ColumnHeightAvailable() const noexcept
    {
      return _columnHeightAvailable;
    }
    void SetColumnHeightAvailable(LayoutUnit available) noexcept
    {
      _columnHeightAvailable = available;
    }
    KRYS_NODISCARD bool InBalancingPass() const noexcept
    {
      return _inBalancingPass;
    }
    void SetInBalancingPass(bool balancing) noexcept
    {
      _inBalancingPass = balancing;
    }
    KRYS_NODISCARD bool NeedsHeightsRecalculation() const noexcept
    {
      return _needsHeightsRecalculation;
    }
    void SetNeedsHeightsRecalculation(bool recalculate) noexcept
    {
      _needsHeightsRecalculation = recalculate;
    }

    KRYS_NODISCARD bool ShouldRelayoutForPagination() const noexcept
    {
      return !_inBalancingPass && _needsHeightsRecalculation;
    }

    void SetColumnCountAndWidth(uint32 count, LayoutUnit width) noexcept
    {
      _columnCount = count;
      _columnWidth = width;
    }

    RenderFragmentContainer *MapFromFlowToFragment(TransformState &transformState) const noexcept final;

    /// @brief The physical translation from the flow's logical offset @p logicalOffset to where it's shown
    /// in @p fragmentContainer.
    KRYS_NODISCARD LayoutSize PhysicalTranslationOffsetFromFlowToFragment(
      const RenderFragmentContainer *fragmentContainer, LayoutUnit logicalOffset) const noexcept;

    /// @brief Moves the physical point @p physicalPoint into the fragment showing it, which is returned.
    RenderFragmentContainer *PhysicalTranslationFromFlowToFragment(LayoutPoint &physicalPoint) const noexcept;

    /// @brief The inverse of PhysicalTranslationFromFlowToFragment: from a point in @p columnSet to the flow.
    KRYS_NODISCARD LayoutSize PhysicalTranslationFromFragmentToFlow(
      const RenderMultiColumnSet *columnSet, const LayoutPoint &physicalPoint) const noexcept;

    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;

    void MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                 TransformState &transformState) const noexcept override;
    KRYS_NODISCARD LayoutSize
      OffsetFromContainer(const RenderElement &container, const LayoutPoint &point,
                          bool *offsetDependsOnPoint = nullptr) const noexcept override;

    KRYS_NODISCARD bool ShouldCheckColumnBreaks() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD SpannerMap &GetSpannerMap() noexcept
    {
      return _spannerMap;
    }

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderMultiColumnFlowThread";
    }
    void AddFragmentToThread(RenderFragmentContainer *fragmentContainer) noexcept override;
    void WillBeRemovedFromTree() noexcept override;
    void FragmentedFlowDescendantBoxLaidOut(RenderBox *descendant) noexcept override;
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;
    KRYS_NODISCARD LayoutUnit InitialLogicalWidth() const noexcept override;
    void SetPageBreak(const RenderBlock *block, LayoutUnit offset,
                      LayoutUnit spaceShortage) noexcept override;
    void UpdateMinimumPageHeight(const RenderBlock *block, LayoutUnit offset,
                                 LayoutUnit minHeight) noexcept override;
    void UpdateSpaceShortageForSizeContainment(const RenderBlock *block, LayoutUnit offset,
                                               LayoutUnit shortage) noexcept override;
    KRYS_NODISCARD RenderFragmentContainer *
      FragmentAtBlockOffset(const RenderBox *box, LayoutUnit offset,
                            bool extendLastFragment = false) const noexcept override;
    void SetFragmentRangeForBox(const RenderBox &box, RenderFragmentContainer *startFragment,
                                RenderFragmentContainer *endFragment) noexcept override;
    bool AddForcedFragmentBreak(const RenderBlock *block, LayoutUnit offset, RenderBox *breakChild,
                                bool isBefore, LayoutUnit *offsetBreakAdjustment = nullptr) noexcept override;
    KRYS_NODISCARD bool IsPageLogicalHeightKnown() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderMultiColumnFlow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderMultiColumnFlow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
