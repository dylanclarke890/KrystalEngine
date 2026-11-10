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

#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentedFlow.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/Fragmentation/RenderBoxFragmentInfo.hpp"
#include "Krystal.Booey/Rendering/Fragmentation/RenderFragmentContainer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableSection.hpp"
#include <algorithm>

namespace krys::boo
{
  void RenderFragmentedFlow::RenderFragmentContainerRange::SetRange(RenderFragmentContainer *start,
                                                                    RenderFragmentContainer *end) noexcept
  {
    _startFragment = CreateWeakPtr(start);
    _endFragment = CreateWeakPtr(end);
    _rangeInvalidated = true;
  }

  RenderFragmentContainer *RenderFragmentedFlow::RenderFragmentContainerRange::StartFragment() const noexcept
  {
    return _startFragment.get();
  }

  RenderFragmentContainer *RenderFragmentedFlow::RenderFragmentContainerRange::EndFragment() const noexcept
  {
    return _endFragment.get();
  }

  RenderFragmentedFlow::RenderFragmentedFlow(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlockFlow(type, document, krys::move(style), {BlockFlowFlag::IsFragmentedFlow})
  {
    krys_debug_assert(IsRenderFragmentedFlow());
  }

  RenderFragmentedFlow::~RenderFragmentedFlow() noexcept = default;

  Span<RenderFragmentContainer *const>
    RenderFragmentedFlow::FragmentsFrom(const RenderFragmentContainer &startFragment) const noexcept
  {
    auto it = std::find(_fragmentList.begin(), _fragmentList.end(), &startFragment);
    krys_debug_assert(it != _fragmentList.end());
    return Span<RenderFragmentContainer *const>(_fragmentList).subspan(it - _fragmentList.begin());
  }

  void RenderFragmentedFlow::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlockFlow::StyleDidChange(diff, oldStyle);

    if (oldStyle
        && oldStyle->GetWritingMode().ComputedWritingMode() != GetWritingMode().ComputedWritingMode())
    {
      InvalidateFragments();
    }
  }

  void RenderFragmentedFlow::RemoveFlowChildInfo(RenderElement &child) noexcept
  {
    if (auto *box = DynamicDowncast<RenderBox>(child))
    {
      RemoveRenderBoxFragmentInfo(*box);
    }
  }

  void
    RenderFragmentedFlow::RemoveFragmentFromThread(RenderFragmentContainer &renderFragmentContainer) noexcept
  {
    std::erase(_fragmentList, &renderFragmentContainer);
  }

  void RenderFragmentedFlow::InvalidateFragments(MarkingBehavior markingParents) noexcept
  {
    if (_fragmentsInvalidated)
    {
      krys_debug_assert(SelfNeedsLayout());
      return;
    }

    _fragmentRangeMap.clear();
    SetNeedsLayout(markingParents);

    _fragmentsInvalidated = true;
  }

  void RenderFragmentedFlow::ValidateFragments() noexcept
  {
    if (_fragmentsInvalidated)
    {
      _fragmentsInvalidated = false;
      _fragmentsHaveUniformLogicalWidth = true;
      _fragmentsHaveUniformLogicalHeight = true;

      if (HasFragments())
      {
        LayoutUnit previousFragmentLogicalWidth;
        LayoutUnit previousFragmentLogicalHeight;
        bool firstFragmentVisited = false;

        for (auto *fragment : _fragmentList)
        {
          krys_debug_assert(!fragment->NeedsLayout() || fragment->IsRenderFragmentContainerSet());

          fragment->DeleteAllRenderBoxFragmentInfo();

          LayoutUnit fragmentLogicalWidth = fragment->PageLogicalWidth();
          LayoutUnit fragmentLogicalHeight = fragment->PageLogicalHeight();

          if (!firstFragmentVisited)
          {
            firstFragmentVisited = true;
          }
          else
          {
            if (_fragmentsHaveUniformLogicalWidth && previousFragmentLogicalWidth != fragmentLogicalWidth)
            {
              _fragmentsHaveUniformLogicalWidth = false;
            }
            if (_fragmentsHaveUniformLogicalHeight && previousFragmentLogicalHeight != fragmentLogicalHeight)
            {
              _fragmentsHaveUniformLogicalHeight = false;
            }
          }

          previousFragmentLogicalWidth = fragmentLogicalWidth;
          previousFragmentLogicalHeight = fragmentLogicalHeight;
        }

        SetFragmentRangeForBox(*this, _fragmentList.front(), _fragmentList.back());
      }
    }

    UpdateLogicalWidth(); // Called to get the maximum logical width for the fragment.
    UpdateFragmentsFragmentedFlowPortionRect();
  }

  void RenderFragmentedFlow::Layout() noexcept
  {
    _pageLogicalSizeChanged = _fragmentsInvalidated && EverHadLayout();

    ValidateFragments();

    RenderBlockFlow::Layout();

    _pageLogicalSizeChanged = false;
  }

