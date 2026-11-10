/*
 * Copyright (C) 2011 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 * Copyright (C) 2017-2020 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentContainer.hpp"
#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Booey/Rendering/Fragmentation/RenderBoxFragmentInfo.hpp"
#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentedFlow.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderOverflow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderFragmentContainer::RenderFragmentContainer(Type type, dom::Element &element, RenderStyle &&style,
                                                   RenderFragmentedFlow *fragmentedFlow) noexcept
      : RenderBlockFlow(type, element, krys::move(style), {BlockFlowFlag::IsFragmentContainer}),
        _fragmentedFlow(CreateWeakPtr(fragmentedFlow))
  {
  }

  RenderFragmentContainer::RenderFragmentContainer(Type type, dom::Document &document, RenderStyle &&style,
                                                   RenderFragmentedFlow *fragmentedFlow) noexcept
      : RenderBlockFlow(type, document, krys::move(style), {BlockFlowFlag::IsFragmentContainer}),
        _fragmentedFlow(CreateWeakPtr(fragmentedFlow))
  {
  }

  RenderFragmentContainer::~RenderFragmentContainer() noexcept = default;

  RenderFragmentedFlow *RenderFragmentContainer::FragmentedFlow() const noexcept
  {
    return _fragmentedFlow.get();
  }

  LayoutPoint
    RenderFragmentContainer::MapFragmentPointIntoFragmentedFlowCoordinates(const LayoutPoint &point) noexcept
  {
    // Assuming the point is relative to the fragment block, 3 cases will be considered:
    // a) top margin, padding or border.
    // b) bottom margin, padding or border.
    // c) non-content fragment area.

    bool isHorizontal = IsHorizontalWritingMode();
    const auto &portion = _fragmentedFlowPortionRect;
    LayoutUnit pointLogicalTop(isHorizontal ? point.Y() : point.X());
    LayoutUnit pointLogicalLeft(isHorizontal ? point.X() : point.Y());
    LayoutUnit fragmentedFlowLogicalTop(isHorizontal ? portion.Y() : portion.X());
    LayoutUnit fragmentedFlowLogicalLeft(isHorizontal ? portion.X() : portion.Y());
    LayoutUnit fragmentedFlowPortionTopBound(isHorizontal ? portion.Height() : portion.Width());
    LayoutUnit fragmentedFlowPortionLeftBound(isHorizontal ? portion.Width() : portion.Height());
    LayoutUnit fragmentedFlowPortionTopMax(isHorizontal ? portion.MaxY() : portion.MaxX());
    LayoutUnit fragmentedFlowPortionLeftMax(isHorizontal ? portion.MaxX() : portion.MaxY());
    LayoutUnit effectiveFixedPointDenominator;
    effectiveFixedPointDenominator.SetRawValue(1);

    auto toPhysical = [isHorizontal](LayoutPoint pointInThread)
    {
      return isHorizontal ? pointInThread : pointInThread.TransposedPoint();
    };

    if (pointLogicalTop < 0)
    {
      return toPhysical(LayoutPoint(0_lu, fragmentedFlowLogicalTop));
    }

    if (pointLogicalTop >= fragmentedFlowPortionTopBound)
    {
      return toPhysical(LayoutPoint(fragmentedFlowPortionLeftBound,
                                    fragmentedFlowPortionTopMax - effectiveFixedPointDenominator));
    }

    if (pointLogicalLeft < 0)
    {
      return toPhysical(LayoutPoint(fragmentedFlowLogicalLeft, pointLogicalTop + fragmentedFlowLogicalTop));
    }
    if (pointLogicalLeft >= fragmentedFlowPortionLeftBound)
    {
      return toPhysical(LayoutPoint(fragmentedFlowPortionLeftMax - effectiveFixedPointDenominator,
                                    pointLogicalTop + fragmentedFlowLogicalTop));
    }
    return toPhysical(
      LayoutPoint(pointLogicalLeft + fragmentedFlowLogicalLeft, pointLogicalTop + fragmentedFlowLogicalTop));
  }

  dom::PositionWithAffinity
    RenderFragmentContainer::PositionForPoint(const LayoutPoint &point, HitTestSource source,
                                              const RenderFragmentContainer *fragment) noexcept
  {
    // Checking for empty fragment blocks.
    if (!IsValid() || !_fragmentedFlow->FirstChild())
    {
      return RenderBlock::PositionForPoint(point, source, fragment);
    }

    return _fragmentedFlow->PositionForPoint(MapFragmentPointIntoFragmentedFlowCoordinates(point), source,
                                             this);
  }

  LayoutUnit RenderFragmentContainer::PageLogicalWidth() const noexcept
  {
    krys_debug_assert(IsValid());
    return _fragmentedFlow->IsHorizontalWritingMode() ? ContentBoxWidth() : ContentBoxHeight();
  }

  LayoutUnit RenderFragmentContainer::PageLogicalHeight() const noexcept
  {
    krys_debug_assert(IsValid());
    return _fragmentedFlow->IsHorizontalWritingMode() ? ContentBoxHeight() : ContentBoxWidth();
  }

  LayoutUnit RenderFragmentContainer::LogicalHeightOfAllFragmentedFlowContent() const noexcept
  {
    return PageLogicalHeight();
  }

  LayoutRect RenderFragmentContainer::FragmentedFlowPortionOverflowRect() const noexcept
  {
    return OverflowRectForFragmentedFlowPortion(FragmentedFlowPortionRect(), IsFirstFragment(),
                                                IsLastFragment());
  }

  LayoutPoint RenderFragmentContainer::FragmentedFlowPortionLocation() const noexcept
  {
    auto portionRect = FragmentedFlowPortionRect();
    if (FragmentedFlow()->GetWritingMode().IsBlockFlipped())
    {
      FragmentedFlow()->FlipForWritingMode(portionRect);
    }
    return portionRect.Location();
  }

  LayoutRect RenderFragmentContainer::OverflowRectForFragmentedFlowPortion(
    const LayoutRect &fragmentedFlowPortionRect, bool isFirstPortion, bool isLastPortion) const noexcept
  {
    krys_debug_assert(IsValid());

    // An infinite-like rect whose values are such that, when converted to float pixel values, they can
    // reasonably represent device pixels. Only clip along the block direction axis.
    LayoutRect clipRect(LayoutUnit::NearlyMin() / 32, LayoutUnit::NearlyMin() / 32,
                        LayoutUnit::NearlyMax() / 16, LayoutUnit::NearlyMax() / 16);

    if (_fragmentedFlow->IsHorizontalWritingMode())
    {
      if (!isFirstPortion)
      {
        clipRect.ShiftYEdgeTo(fragmentedFlowPortionRect.Y());
      }
      if (!isLastPortion)
      {
        clipRect.ShiftMaxYEdgeTo(fragmentedFlowPortionRect.MaxY());
      }
      return clipRect;
    }

    if (!isFirstPortion)
    {
      clipRect.ShiftXEdgeTo(fragmentedFlowPortionRect.X());
    }
    if (!isLastPortion)
    {
      clipRect.ShiftMaxXEdgeTo(fragmentedFlowPortionRect.MaxX());
    }
    return clipRect;
  }

  LayoutUnit RenderFragmentContainer::PageLogicalTopForOffset(LayoutUnit /*offset*/) const noexcept
  {
    return FragmentedFlow()->IsHorizontalWritingMode() ? FragmentedFlowPortionRect().Y()
                                                       : FragmentedFlowPortionRect().X();
  }

  bool RenderFragmentContainer::IsFirstFragment() const noexcept
  {
    krys_debug_assert(IsValid());
    return _fragmentedFlow->FirstFragment() == this;
  }

  bool RenderFragmentContainer::IsLastFragment() const noexcept
  {
    krys_debug_assert(IsValid());
    return _fragmentedFlow->LastFragment() == this;
  }

  bool RenderFragmentContainer::ShouldClipFragmentedFlowContent() const noexcept
  {
    return HasNonVisibleOverflow();
  }

  void RenderFragmentContainer::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlockFlow::StyleDidChange(diff, oldStyle);

    if (!IsValid())
    {
      return;
    }

    if (oldStyle
        && oldStyle->GetWritingMode().ComputedWritingMode() != GetWritingMode().ComputedWritingMode())
    {
      _fragmentedFlow->FragmentChangedWritingMode(this);
    }
  }

  void RenderFragmentContainer::RepaintFragmentedFlowContent(const LayoutRect &repaintRect) const noexcept
  {
    RepaintFragmentedFlowContentRectangle(repaintRect, FragmentedFlowPortionRect(),
                                          ContentBoxRect().Location());
  }

  void RenderFragmentContainer::RepaintFragmentedFlowContentRectangle(
    const LayoutRect &repaintRect, const LayoutRect &fragmentedFlowPortionRect,
    const LayoutPoint &fragmentLocation, const LayoutRect *fragmentedFlowPortionClipRect) const noexcept
  {
    krys_debug_assert(IsValid());

    // We only have to issue a repaint in this fragment if the fragment rect intersects the repaint rect.
    LayoutRect clippedRect(repaintRect);

    if (fragmentedFlowPortionClipRect)
    {
      LayoutRect flippedFragmentedFlowPortionClipRect(*fragmentedFlowPortionClipRect);
      FragmentedFlow()->FlipForWritingMode(flippedFragmentedFlowPortionClipRect);
      clippedRect.Intersect(flippedFragmentedFlowPortionClipRect);
    }

    if (clippedRect.IsEmpty())
    {
      return;
    }

    LayoutRect flippedFragmentedFlowPortionRect(fragmentedFlowPortionRect);
    // Put the fragment rects into physical coordinates.
    FragmentedFlow()->FlipForWritingMode(flippedFragmentedFlowPortionRect);

    // Put the fragment rect into the fragment's physical coordinate space.
    clippedRect.SetLocation(fragmentLocation
                            + (clippedRect.Location() - flippedFragmentedFlowPortionRect.Location()));

    // Now switch to the fragment's writing mode coordinate space and let it repaint itself.
    FlipForWritingMode(clippedRect);

    RepaintRectangle(clippedRect);
  }

  LayoutRect RenderFragmentContainer::FragmentedFlowContentRectangle(
    const LayoutRect &rect, const LayoutRect &fragmentedFlowPortionRect, const LayoutPoint &fragmentLocation,
    const LayoutRect *fragmentedFlowPortionClipRect) const noexcept
  {
    auto clippedRect = rect;

    if (fragmentedFlowPortionClipRect)
    {
      LayoutRect flippedFragmentedFlowPortionClipRect(*fragmentedFlowPortionClipRect);
      FragmentedFlow()->FlipForWritingMode(flippedFragmentedFlowPortionClipRect);
      // Edge-inclusive to avoid rects with zero height or width becoming zero-sized.
      (void)clippedRect.EdgeInclusiveIntersect(flippedFragmentedFlowPortionClipRect);
    }

    LayoutRect flippedFragmentedFlowPortionRect(fragmentedFlowPortionRect);
    FragmentedFlow()->FlipForWritingMode(flippedFragmentedFlowPortionRect);

    // Put the fragment rect into the fragment's physical coordinate space.
    clippedRect.SetLocation(fragmentLocation
                            + (clippedRect.Location() - flippedFragmentedFlowPortionRect.Location()));

    // Now switch to the fragment's writing mode coordinate space.
    FlipForWritingMode(clippedRect);

    return clippedRect;
  }

  List<LayoutRect>
    RenderFragmentContainer::FragmentRectsForFlowContentRect(const LayoutRect &contentRect) const noexcept
  {
    auto fragmentRect = contentRect;

    auto flippedFragmentedFlowPortionRect = FragmentedFlowPortionRect();
    FragmentedFlow()->FlipForWritingMode(flippedFragmentedFlowPortionRect);
    fragmentRect.SetLocation(ContentBoxRect().Location()
                             + (fragmentRect.Location() - flippedFragmentedFlowPortionRect.Location()));

    FlipForWritingMode(fragmentRect);

    return {fragmentRect};
  }

  void RenderFragmentContainer::InstallFragmentedFlow() noexcept
  {
    krys_debug_assert_msg(false, "Only fragment container sets are made.");
  }

  void RenderFragmentContainer::AttachFragment() noexcept
  {
    if (RenderTreeBeingDestroyed())
    {
      return;
    }

    // A fragment starts off invalid.
    SetIsValid(false);

    // Initialize the flow thread reference and create the flow thread object if needed.
    // The flow thread lifetime is influenced by the number of fragments attached to it,
    // and we are attaching the fragment to the flow thread.
    InstallFragmentedFlow();

    if (!_fragmentedFlow)
    {
      return;
    }

    // Only after adding the fragment to the thread, the fragment is marked to be valid.
    _fragmentedFlow->AddFragmentToThread(this);
  }

  void RenderFragmentContainer::DetachFragment() noexcept
  {
    if (_fragmentedFlow)
    {
      _fragmentedFlow->RemoveFragmentFromThread(*this);
    }
    _fragmentedFlow = nullptr;
  }

  RenderBoxFragmentInfo *
    RenderFragmentContainer::GetRenderBoxFragmentInfo(const RenderBox &box) const noexcept
  {
    krys_debug_assert(IsValid());
    auto it = _renderBoxFragmentInfo.find(&box);
    return it != _renderBoxFragmentInfo.end() ? it->second.get() : nullptr;
  }

  RenderBoxFragmentInfo *
    RenderFragmentContainer::SetRenderBoxFragmentInfo(const RenderBox &box, LayoutUnit logicalLeftInset,
                                                      LayoutUnit logicalRightInset,
                                                      bool containingBlockChainIsInset) noexcept
  {
    krys_debug_assert(IsValid());

    auto [it, isNewEntry] = _renderBoxFragmentInfo.try_emplace(&box);
    if (isNewEntry)
    {
      it->second =
        CreateUnique<RenderBoxFragmentInfo>(logicalLeftInset, logicalRightInset, containingBlockChainIsInset);
    }
    return it->second.get();
  }

  UniquePtr<RenderBoxFragmentInfo>
    RenderFragmentContainer::TakeRenderBoxFragmentInfo(const RenderBox &box) noexcept
  {
    auto node = _renderBoxFragmentInfo.extract(&box);
    return node ? krys::move(node.mapped()) : nullptr;
  }

  void RenderFragmentContainer::RemoveRenderBoxFragmentInfo(const RenderBox &box) noexcept
  {
    _renderBoxFragmentInfo.erase(&box);
  }

  void RenderFragmentContainer::DeleteAllRenderBoxFragmentInfo() noexcept
  {
    _renderBoxFragmentInfo.clear();
  }

  LayoutUnit
    RenderFragmentContainer::LogicalTopOfFragmentedFlowContentRect(const LayoutRect &rect) const noexcept
  {
    krys_debug_assert(IsValid());
    return FragmentedFlow()->IsHorizontalWritingMode() ? rect.Y() : rect.X();
  }

  LayoutUnit
    RenderFragmentContainer::LogicalBottomOfFragmentedFlowContentRect(const LayoutRect &rect) const noexcept
  {
    krys_debug_assert(IsValid());
    return FragmentedFlow()->IsHorizontalWritingMode() ? rect.MaxY() : rect.MaxX();
  }

  void RenderFragmentContainer::InsertedIntoTree() noexcept
  {
    AttachFragment();
    if (IsValid())
    {
      RenderBlockFlow::InsertedIntoTree();
    }
  }

  void RenderFragmentContainer::WillBeRemovedFromTree() noexcept
  {
    RenderBlockFlow::WillBeRemovedFromTree();

    DetachFragment();
  }

  void RenderFragmentContainer::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                              LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (!IsValid())
    {
      RenderBlockFlow::ComputeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);
      return;
    }
    maxLogicalWidth = 0_lu;
    minLogicalWidth = 0_lu;
  }

  void RenderFragmentContainer::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    if (!IsValid())
    {
      RenderBlockFlow::ComputePreferredLogicalWidths();
      return;
    }

    // FIXME: Currently, the code handles only the <length> case for min-width/max-width.
    // It should also support other values, like percentage, calc or viewport relative.
    _minPreferredLogicalWidth = 0_lu;
    _maxPreferredLogicalWidth = 0_lu;

    const auto &styleToUse = Style();
    if (auto fixedLogicalWidth = styleToUse.LogicalWidth().TryFixed();
        fixedLogicalWidth && fixedLogicalWidth->IsPositive())
    {
      _minPreferredLogicalWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(styleToUse.UsedZoomForLength())});
      _maxPreferredLogicalWidth = _minPreferredLogicalWidth;
    }
    else
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }

    RenderBox::ComputePreferredLogicalWidths(styleToUse.LogicalMinWidth(), styleToUse.LogicalMaxWidth(),
                                             BorderAndPaddingLogicalWidth());

    ClearNeedsPreferredWidthsUpdate();
  }

  LayoutRect RenderFragmentContainer::ComputedVisualOverflowRectForBox(const RenderBox &box) const noexcept
  {
    krys_debug_assert(_fragmentedFlow->ObjectShouldFragmentInFlowFragment(&box, this));

    auto borderBox = box.BorderBoxRect();
    if (borderBox.IsEmpty())
    {
      return {};
    }

    borderBox = RectFlowPortionForBox(box, borderBox);
    _fragmentedFlow->FlipForWritingModeLocalCoordinates(borderBox);
    return borderBox;
  }

  LayoutRect RenderFragmentContainer::ComputedLayoutOverflowRectForBox(const RenderBox &box) const noexcept
  {
    krys_debug_assert(_fragmentedFlow->ObjectShouldFragmentInFlowFragment(&box, this));

    auto clientBox = box.ClientBoxRect();
    if (clientBox.IsEmpty())
    {
      return {};
    }

    clientBox = RectFlowPortionForBox(box, clientBox);
    _fragmentedFlow->FlipForWritingModeLocalCoordinates(clientBox);
    return clientBox;
  }

  RenderOverflow *RenderFragmentContainer::OverflowForBox(const RenderBox &box) const noexcept
  {
    krys_debug_assert(IsValid());

    auto *boxInfo = GetRenderBoxFragmentInfo(box);
    if (!boxInfo)
    {
      return nullptr;
    }

    if (auto *overflow = boxInfo->Overflow())
    {
      return overflow;
    }

    boxInfo->CreateOverflow(ComputedLayoutOverflowRectForBox(box), ComputedVisualOverflowRectForBox(box));
    return boxInfo->Overflow();
  }

  LayoutRect RenderFragmentContainer::RectFlowPortionForBox(const RenderBox &box,
                                                            const LayoutRect &rect) const noexcept
  {
    LayoutRect mappedRect = _fragmentedFlow->MapFromLocalToFragmentedFlow(&box, rect);

    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (_fragmentedFlow->GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      if (FragmentedFlow()->IsHorizontalWritingMode())
      {
        if (this != startFragment)
        {
          mappedRect.ShiftYEdgeTo(std::max<LayoutUnit>(LogicalTopForFragmentedFlowContent(), mappedRect.Y()));
        }
        if (this != endFragment)
        {
          mappedRect.SetHeight(std::max<LayoutUnit>(
            0_lu, std::min<LayoutUnit>(LogicalBottomForFragmentedFlowContent() - mappedRect.Y(),
                                       mappedRect.Height())));
        }
      }
      else
      {
        if (this != startFragment)
        {
          mappedRect.ShiftXEdgeTo(std::max<LayoutUnit>(LogicalTopForFragmentedFlowContent(), mappedRect.X()));
        }
        if (this != endFragment)
        {
          mappedRect.SetWidth(std::max<LayoutUnit>(
            0_lu, std::min<LayoutUnit>(LogicalBottomForFragmentedFlowContent() - mappedRect.X(),
                                       mappedRect.Width())));
        }
      }
    }

    return _fragmentedFlow->MapFromFragmentedFlowToLocal(&box, mappedRect);
  }

  void RenderFragmentContainer::AddLayoutOverflowForBox(const RenderBox &box, const LayoutRect &rect) noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    if (auto *overflow = OverflowForBox(box))
    {
      overflow->AddLayoutOverflow(rect);
    }
  }

  void RenderFragmentContainer::AddVisualOverflowForBox(const RenderBox &box, const LayoutRect &rect) noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    if (auto *overflow = OverflowForBox(box))
    {
      LayoutRect flippedRect = rect;
      FragmentedFlow()->FlipForWritingModeLocalCoordinates(flippedRect);
      overflow->AddVisualOverflow(flippedRect);
    }
  }

  LayoutRect RenderFragmentContainer::VisualOverflowRectForBox(const RenderBox &box) const noexcept
  {
    if (auto *overflow = OverflowForBox(box))
    {
      return overflow->VisualOverflowRect();
    }
    return ComputedVisualOverflowRectForBox(box);
  }

  // FIXME: This doesn't work for writing modes.
  LayoutRect RenderFragmentContainer::LayoutOverflowRectForBoxForPropagation(const RenderBox &box) noexcept
  {
    // Only propagate interior layout overflow if we don't clip it.
    LayoutRect rect = box.BorderBoxRect();
    rect = RectFlowPortionForBox(box, rect);
    if (!box.HasNonVisibleOverflow())
    {
      auto *overflow = OverflowForBox(box);
      rect.Unite(overflow ? overflow->LayoutOverflowRect() : ComputedLayoutOverflowRectForBox(box));
    }

    bool hasTransform = box.IsTransformed();
    if (box.IsInFlowPositioned() || hasTransform)
    {
      if (hasTransform)
      {
        rect = box.Layer()->CurrentTransform().MapRect(rect);
      }

      if (box.IsInFlowPositioned())
      {
        rect.Move(box.OffsetForInFlowPosition());
      }
    }

    return rect;
  }

  LayoutRect RenderFragmentContainer::VisualOverflowRectForBoxForPropagation(const RenderBox &box) noexcept
  {
    LayoutRect rect = VisualOverflowRectForBox(box);
    FragmentedFlow()->FlipForWritingModeLocalCoordinates(rect);
    return rect;
  }

  CurrentRenderFragmentContainerMaintainer::CurrentRenderFragmentContainerMaintainer(
    RenderFragmentContainer &fragment) noexcept
      : _fragment(fragment)
  {
    auto *fragmentedFlow = fragment.FragmentedFlow();
    // A flow thread can have only one current fragment.
    krys_debug_assert(!fragmentedFlow->CurrentFragment());
    fragmentedFlow->SetCurrentFragmentMaintainer(this);
  }

  CurrentRenderFragmentContainerMaintainer::~CurrentRenderFragmentContainerMaintainer() noexcept
  {
    _fragment.FragmentedFlow()->SetCurrentFragmentMaintainer(nullptr);
  }
}
