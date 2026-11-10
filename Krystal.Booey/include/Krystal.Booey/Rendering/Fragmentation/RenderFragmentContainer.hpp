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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
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

#include "Krystal.Booey/Rendering/Layers/LayerFragment.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBoxFragmentInfo;
  class RenderFragmentedFlow;

  /// @brief A box a fragmented flow's content is shown in: a viewport onto a portion of the flow (one column,
  /// or a run of columns for a fragment container set).
  class RenderFragmentContainer : public RenderBlockFlow
  {
    KRYS_NON_COPY_MOVABLE(RenderFragmentContainer);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderFragmentContainer);

  protected:
    WeakPtr<RenderFragmentedFlow> _fragmentedFlow;

  private:
    LayoutRect _fragmentedFlowPortionRect;

    // This map holds unique information about a block that is split across fragments.
    // A RenderBoxFragmentInfo* tells us about any layout information for a RenderBox that
    // is unique to the fragment. For now it just holds logical width information for RenderBlocks.
    // Boxes leave it when they leave the flow (RenderFragmentedFlow::RemoveFlowChildInfo).
    HashMap<const RenderBox *, UniquePtr<RenderBoxFragmentInfo>> _renderBoxFragmentInfo;

    bool _isValid {false};

  public:
    void StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept override;

    void SetFragmentedFlowPortionRect(const LayoutRect &rect) noexcept
    {
      _fragmentedFlowPortionRect = rect;
    }
    KRYS_NODISCARD LayoutRect FragmentedFlowPortionRect() const noexcept
    {
      return _fragmentedFlowPortionRect;
    }
    KRYS_NODISCARD LayoutRect FragmentedFlowPortionOverflowRect() const noexcept;

    KRYS_NODISCARD LayoutPoint FragmentedFlowPortionLocation() const noexcept;

    virtual void AttachFragment() noexcept;
    virtual void DetachFragment() noexcept;

    KRYS_NODISCARD RenderFragmentedFlow *FragmentedFlow() const noexcept;

    /// @brief Whether the fragment is attached to its flow. Valid fragments do not create circular
    /// dependencies with other flows.
    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return _isValid;
    }
    void SetIsValid(bool valid) noexcept
    {
      _isValid = valid;
    }

    KRYS_NODISCARD RenderBoxFragmentInfo *GetRenderBoxFragmentInfo(const RenderBox &box) const noexcept;
    RenderBoxFragmentInfo *SetRenderBoxFragmentInfo(const RenderBox &box, LayoutUnit logicalLeftInset,
                                                    LayoutUnit logicalRightInset,
                                                    bool containingBlockChainIsInset) noexcept;
    KRYS_NODISCARD UniquePtr<RenderBoxFragmentInfo> TakeRenderBoxFragmentInfo(const RenderBox &box) noexcept;
    void RemoveRenderBoxFragmentInfo(const RenderBox &box) noexcept;

    void DeleteAllRenderBoxFragmentInfo() noexcept;

    KRYS_NODISCARD bool IsFirstFragment() const noexcept;
    KRYS_NODISCARD bool IsLastFragment() const noexcept;
    KRYS_NODISCARD virtual bool ShouldClipFragmentedFlowContent() const noexcept;

    /// @brief The logical width of a "page": the content width of a fragment, or of one of the columns or
    /// pages of a set.
    KRYS_NODISCARD virtual LayoutUnit PageLogicalWidth() const noexcept;
    /// @brief The logical height of a "page": the content height of a fragment, or of one of the columns or
    /// pages of a set.
    KRYS_NODISCARD virtual LayoutUnit PageLogicalHeight() const noexcept;

    KRYS_NODISCARD LayoutUnit LogicalTopOfFragmentedFlowContentRect(const LayoutRect &rect) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalBottomOfFragmentedFlowContentRect(const LayoutRect &rect) const noexcept;
    KRYS_NODISCARD LayoutUnit LogicalTopForFragmentedFlowContent() const noexcept
    {
      return LogicalTopOfFragmentedFlowContentRect(FragmentedFlowPortionRect());
    }
    KRYS_NODISCARD LayoutUnit LogicalBottomForFragmentedFlowContent() const noexcept
    {
      return LogicalBottomOfFragmentedFlowContentRect(FragmentedFlowPortionRect());
    }

    /// @brief The logical height of the flow's portion the fragment shows: its page height, or for a set the
    /// height of all its columns or pages added together.
    KRYS_NODISCARD virtual LayoutUnit LogicalHeightOfAllFragmentedFlowContent() const noexcept;

    /// @brief The top of the nearest page inside the fragment: the logical top of the flow's portion it
    /// shows, or for a set the top of the column or page holding @p offset.
    KRYS_NODISCARD virtual LayoutUnit PageLogicalTopForOffset(LayoutUnit offset) const noexcept;

    /// @brief Whether the fragment is a run of same-sized columns or pages.
    KRYS_NODISCARD virtual bool IsRenderFragmentContainerSet() const noexcept
    {
      return false;
    }

    virtual void RepaintFragmentedFlowContent(const LayoutRect &repaintRect) const noexcept;

    virtual void CollectLayerFragments(LayerFragments &, const LayoutRect &,
                                       const LayoutRect &) const noexcept
    {
    }

    void AddLayoutOverflowForBox(const RenderBox &box, const LayoutRect &rect) noexcept;
    void AddVisualOverflowForBox(const RenderBox &box, const LayoutRect &rect) noexcept;
    KRYS_NODISCARD LayoutRect VisualOverflowRectForBox(const RenderBox &box) const noexcept;
    KRYS_NODISCARD LayoutRect LayoutOverflowRectForBoxForPropagation(const RenderBox &box) noexcept;
    KRYS_NODISCARD LayoutRect VisualOverflowRectForBoxForPropagation(const RenderBox &box) noexcept;

    /// @brief The part of @p rect (in @p box's coordinates) inside the fragment.
    KRYS_NODISCARD LayoutRect RectFlowPortionForBox(const RenderBox &box,
                                                    const LayoutRect &rect) const noexcept;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CanHaveGeneratedChildren() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD dom::PositionWithAffinity
      PositionForPoint(const LayoutPoint &point, HitTestSource source,
                       const RenderFragmentContainer *fragment = nullptr) noexcept override;

    /// @brief Where @p contentRect (in the flow's coordinates) is shown in the fragment.
    KRYS_NODISCARD virtual List<LayoutRect>
      FragmentRectsForFlowContentRect(const LayoutRect &contentRect) const noexcept;
    KRYS_NODISCARD virtual bool ContentRectSpansFragments(const LayoutRect &) const noexcept
    {
      return false;
    }

  protected:
    RenderFragmentContainer(Type type, dom::Element &element, RenderStyle &&style,
                            RenderFragmentedFlow *fragmentedFlow) noexcept;
    RenderFragmentContainer(Type type, dom::Document &document, RenderStyle &&style,
                            RenderFragmentedFlow *fragmentedFlow) noexcept;
    ~RenderFragmentContainer() noexcept override;

    KRYS_NODISCARD RenderOverflow *OverflowForBox(const RenderBox &box) const noexcept;

    void ComputePreferredLogicalWidths() noexcept override;
    void ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                       LayoutUnit &maxLogicalWidth) const noexcept override;

    KRYS_NODISCARD LayoutRect OverflowRectForFragmentedFlowPortion(
      const LayoutRect &fragmentedFlowPortionRect, bool isFirstPortion, bool isLastPortion) const noexcept;
    void RepaintFragmentedFlowContentRectangle(
      const LayoutRect &repaintRect, const LayoutRect &fragmentedFlowPortionRect,
      const LayoutPoint &fragmentLocation,
      const LayoutRect *fragmentedFlowPortionClipRect = nullptr) const noexcept;

    KRYS_NODISCARD LayoutRect FragmentedFlowContentRectangle(
      const LayoutRect &rect, const LayoutRect &fragmentedFlowPortionRect,
      const LayoutPoint &fragmentLocation,
      const LayoutRect *fragmentedFlowPortionClipRect = nullptr) const noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderFragmentContainer";
    }

    void InsertedIntoTree() noexcept override;
    void WillBeRemovedFromTree() noexcept override;

    /// @brief Connects the fragment to its flow, which a set did when it was made.
    virtual void InstallFragmentedFlow() noexcept;

    KRYS_NODISCARD LayoutPoint
      MapFragmentPointIntoFragmentedFlowCoordinates(const LayoutPoint &point) noexcept;
    KRYS_NODISCARD LayoutRect ComputedVisualOverflowRectForBox(const RenderBox &box) const noexcept;
    KRYS_NODISCARD LayoutRect ComputedLayoutOverflowRectForBox(const RenderBox &box) const noexcept;
  };

  /// @brief Makes a fragment its flow's current fragment for the maintainer's lifetime.
  class CurrentRenderFragmentContainerMaintainer
  {
    KRYS_NON_COPY_MOVABLE(CurrentRenderFragmentContainerMaintainer);

  private:
    RenderFragmentContainer &_fragment;

  public:
    explicit CurrentRenderFragmentContainerMaintainer(RenderFragmentContainer &fragment) noexcept;
    ~CurrentRenderFragmentContainerMaintainer() noexcept;

    KRYS_NODISCARD RenderFragmentContainer &Fragment() const noexcept
    {
      return _fragment;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderFragmentContainer)
  static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderFragmentContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