  void RenderFragmentedFlow::UpdateLogicalWidth() noexcept
  {
    LayoutUnit logicalWidth = InitialLogicalWidth();
    for (auto *fragment : _fragmentList)
    {
      krys_debug_assert(!fragment->NeedsLayout() || fragment->IsRenderFragmentContainerSet());
      logicalWidth = std::max(fragment->PageLogicalWidth(), logicalWidth);
    }
    SetLogicalWidth(logicalWidth);

    // If the fragments have non-uniform logical widths, then insert inset information for the
    // RenderFragmentedFlow.
    for (auto *fragment : _fragmentList)
    {
      LayoutUnit fragmentLogicalWidth = fragment->PageLogicalWidth();
      LayoutUnit logicalLeft =
        GetWritingMode().IsLogicalLeftInlineStart() ? 0_lu : logicalWidth - fragmentLogicalWidth;
      fragment->SetRenderBoxFragmentInfo(*this, logicalLeft, fragmentLogicalWidth, false);
    }
  }

  RenderBox::LogicalExtentComputedValues
    RenderFragmentedFlow::ComputeLogicalHeight(LayoutUnit, LayoutUnit logicalTop) const noexcept
  {
    LogicalExtentComputedValues computedValues;
    computedValues.Position = logicalTop;
    computedValues.Extent = 0_lu;

    const LayoutUnit maxFlowSize = MaxLogicalHeight();
    for (auto *fragment : _fragmentList)
    {
      krys_debug_assert(!fragment->NeedsLayout() || fragment->IsRenderFragmentContainerSet());

      LayoutUnit distanceToMaxSize = maxFlowSize - computedValues.Extent;
      computedValues.Extent +=
        std::min(distanceToMaxSize, fragment->LogicalHeightOfAllFragmentedFlowContent());

      // If we reached the maximum size there's no point in going further.
      if (computedValues.Extent == maxFlowSize)
      {
        return computedValues;
      }
    }
    return computedValues;
  }

