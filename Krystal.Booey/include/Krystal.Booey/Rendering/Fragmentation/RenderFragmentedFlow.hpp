/*
 * Copyright (C) 2011 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentContainer.hpp"
#include "Krystal.Booey/Rendering/Layers/LayerFragment.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CurrentRenderFragmentContainerMaintainer;
  class RenderFragmentContainer;

  /// @brief The renderers of fragmented content (the content of a multi-column container), laid out as one
  /// tall strip that its fragment containers each show a portion of. It isn't painted or hit tested itself:
  /// its fragments forward their painting and hit testing to it.
  class RenderFragmentedFlow : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderFragmentedFlow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderFragmentedFlow);

  protected:
    /// @brief The fragments a box is laid out in.
    class RenderFragmentContainerRange
    {
    private:
      WeakPtr<RenderFragmentContainer> _startFragment;
      WeakPtr<RenderFragmentContainer> _endFragment;
      bool _rangeInvalidated {false};

    public:
      RenderFragmentContainerRange() noexcept = default;
      RenderFragmentContainerRange(RenderFragmentContainer *start, RenderFragmentContainer *end) noexcept
      {
        SetRange(start, end);
      }

      void SetRange(RenderFragmentContainer *start, RenderFragmentContainer *end) noexcept;

      KRYS_NODISCARD RenderFragmentContainer *StartFragment() const noexcept;
      KRYS_NODISCARD RenderFragmentContainer *EndFragment() const noexcept;
      KRYS_NODISCARD bool RangeInvalidated() const noexcept
      {
        return _rangeInvalidated;
      }
      void ClearRangeInvalidated() noexcept
      {
        _rangeInvalidated = false;
      }
    };

    /// @brief The block-axis interval of the flow a fragment shows.
    struct FragmentInterval
    {
      LayoutUnit Low;
      LayoutUnit High;
      RenderFragmentContainer *Fragment {nullptr};
    };

    // Fragments leave the list when they're removed from the tree (RenderFragmentContainer::DetachFragment).
    List<RenderFragmentContainer *> _fragmentList;

    // Maps a box to the fragments it's rendered in. Boxes leave it when they leave the flow
    // (RemoveFlowChildInfo).
    HashMap<const RenderBox *, RenderFragmentContainerRange> _fragmentRangeMap;

    // The fragments' intervals in flow order, which are contiguous, so searched by binary search.
    List<FragmentInterval> _fragmentIntervals;

    CurrentRenderFragmentContainerMaintainer *_currentFragmentMaintainer {nullptr};

    bool _fragmentsInvalidated : 1 {false};
    bool _fragmentsHaveUniformLogicalWidth : 1 {true};
    bool _fragmentsHaveUniformLogicalHeight : 1 {true};
    bool _pageLogicalSizeChanged : 1 {false};

  public:
    ~RenderFragmentedFlow() noexcept override;

    /// @brief Drops what the flow knows about @p child (the fragments it's in).
    virtual void RemoveFlowChildInfo(RenderElement &child) noexcept;

    virtual void AddFragmentToThread(RenderFragmentContainer *fragment) noexcept = 0;
    void RemoveFragmentFromThread(RenderFragmentContainer &fragment) noexcept;
    KRYS_NODISCARD Span<RenderFragmentContainer *const> RenderFragmentContainerList() const noexcept
    {
      return _fragmentList;
    }

    void UpdateLogicalWidth() noexcept final;
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;

    KRYS_NODISCARD bool HasFragments() const noexcept
    {
      return !_fragmentList.empty();
    }
    virtual void FragmentChangedWritingMode(RenderFragmentContainer *) noexcept
    {
    }

    void ValidateFragments() noexcept;
    void InvalidateFragments(
      MarkingBehavior markingParents = MarkingBehavior::MarkContainingBlockChain) noexcept;
    KRYS_NODISCARD bool HasValidFragmentInfo() const noexcept
    {
      return !_fragmentsInvalidated && !_fragmentList.empty();
    }

    /// @brief Called when a descendant box's layout is finished and it has been positioned within its
    /// container.
    virtual void FragmentedFlowDescendantBoxLaidOut(RenderBox *) noexcept
    {
    }

    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    void RepaintRectangleInFragments(const LayoutRect &repaintRect) const noexcept;

    KRYS_NODISCARD LayoutUnit PageLogicalTopForOffset(LayoutUnit offset) const noexcept;
    KRYS_NODISCARD LayoutUnit PageLogicalWidthForOffset(LayoutUnit offset) const noexcept;
    KRYS_NODISCARD LayoutUnit PageLogicalHeightForOffset(LayoutUnit offset) const noexcept;
    KRYS_NODISCARD LayoutUnit PageRemainingLogicalHeightForOffset(
      LayoutUnit offset,
      PageBoundaryRule pageBoundaryRule = PageBoundaryRule::IncludePageBoundary) const noexcept;

    virtual void SetPageBreak(const RenderBlock *, LayoutUnit /*offset*/,
                              LayoutUnit /*spaceShortage*/) noexcept
    {
    }
    virtual void UpdateMinimumPageHeight(const RenderBlock *, LayoutUnit /*offset*/,
                                         LayoutUnit /*minHeight*/) noexcept
    {
    }
    virtual void UpdateSpaceShortageForSizeContainment(const RenderBlock *, LayoutUnit /*offset*/,
                                                       LayoutUnit /*shortage*/) noexcept
    {
    }

    /// @brief The fragment showing the flow's block offset @p offset, clamped to @p clampBox's fragments.
    KRYS_NODISCARD virtual RenderFragmentContainer *
      FragmentAtBlockOffset(const RenderBox *clampBox, LayoutUnit offset,
                            bool extendLastFragment = false) const noexcept;

    KRYS_NODISCARD bool FragmentsHaveUniformLogicalWidth() const noexcept
    {
      return _fragmentsHaveUniformLogicalWidth;
    }
    KRYS_NODISCARD bool FragmentsHaveUniformLogicalHeight() const noexcept
    {
      return _fragmentsHaveUniformLogicalHeight;
    }

    /// @brief Moves @p transformState from the flow into the fragment showing it.
    virtual RenderFragmentContainer *MapFromFlowToFragment(TransformState &transformState) const noexcept;

    void LogicalWidthChangedInFragmentsForBlock(const RenderBlock &block,
                                                RelayoutChildren &relayoutChildren) noexcept;

    KRYS_NODISCARD LayoutUnit ContentLogicalWidthOfFirstFragment() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentLogicalHeightOfFirstFragment() const noexcept;
    KRYS_NODISCARD LayoutUnit ContentLogicalLeftOfFirstFragment() const noexcept;

    KRYS_NODISCARD RenderFragmentContainer *FirstFragment() const noexcept;
    KRYS_NODISCARD RenderFragmentContainer *LastFragment() const noexcept;

    virtual void SetFragmentRangeForBox(const RenderBox &box, RenderFragmentContainer *startFragment,
                                        RenderFragmentContainer *endFragment) noexcept;
    bool GetFragmentRangeForBox(const RenderBox &box, RenderFragmentContainer *&startFragment,
                                RenderFragmentContainer *&endFragment) const noexcept;
    bool ComputedFragmentRangeForBox(const RenderBox &box, RenderFragmentContainer *&startFragment,
                                     RenderFragmentContainer *&endFragment) const noexcept;
    KRYS_NODISCARD bool HasCachedFragmentRangeForBox(const RenderBox &box) const noexcept;

    /// @brief Whether @p object is in @p fragment, a fragment of the flow.
    KRYS_NODISCARD bool ObjectInFlowFragment(const RenderObject *object,
                                             const RenderFragmentContainer *fragment) const noexcept;

    /// @brief Whether @p object should be painted in @p fragment, a fragment of the flow.
    KRYS_NODISCARD bool
      ObjectShouldFragmentInFlowFragment(const RenderObject *object,
                                         const RenderFragmentContainer *fragment) const noexcept;

    virtual bool AddForcedFragmentBreak(const RenderBlock *block, LayoutUnit offsetBreakInFragmentedFlow,
                                        RenderBox *breakChild, bool isBefore,
                                        LayoutUnit *offsetBreakAdjustment = nullptr) noexcept;
    virtual void ApplyBreakAfterContent(LayoutUnit) noexcept
    {
    }

    KRYS_NODISCARD virtual bool IsPageLogicalHeightKnown() const noexcept
    {
      return true;
    }
    KRYS_NODISCARD bool PageLogicalSizeChanged() const noexcept
    {
      return _pageLogicalSizeChanged;
    }

    void CollectLayerFragments(LayerFragments &layerFragments, const LayoutRect &layerBoundingBox,
                               const LayoutRect &dirtyRect) const noexcept;
    KRYS_NODISCARD LayoutRect FragmentsBoundingBox(const LayoutRect &layerBoundingBox) const noexcept;

    KRYS_NODISCARD LayoutUnit
      OffsetFromLogicalTopOfFirstFragment(const RenderBlock *currentBlock) const noexcept;
    void ClearRenderBoxFragmentInfoAndCustomStyle(const RenderBox &box,
                                                  const RenderFragmentContainer *newStartFragment,
                                                  const RenderFragmentContainer *newEndFragment,
                                                  const RenderFragmentContainer *oldStartFragment,
                                                  const RenderFragmentContainer *oldEndFragment) noexcept;

    void AddFragmentsVisualEffectOverflow(const RenderBox &box) noexcept;
    void AddFragmentsVisualOverflowFromTheme(const RenderBlock &block) noexcept;
    void AddFragmentsOverflowFromChild(const RenderBox &box, const RenderBox &child,
                                       const LayoutSize &delta) noexcept;
    void AddFragmentsLayoutOverflow(const RenderBox &box, const LayoutRect &layoutOverflow) noexcept;
    void AddFragmentsVisualOverflow(const RenderBox &box, const LayoutRect &visualOverflow) noexcept;
    void ClearFragmentsOverflow(const RenderBox &box) noexcept;

    KRYS_NODISCARD LayoutRect MapFromFragmentedFlowToLocal(const RenderBox *box,
                                                           const LayoutRect &rect) const noexcept;
    KRYS_NODISCARD LayoutRect MapFromLocalToFragmentedFlow(const RenderBox *box,
                                                           const LayoutRect &localRect) const noexcept;

    void FlipForWritingModeLocalCoordinates(LayoutRect &rect) const noexcept;

    /// @brief Used to estimate the maximum height of the flow.
    KRYS_NODISCARD static LayoutUnit MaxLogicalHeight() noexcept
    {
      return LayoutUnit::Max() / 2;
    }

    KRYS_NODISCARD bool FragmentInRange(const RenderFragmentContainer *targetFragment,
                                        const RenderFragmentContainer *startFragment,
                                        const RenderFragmentContainer *endFragment) const noexcept;

    bool AbsoluteQuadsForBox(List<FloatQuad> &quads, bool *wasFixed, const RenderBox &box) const noexcept;

    KRYS_NODISCARD bool BoxIsFragmented(const RenderBox &box) const noexcept;

    void Layout() noexcept override;

    void SetCurrentFragmentMaintainer(
      CurrentRenderFragmentContainerMaintainer *currentFragmentMaintainer) noexcept
    {
      _currentFragmentMaintainer = currentFragmentMaintainer;
    }
    KRYS_NODISCARD RenderFragmentContainer *CurrentFragment() const noexcept;

    KRYS_NODISCARD bool CachedEnclosingFragmentedFlowNeedsUpdate() const noexcept override
    {
      return false;
    }

    // FIXME: Eventually as column and fragment flow threads start nesting, this may end up changing.
    KRYS_NODISCARD virtual bool ShouldCheckColumnBreaks() const noexcept
    {
      return false;
    }

  private:
    // Always create a RenderLayer for the RenderFragmentedFlow so that we
    // can easily avoid drawing the children directly.
    KRYS_NODISCARD bool RequiresLayer() const noexcept final
    {
      return true;
    }

  protected:
    RenderFragmentedFlow(Type type, dom::Document &document, RenderStyle &&style) noexcept;

    KRYS_NODISCARD RenderFragmentedFlow *LocateEnclosingFragmentedFlow() const noexcept override
    {
      return const_cast<RenderFragmentedFlow *>(this);
    }

    /// @brief The flow's logical width before it has fragments; columns use the column width.
    KRYS_NODISCARD virtual LayoutUnit InitialLogicalWidth() const noexcept
    {
      return 0_lu;
    }

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &transformState,
                             EnumSet<MapCoordinatesMode> mode, bool *wasFixed) const noexcept override;

    void UpdateFragmentsFragmentedFlowPortionRect() noexcept;
    KRYS_NODISCARD bool ShouldRepaint(const LayoutRect &rect) const noexcept;

    bool GetFragmentRangeForBoxFromCachedInfo(const RenderBox &box, RenderFragmentContainer *&startFragment,
                                              RenderFragmentContainer *&endFragment) const noexcept;

    void RemoveRenderBoxFragmentInfo(const RenderBox &box) noexcept;

    /// @brief The fragments from @p startFragment (one of the flow's) to the last.
    KRYS_NODISCARD Span<RenderFragmentContainer *const>
      FragmentsFrom(const RenderFragmentContainer &startFragment) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderFragmentedFlow)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderFragmentedFlow();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