  bool RenderFragmentedFlow::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                         const HitTestLocation &locationInContainer,
                                         const LayoutPoint &accumulatedOffset,
                                         HitTestAction hitTestAction) noexcept
  {
    if (hitTestAction == HitTestBlockBackground)
    {
      return false;
    }
    return RenderBlockFlow::NodeAtPoint(request, result, locationInContainer, accumulatedOffset,
                                        hitTestAction);
  }

  bool RenderFragmentedFlow::ShouldRepaint(const LayoutRect &rect) const noexcept
  {
    return !rect.IsEmpty();
  }

  void RenderFragmentedFlow::RepaintRectangleInFragments(const LayoutRect &repaintRect) const noexcept
  {
    if (!ShouldRepaint(repaintRect) || !HasValidFragmentInfo())
    {
      return;
    }

    // We can't use layout state to repaint, since the fragments are somewhere else.
    LayoutStateDisabler layoutStateDisabler(LayoutContext());

    for (auto *fragment : _fragmentList)
    {
      fragment->RepaintFragmentedFlowContent(repaintRect);
    }
  }

  bool RenderFragmentedFlow::AbsoluteQuadsForBox(List<FloatQuad> &quads, bool *wasFixed,
                                                 const RenderBox &box) const noexcept
  {
    if (!HasValidFragmentInfo())
    {
      return false;
    }

    auto boxRect = FloatRect {{}, box.Size()};
    auto boxRectInFlowCoordinates = LayoutRect {box.LocalToContainerQuad(boxRect, this).BoundingBox()};

    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!ComputedFragmentRangeForBox(box, startFragment, endFragment))
    {
      return false;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      for (auto rect : fragment->FragmentRectsForFlowContentRect(boxRectInFlowCoordinates))
      {
        quads.push_back(
          fragment->LocalToAbsoluteQuad(FloatRect(rect), MapCoordinatesMode::UseTransforms, wasFixed));
      }

      if (fragment == endFragment)
      {
        break;
      }
    }

    return true;
  }

  bool RenderFragmentedFlow::BoxIsFragmented(const RenderBox &box) const noexcept
  {
    krys_debug_assert(HasValidFragmentInfo());

    auto boxRect = FloatRect {{}, box.Size()};
    auto boxRectInFlowCoordinates = LayoutRect {box.LocalToContainerQuad(boxRect, this).BoundingBox()};

    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    (void)ComputedFragmentRangeForBox(box, startFragment, endFragment);
    if (startFragment != endFragment)
    {
      return true;
    }

    return startFragment->ContentRectSpansFragments(boxRectInFlowCoordinates);
  }

  RenderFragmentContainer *RenderFragmentedFlow::FragmentAtBlockOffset(const RenderBox *clampBox,
                                                                       LayoutUnit offset,
                                                                       bool extendLastFragment) const noexcept
  {
    krys_debug_assert(!_fragmentsInvalidated);

    if (_fragmentList.empty())
    {
      return nullptr;
    }

    if (_fragmentList.size() == 1 && extendLastFragment)
    {
      return _fragmentList.front();
    }

    auto clamp = [clampBox](RenderFragmentContainer *fragment)
    {
      return clampBox ? clampBox->ClampToStartAndEndFragments(fragment) : fragment;
    };

    if (offset <= 0)
    {
      return clamp(_fragmentList.front());
    }

    // The intervals are contiguous and in order, so the first one ending after the offset is the only one
    // that can hold it.
    auto it = std::upper_bound(_fragmentIntervals.begin(), _fragmentIntervals.end(), offset,
                               [](LayoutUnit value, const FragmentInterval &interval)
                               { return value < interval.High; });
    if (it != _fragmentIntervals.end() && it->Low <= offset)
    {
      return clamp(it->Fragment);
    }

    // If no fragment was found, the offset is in the flow thread overflow.
    // The last fragment will contain the offset if extendLastFragment is set or if the last fragment is a
    // set.
    if (extendLastFragment || _fragmentList.back()->IsRenderFragmentContainerSet())
    {
      return clamp(_fragmentList.back());
    }

    return nullptr;
  }

  LayoutUnit RenderFragmentedFlow::PageLogicalTopForOffset(LayoutUnit offset) const noexcept
  {
    auto *fragment = FragmentAtBlockOffset(nullptr, offset, false);
    return fragment ? fragment->PageLogicalTopForOffset(offset) : 0_lu;
  }

  LayoutUnit RenderFragmentedFlow::PageLogicalWidthForOffset(LayoutUnit offset) const noexcept
  {
    auto *fragment = FragmentAtBlockOffset(nullptr, offset, true);
    return fragment ? fragment->PageLogicalWidth() : ContentBoxLogicalWidth();
  }

  LayoutUnit RenderFragmentedFlow::PageLogicalHeightForOffset(LayoutUnit offset) const noexcept
  {
    auto *fragment = FragmentAtBlockOffset(nullptr, offset, false);
    return fragment ? fragment->PageLogicalHeight() : 0_lu;
  }

  LayoutUnit RenderFragmentedFlow::PageRemainingLogicalHeightForOffset(
    LayoutUnit offset, PageBoundaryRule pageBoundaryRule) const noexcept
  {
    auto *fragment = FragmentAtBlockOffset(nullptr, offset, false);
    if (!fragment)
    {
      return 0_lu;
    }

    LayoutUnit pageLogicalTop = fragment->PageLogicalTopForOffset(offset);
    LayoutUnit pageLogicalHeight = fragment->PageLogicalHeight();
    LayoutUnit pageLogicalBottom = pageLogicalTop + pageLogicalHeight;
    LayoutUnit remainingHeight = pageLogicalBottom - offset;
    if (pageBoundaryRule == PageBoundaryRule::IncludePageBoundary)
    {
      // If IncludePageBoundary is set, the line exactly on the top edge of a
      // fragment will act as being part of the previous fragment.
      remainingHeight = IntMod(remainingHeight, pageLogicalHeight);
    }
    else if (!remainingHeight)
    {
      // When pageBoundaryRule is ExcludePageBoundary, we shouldn't just return 0 if there's no
      // space left, because in that case we're at a column boundary, in which case we should
      // return the amount of space remaining in the *next* column. Note that the page height
      // itself may be 0, though.
      remainingHeight = pageLogicalHeight;
    }
    return remainingHeight;
  }

  RenderFragmentContainer *
    RenderFragmentedFlow::MapFromFlowToFragment(TransformState &transformState) const noexcept
  {
    if (!HasValidFragmentInfo())
    {
      return nullptr;
    }

    auto *fragment = CurrentFragment();
    if (!fragment)
    {
      LayoutRect boxRect = transformState.MappedQuad().EnclosingBoundingBox();
      FlipForWritingMode(boxRect);

      LayoutPoint center = boxRect.Center();
      fragment = FragmentAtBlockOffset(this, IsHorizontalWritingMode() ? center.Y() : center.X(), true);
      if (!fragment)
      {
        return nullptr;
      }
    }

    LayoutRect flippedFragmentRect(fragment->FragmentedFlowPortionRect());
    FlipForWritingMode(flippedFragmentRect);

    transformState.Move(fragment->ContentBoxRect().Location() - flippedFragmentRect.Location());

    return fragment;
  }

  void RenderFragmentedFlow::RemoveRenderBoxFragmentInfo(const RenderBox &box) noexcept
  {
    if (!HasFragments())
    {
      return;
    }

    // If the fragment chain was invalidated the next layout will clear the box information from all the
    // fragments.
    if (_fragmentsInvalidated)
    {
      krys_debug_assert(SelfNeedsLayout());
      return;
    }

    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      for (auto *fragment : FragmentsFrom(*startFragment))
      {
        fragment->RemoveRenderBoxFragmentInfo(box);
        if (fragment == endFragment)
        {
          break;
        }
      }
    }

#if KRYS_DEBUG
    // We have to make sure we did not leave any RenderBoxFragmentInfo attached.
    for (auto *fragment : _fragmentList)
    {
      krys_debug_assert(!fragment->GetRenderBoxFragmentInfo(box));
    }
#endif

    _fragmentRangeMap.erase(&box);
  }

  void
    RenderFragmentedFlow::LogicalWidthChangedInFragmentsForBlock(const RenderBlock &block,
                                                                 RelayoutChildren &relayoutChildren) noexcept
  {
    if (!HasValidFragmentInfo())
    {
      return;
    }

    auto it = _fragmentRangeMap.find(&block);
    if (it == _fragmentRangeMap.end())
    {
      return;
    }

    auto &range = it->second;
    bool rangeInvalidated = range.RangeInvalidated();
    range.ClearRangeInvalidated();

    // If there will be a relayout anyway skip the next steps because they only verify
    // the state of the ranges.
    if (relayoutChildren == RelayoutChildren::Yes)
    {
      return;
    }

    // Not necessary for the flow thread, since we already computed the correct info for it.
    // If the fragments have changed invalidate the children.
    if (&block == this)
    {
      relayoutChildren = _pageLogicalSizeChanged ? RelayoutChildren::Yes : RelayoutChildren::No;
      return;
    }

    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(block, startFragment, endFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      krys_debug_assert(!fragment->NeedsLayout() || fragment->IsRenderFragmentContainerSet());

      // We have no information computed for this fragment so we need to do it.
      auto oldInfo = fragment->TakeRenderBoxFragmentInfo(block);
      if (!oldInfo)
      {
        relayoutChildren = rangeInvalidated ? RelayoutChildren::Yes : RelayoutChildren::No;
        return;
      }

      LayoutUnit oldLogicalWidth = oldInfo->LogicalWidth();
      auto *newInfo = block.GetRenderBoxFragmentInfo(fragment);
      if (!newInfo || newInfo->LogicalWidth() != oldLogicalWidth)
      {
        relayoutChildren = RelayoutChildren::Yes;
        return;
      }

      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  LayoutUnit RenderFragmentedFlow::ContentLogicalWidthOfFirstFragment() const noexcept
  {
    auto *firstValidFragmentInFlow = FirstFragment();
    if (!firstValidFragmentInFlow)
    {
      return 0_lu;
    }
    return IsHorizontalWritingMode() ? firstValidFragmentInFlow->ContentBoxWidth()
                                     : firstValidFragmentInFlow->ContentBoxHeight();
  }

  LayoutUnit RenderFragmentedFlow::ContentLogicalHeightOfFirstFragment() const noexcept
  {
    auto *firstValidFragmentInFlow = FirstFragment();
    if (!firstValidFragmentInFlow)
    {
      return 0_lu;
    }
    return IsHorizontalWritingMode() ? firstValidFragmentInFlow->ContentBoxHeight()
                                     : firstValidFragmentInFlow->ContentBoxWidth();
  }

  LayoutUnit RenderFragmentedFlow::ContentLogicalLeftOfFirstFragment() const noexcept
  {
    auto *firstValidFragmentInFlow = FirstFragment();
    if (!firstValidFragmentInFlow)
    {
      return 0_lu;
    }
    return IsHorizontalWritingMode() ? firstValidFragmentInFlow->FragmentedFlowPortionRect().X()
                                     : firstValidFragmentInFlow->FragmentedFlowPortionRect().Y();
  }

  RenderFragmentContainer *RenderFragmentedFlow::FirstFragment() const noexcept
  {
    return HasFragments() ? _fragmentList.front() : nullptr;
  }

  RenderFragmentContainer *RenderFragmentedFlow::LastFragment() const noexcept
  {
    return HasFragments() ? _fragmentList.back() : nullptr;
  }

  void RenderFragmentedFlow::ClearRenderBoxFragmentInfoAndCustomStyle(
    const RenderBox &box, const RenderFragmentContainer *newStartFragment,
    const RenderFragmentContainer *newEndFragment, const RenderFragmentContainer *oldStartFragment,
    const RenderFragmentContainer *oldEndFragment) noexcept
  {
    krys_debug_assert(newStartFragment && newEndFragment && oldStartFragment && oldEndFragment);

    bool insideOldFragmentRange = false;
    bool insideNewFragmentRange = false;
    for (auto *fragment : _fragmentList)
    {
      if (oldStartFragment == fragment)
      {
        insideOldFragmentRange = true;
      }
      if (newStartFragment == fragment)
      {
        insideNewFragmentRange = true;
      }

      if (!(insideOldFragmentRange && insideNewFragmentRange))
      {
        if (fragment->GetRenderBoxFragmentInfo(box))
        {
          fragment->RemoveRenderBoxFragmentInfo(box);
        }
      }

      if (oldEndFragment == fragment)
      {
        insideOldFragmentRange = false;
      }
      if (newEndFragment == fragment)
      {
        insideNewFragmentRange = false;
      }
    }
  }

  void RenderFragmentedFlow::SetFragmentRangeForBox(const RenderBox &box,
                                                    RenderFragmentContainer *startFragment,
                                                    RenderFragmentContainer *endFragment) noexcept
  {
    krys_debug_assert(HasFragments());
    krys_debug_assert(startFragment && endFragment && startFragment->FragmentedFlow() == this
                      && endFragment->FragmentedFlow() == this);
    auto [it, isNewEntry] =
      _fragmentRangeMap.try_emplace(&box, RenderFragmentContainerRange(startFragment, endFragment));
    if (isNewEntry)
    {
      return;
    }

    // If nothing changed, just bail.
    auto &range = it->second;
    if (range.StartFragment() == startFragment && range.EndFragment() == endFragment)
    {
      return;
    }
    ClearRenderBoxFragmentInfoAndCustomStyle(box, startFragment, endFragment, range.StartFragment(),
                                             range.EndFragment());
    range.SetRange(startFragment, endFragment);
  }

  bool RenderFragmentedFlow::HasCachedFragmentRangeForBox(const RenderBox &box) const noexcept
  {
    return _fragmentRangeMap.contains(&box);
  }

  bool RenderFragmentedFlow::GetFragmentRangeForBoxFromCachedInfo(
    const RenderBox &box, RenderFragmentContainer *&startFragment,
    RenderFragmentContainer *&endFragment) const noexcept
  {
    krys_debug_assert(HasValidFragmentInfo());
    krys_debug_assert(!startFragment && !endFragment);

    auto it = _fragmentRangeMap.find(&box);
    if (it == _fragmentRangeMap.end())
    {
      return false;
    }

    startFragment = it->second.StartFragment();
    endFragment = it->second.EndFragment();
    krys_debug_assert(std::ranges::contains(_fragmentList, startFragment)
                      && std::ranges::contains(_fragmentList, endFragment));
    return true;
  }

  bool RenderFragmentedFlow::GetFragmentRangeForBox(const RenderBox &box,
                                                    RenderFragmentContainer *&startFragment,
                                                    RenderFragmentContainer *&endFragment) const noexcept
  {
    startFragment = endFragment = nullptr;
    // We clear the ranges when we invalidate the fragments.
    if (!HasValidFragmentInfo())
    {
      return false;
    }

    if (_fragmentList.size() == 1)
    {
      startFragment = endFragment = _fragmentList.front();
      return true;
    }

    return GetFragmentRangeForBoxFromCachedInfo(box, startFragment, endFragment);
  }

  bool RenderFragmentedFlow::ComputedFragmentRangeForBox(const RenderBox &box,
                                                         RenderFragmentContainer *&startFragment,
                                                         RenderFragmentContainer *&endFragment) const noexcept
  {
    startFragment = endFragment = nullptr;
    // We clear the ranges when we invalidate the fragments.
    if (!HasValidFragmentInfo())
    {
      return false;
    }

    if (GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      return true;
    }

    // Search the fragment range using the information provided by the containing block chain.
    const auto *containingBlock = &box;
    while (!containingBlock->IsRenderFragmentedFlow())
    {
      // FIXME: Use the ContainingBlock() value once we patch all the layout systems to be fragment range
      // aware.
      krys_debug_assert(containingBlock->Parent());
      containingBlock = &containingBlock->Parent()->EnclosingBox();

      // If a box doesn't have a cached fragment range it usually means the box belongs to a line so
      // startFragment should be equal with endFragment.
      // FIXME: Find the cases when this startFragment should not be equal with endFragment and make sure
      // these boxes have cached fragment ranges.
      if (HasCachedFragmentRangeForBox(*containingBlock))
      {
        startFragment = endFragment =
          FragmentAtBlockOffset(containingBlock, containingBlock->OffsetFromLogicalTopOfFirstPage(), true);
        return true;
      }
    }
    krys_debug_assert_msg(false, "A box in the flow has a containing block with a fragment range.");
    return false;
  }

  bool RenderFragmentedFlow::FragmentInRange(const RenderFragmentContainer *targetFragment,
                                             const RenderFragmentContainer *startFragment,
                                             const RenderFragmentContainer *endFragment) const noexcept
  {
    krys_debug_assert(targetFragment);

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      if (targetFragment == fragment)
      {
        return true;
      }
      if (fragment == endFragment)
      {
        break;
      }
    }

    return false;
  }

  bool RenderFragmentedFlow::ObjectShouldFragmentInFlowFragment(
    const RenderObject *object, const RenderFragmentContainer *fragment) const noexcept
  {
    krys_debug_assert(object);

    if (object->EnclosingFragmentedFlow() != this)
    {
      return false;
    }

    if (!fragment || !std::ranges::contains(_fragmentList, fragment))
    {
      return false;
    }

    RenderFragmentContainer *enclosingBoxStartFragment = nullptr;
    RenderFragmentContainer *enclosingBoxEndFragment = nullptr;
    // If the box has no range, do not check FragmentInRange. Boxes inside inlines do not get ranges.
    // Instead, the containing line will abort when trying to paint inside the wrong fragment.
    if (ComputedFragmentRangeForBox(object->EnclosingBox(), enclosingBoxStartFragment,
                                    enclosingBoxEndFragment)
        && !FragmentInRange(fragment, enclosingBoxStartFragment, enclosingBoxEndFragment))
    {
      return false;
    }

    return object->IsRenderBox() || object->IsRenderInline();
  }

  bool RenderFragmentedFlow::ObjectInFlowFragment(const RenderObject *object,
                                                  const RenderFragmentContainer *fragment) const noexcept
  {
    krys_debug_assert(object);
    krys_debug_assert(fragment);

    if (object->EnclosingFragmentedFlow() != this)
    {
      return false;
    }

    if (!std::ranges::contains(_fragmentList, fragment))
    {
      return false;
    }

    RenderFragmentContainer *enclosingBoxStartFragment = nullptr;
    RenderFragmentContainer *enclosingBoxEndFragment = nullptr;
    if (!GetFragmentRangeForBox(object->EnclosingBox(), enclosingBoxStartFragment, enclosingBoxEndFragment))
    {
      return false;
    }

    if (!FragmentInRange(fragment, enclosingBoxStartFragment, enclosingBoxEndFragment))
    {
      return false;
    }

    if (object->IsRenderBox())
    {
      return true;
    }

    LayoutRect objectABBRect = object->AbsoluteBoundingBoxRect(true);
    if (!objectABBRect.Width())
    {
      objectABBRect.SetWidth(1_lu);
    }
    if (!objectABBRect.Height())
    {
      objectABBRect.SetHeight(1_lu);
    }
    if (objectABBRect.Intersects(fragment->AbsoluteBoundingBoxRect(true)))
    {
      return true;
    }

    if (fragment == LastFragment())
    {
      // If the object does not intersect any of the enclosing box fragments
      // then the object is in last fragment.
      for (auto *currentFragment : FragmentsFrom(*enclosingBoxStartFragment))
      {
        if (currentFragment == fragment)
        {
          break;
        }
        if (objectABBRect.Intersects(currentFragment->AbsoluteBoundingBoxRect(true)))
        {
          return false;
        }
      }
      return true;
    }

    return false;
  }

  void RenderFragmentedFlow::UpdateFragmentsFragmentedFlowPortionRect() noexcept
  {
    LayoutUnit logicalHeight;
    _fragmentIntervals.clear();
    for (auto *fragment : _fragmentList)
    {
      LayoutUnit fragmentLogicalWidth = fragment->PageLogicalWidth();
      LayoutUnit fragmentLogicalHeight = std::min<LayoutUnit>(
        MaxLogicalHeight() - logicalHeight, fragment->LogicalHeightOfAllFragmentedFlowContent());

      LayoutRect fragmentRect(
        GetWritingMode().IsLogicalLeftInlineStart() ? 0_lu : LogicalWidth() - fragmentLogicalWidth,
        logicalHeight, fragmentLogicalWidth, fragmentLogicalHeight);

      fragment->SetFragmentedFlowPortionRect(IsHorizontalWritingMode() ? fragmentRect
                                                                       : fragmentRect.TransposedRect());

      _fragmentIntervals.push_back({logicalHeight, logicalHeight + fragmentLogicalHeight, fragment});

      logicalHeight += fragmentLogicalHeight;
    }
  }

  // Even if we require the break to occur at offsetBreakInFragmentedFlow, because fragments may have
  // min/max-height values, it is possible that the break will occur at a different offset than the original
  // one required. offsetBreakAdjustment measures the different between the requested break offset and the
  // current break offset.
  bool RenderFragmentedFlow::AddForcedFragmentBreak(const RenderBlock *block,
                                                    LayoutUnit offsetBreakInFragmentedFlow, RenderBox *, bool,
                                                    LayoutUnit *offsetBreakAdjustment) noexcept
  {
    // We need to update the fragments flow thread portion rect because we are going to process
    // a break on these fragments.
    UpdateFragmentsFragmentedFlowPortionRect();

    // Simulate a fragment break at offsetBreakInFragmentedFlow. If it points inside an auto logical height
    // fragment, then it determines the fragment computed auto height.
    auto *fragment = FragmentAtBlockOffset(block, offsetBreakInFragmentedFlow);
    if (!fragment)
    {
      return false;
    }

    auto portionRect = fragment->FragmentedFlowPortionRect();
    LayoutUnit currentFragmentOffsetInFragmentedFlow =
      IsHorizontalWritingMode() ? portionRect.MaxY() : portionRect.MaxX();

    if (offsetBreakAdjustment)
    {
      *offsetBreakAdjustment =
        std::max<LayoutUnit>(0_lu, currentFragmentOffsetInFragmentedFlow - offsetBreakInFragmentedFlow);
    }

    return false;
  }

  void RenderFragmentedFlow::CollectLayerFragments(LayerFragments &layerFragments,
                                                   const LayoutRect &layerBoundingBox,
                                                   const LayoutRect &dirtyRect) const noexcept
  {
    krys_debug_assert(!_fragmentsInvalidated || IsSkippedContent());

    for (auto *fragment : _fragmentList)
    {
      fragment->CollectLayerFragments(layerFragments, layerBoundingBox, dirtyRect);
    }
  }

  LayoutRect RenderFragmentedFlow::FragmentsBoundingBox(const LayoutRect &layerBoundingBox) const noexcept
  {
    krys_debug_assert(!_fragmentsInvalidated);

    LayoutRect result;
    for (auto *fragment : _fragmentList)
    {
      LayerFragments fragments;
      fragment->CollectLayerFragments(fragments, layerBoundingBox, LayoutRect::InfiniteRect());
      for (const auto &layerFragment : fragments)
      {
        LayoutRect fragmentRect(layerBoundingBox);
        fragmentRect.Intersect(layerFragment.PaginationClip);
        fragmentRect.Move(layerFragment.PaginationOffset);
        result.Unite(fragmentRect);
      }
    }

    return result;
  }

  LayoutUnit
    RenderFragmentedFlow::OffsetFromLogicalTopOfFirstFragment(const RenderBlock *currentBlock) const noexcept
  {
    // As a last resort, take the slow path.
    LayoutRect blockRect(0_lu, 0_lu, currentBlock->Width(), currentBlock->Height());
    while (currentBlock && !currentBlock->IsRenderView() && !currentBlock->IsRenderFragmentedFlow())
    {
      auto *containerBlock = currentBlock->ContainingBlock();
      krys_debug_assert(containerBlock);
      if (!containerBlock)
      {
        return 0_lu;
      }
      LayoutPoint currentBlockLocation = currentBlock->Location();
      if (auto *cell = DynamicDowncast<RenderTableCell>(*currentBlock))
      {
        if (auto *section = cell->Section())
        {
          currentBlockLocation.MoveBy(section->Location());
        }
      }

      if (containerBlock->GetWritingMode().BlockDirection()
          != currentBlock->GetWritingMode().BlockDirection())
      {
        // We have to put the block rect in container coordinates
        // and we have to take into account both the container and current block flipping modes
        if (containerBlock->GetWritingMode().IsBlockFlipped())
        {
          if (containerBlock->IsHorizontalWritingMode())
          {
            blockRect.SetY(currentBlock->Height() - blockRect.MaxY());
          }
          else
          {
            blockRect.SetX(currentBlock->Width() - blockRect.MaxX());
          }
        }
        currentBlock->FlipForWritingMode(blockRect);
      }
      blockRect.MoveBy(currentBlockLocation);
      currentBlock = containerBlock;
    }

    return currentBlock->IsHorizontalWritingMode() ? blockRect.Y() : blockRect.X();
  }

  void RenderFragmentedFlow::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                                 TransformState &transformState,
                                                 EnumSet<MapCoordinatesMode> mode,
                                                 bool *wasFixed) const noexcept
  {
    if (this == ancestorContainer)
    {
      return;
    }

    auto *fragment = MapFromFlowToFragment(transformState);
    if (!fragment)
    {
      return;
    }

    // If the repaint container is null, we have to climb up to the RenderView, otherwise swap
    // it with the fragment's repaint container.
    ancestorContainer = ancestorContainer ? fragment->ContainerForRepaint().Renderer : nullptr;

    if (auto *fragmentFragmentedFlow = fragment->EnclosingFragmentedFlow())
    {
      RenderFragmentContainer *startFragment = nullptr;
      RenderFragmentContainer *endFragment = nullptr;
      if (fragmentFragmentedFlow->GetFragmentRangeForBox(*fragment, startFragment, endFragment))
      {
        CurrentRenderFragmentContainerMaintainer fragmentMaintainer(*startFragment);
        static_cast<const RenderObject *>(fragment)->MapLocalToContainer(ancestorContainer, transformState,
                                                                         mode, wasFixed);
        return;
      }
    }

    static_cast<const RenderObject *>(fragment)->MapLocalToContainer(ancestorContainer, transformState, mode,
                                                                     wasFixed);
  }

  // FIXME: Make this function faster. Walking the render tree is slow, better use a caching mechanism (e.g.
  // |cachedOffsetFromLogicalTopOfFirstFragment|).
  LayoutRect RenderFragmentedFlow::MapFromLocalToFragmentedFlow(const RenderBox *box,
                                                                const LayoutRect &localRect) const noexcept
  {
    LayoutRect boxRect = localRect;

    while (box && box != this)
    {
      auto *containerBlock = box->ContainingBlock();
      krys_debug_assert(containerBlock);
      if (!containerBlock)
      {
        return {};
      }
      LayoutPoint currentBoxLocation = box->Location();

      if (containerBlock->GetWritingMode().BlockDirection() != box->GetWritingMode().BlockDirection())
      {
        box->FlipForWritingMode(boxRect);
      }

      boxRect.MoveBy(currentBoxLocation);
      box = containerBlock;
    }

    return boxRect;
  }

  // FIXME: Make this function faster. Walking the render tree is slow, better use a caching mechanism (e.g.
  // |cachedOffsetFromLogicalTopOfFirstFragment|).
  LayoutRect RenderFragmentedFlow::MapFromFragmentedFlowToLocal(const RenderBox *box,
                                                                const LayoutRect &rect) const noexcept
  {
    LayoutRect localRect = rect;
    if (box == this)
    {
      return localRect;
    }

    auto *containerBlock = box->ContainingBlock();
    krys_debug_assert(containerBlock);
    if (!containerBlock)
    {
      return {};
    }
    localRect = MapFromFragmentedFlowToLocal(containerBlock, localRect);

    LayoutPoint currentBoxLocation = box->Location();
    localRect.MoveBy(-currentBoxLocation);

    if (containerBlock->GetWritingMode().BlockDirection() != box->GetWritingMode().BlockDirection())
    {
      box->FlipForWritingMode(localRect);
    }

    return localRect;
  }

  void RenderFragmentedFlow::FlipForWritingModeLocalCoordinates(LayoutRect &rect) const noexcept
  {
    if (!GetWritingMode().IsBlockFlipped())
    {
      return;
    }

    if (IsHorizontalWritingMode())
    {
      rect.SetY(0_lu - rect.MaxY());
    }
    else
    {
      rect.SetX(0_lu - rect.MaxX());
    }
  }

  void RenderFragmentedFlow::AddFragmentsVisualEffectOverflow(const RenderBox &box) noexcept
  {
    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      LayoutRect borderBox = box.BorderBoxRect();
      borderBox = box.ApplyVisualEffectOverflow(borderBox);
      borderBox = fragment->RectFlowPortionForBox(box, borderBox);

      fragment->AddVisualOverflowForBox(box, borderBox);
      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  void RenderFragmentedFlow::AddFragmentsVisualOverflowFromTheme(const RenderBlock &block) noexcept
  {
    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(block, startFragment, endFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      LayoutRect borderBox = block.BorderBoxRect();
      borderBox = fragment->RectFlowPortionForBox(block, borderBox);

      FloatRect inflatedRect = borderBox;
      RenderTheme::Singleton().AdjustRepaintRect(block, inflatedRect);

      fragment->AddVisualOverflowForBox(block, LayoutRect(SnappedIntRect(LayoutRect(inflatedRect))));
      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  void RenderFragmentedFlow::AddFragmentsOverflowFromChild(const RenderBox &box, const RenderBox &child,
                                                           const LayoutSize &delta) noexcept
  {
    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(child, startFragment, endFragment))
    {
      return;
    }

    RenderFragmentContainer *containerStartFragment = nullptr;
    RenderFragmentContainer *containerEndFragment = nullptr;
    if (!GetFragmentRangeForBox(box, containerStartFragment, containerEndFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      if (FragmentInRange(fragment, containerStartFragment, containerEndFragment))
      {
        LayoutRect childLayoutOverflowRect = fragment->LayoutOverflowRectForBoxForPropagation(child);
        childLayoutOverflowRect.Move(delta);

        fragment->AddLayoutOverflowForBox(box, childLayoutOverflowRect);

        if (!child.HasSelfPaintingLayer() && !box.HasNonVisibleOverflow())
        {
          LayoutRect childVisualOverflowRect = fragment->VisualOverflowRectForBoxForPropagation(child);
          childVisualOverflowRect.Move(delta);
          fragment->AddVisualOverflowForBox(box, childVisualOverflowRect);
        }
      }

      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  void RenderFragmentedFlow::AddFragmentsLayoutOverflow(const RenderBox &box,
                                                        const LayoutRect &layoutOverflow) noexcept
  {
    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      fragment->AddLayoutOverflowForBox(box, fragment->RectFlowPortionForBox(box, layoutOverflow));

      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  void RenderFragmentedFlow::AddFragmentsVisualOverflow(const RenderBox &box,
                                                        const LayoutRect &visualOverflow) noexcept
  {
    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      fragment->AddVisualOverflowForBox(box, fragment->RectFlowPortionForBox(box, visualOverflow));

      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  void RenderFragmentedFlow::ClearFragmentsOverflow(const RenderBox &box) noexcept
  {
    RenderFragmentContainer *startFragment = nullptr;
    RenderFragmentContainer *endFragment = nullptr;
    if (!GetFragmentRangeForBox(box, startFragment, endFragment))
    {
      return;
    }

    for (auto *fragment : FragmentsFrom(*startFragment))
    {
      auto *boxInfo = fragment->GetRenderBoxFragmentInfo(box);
      if (boxInfo && boxInfo->Overflow())
      {
        boxInfo->ClearOverflow();
      }

      if (fragment == endFragment)
      {
        break;
      }
    }
  }

  RenderFragmentContainer *RenderFragmentedFlow::CurrentFragment() const noexcept
  {
    return _currentFragmentMaintainer ? &_currentFragmentMaintainer->Fragment() : nullptr;
  }
}
