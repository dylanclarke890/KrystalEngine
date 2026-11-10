/*
 * Copyright (C) 2012-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
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
 * PROFITS; OR BUSINESS IN..0TERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSet.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSpannerPlaceholder.hpp"

namespace krys::boo
{
  RenderMultiColumnFlow::RenderMultiColumnFlow(dom::Document &document, RenderStyle &&style) noexcept
      : RenderFragmentedFlow(Type::MultiColumnFlow, document, krys::move(style))
  {
    SetFragmentedFlowState(FragmentedFlowState::InsideFlow);
    krys_debug_assert(IsRenderMultiColumnFlow());
  }

  RenderMultiColumnFlow::~RenderMultiColumnFlow() noexcept = default;

  RenderBlockFlow *RenderMultiColumnFlow::MultiColumnBlockFlow() const noexcept
  {
    return Downcast<RenderBlockFlow>(Parent());
  }

  RenderMultiColumnSet *RenderMultiColumnFlow::FirstMultiColumnSet() const noexcept
  {
    for (auto *sibling = NextSibling(); sibling; sibling = sibling->NextSibling())
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*sibling))
      {
        return multiColumnSet;
      }
    }
    return nullptr;
  }

  RenderMultiColumnSet *RenderMultiColumnFlow::LastMultiColumnSet() const noexcept
  {
    krys_debug_assert(MultiColumnBlockFlow());

    for (auto *sibling = MultiColumnBlockFlow()->LastChild(); sibling; sibling = sibling->PreviousSibling())
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*sibling))
      {
        return multiColumnSet;
      }
    }
    return nullptr;
  }

  RenderBox *RenderMultiColumnFlow::FirstColumnSetOrSpanner() const noexcept
  {
    auto *sibling = NextSibling();
    if (!sibling)
    {
      return nullptr;
    }
    krys_debug_assert(Is<RenderMultiColumnSet>(*sibling)
                      || FindColumnSpannerPlaceholder(Downcast<RenderBox>(*sibling)));
    return Downcast<RenderBox>(sibling);
  }

  RenderBox *RenderMultiColumnFlow::NextColumnSetOrSpannerSiblingOf(const RenderBox *child) noexcept
  {
    return child ? child->NextSiblingBox() : nullptr;
  }

  RenderBox *RenderMultiColumnFlow::PreviousColumnSetOrSpannerSiblingOf(const RenderBox *child) noexcept
  {
    if (!child)
    {
      return nullptr;
    }
    if (auto *sibling = child->PreviousSiblingBox(); sibling && !Is<RenderFragmentedFlow>(*sibling))
    {
      return sibling;
    }
    return nullptr;
  }

  RenderMultiColumnSpannerPlaceholder *
    RenderMultiColumnFlow::FindColumnSpannerPlaceholder(const RenderBox &spanner) const noexcept
  {
    auto it = _spannerMap.find(&spanner);
    return it != _spannerMap.end() ? it->second.get() : nullptr;
  }

  void RenderMultiColumnFlow::Layout() noexcept
  {
    krys_debug_assert(!_inLayout);
    _inLayout = true;
    _lastSetWorkedOn = nullptr;
    if (auto *first = FirstColumnSetOrSpanner())
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*first))
      {
        _lastSetWorkedOn = CreateWeakPtr(multiColumnSet);
        multiColumnSet->BeginFlow(this);
      }
    }
    RenderFragmentedFlow::Layout();
    if (auto *lastSet = LastMultiColumnSet())
    {
      if (!NextColumnSetOrSpannerSiblingOf(lastSet))
      {
        lastSet->EndFlow(this, LogicalHeight());
      }
      lastSet->ExpandToEncompassFragmentedFlowContentsIfNeeded();
    }
    _inLayout = false;
    _lastSetWorkedOn = nullptr;
  }

  void RenderMultiColumnFlow::AddFragmentToThread(RenderFragmentContainer *fragmentContainer) noexcept
  {
    auto *columnSet = Downcast<RenderMultiColumnSet>(fragmentContainer);
    auto position = _fragmentList.end();
    if (auto *nextSet = columnSet->NextSiblingMultiColumnSet())
    {
      position = std::find(_fragmentList.begin(), _fragmentList.end(), nextSet);
      krys_debug_assert(position != _fragmentList.end());
    }
    _fragmentList.insert(position, columnSet);
    fragmentContainer->SetIsValid(true);
  }

  void RenderMultiColumnFlow::WillBeRemovedFromTree() noexcept
  {
    // Detach all column sets from the flow thread. Cannot destroy them at this point, since they
    // are siblings of this object, and there may be pointers to this object's sibling somewhere
    // further up on the call stack.
    for (auto *columnSet = FirstMultiColumnSet(); columnSet;
         columnSet = columnSet->NextSiblingMultiColumnSet())
    {
      columnSet->DetachFragment();
    }
    RenderFragmentedFlow::WillBeRemovedFromTree();
  }

  void RenderMultiColumnFlow::FragmentedFlowDescendantBoxLaidOut(RenderBox *descendant) noexcept
  {
    auto *placeholder = DynamicDowncast<RenderMultiColumnSpannerPlaceholder>(*descendant);
    if (!placeholder)
    {
      return;
    }

    auto *container = placeholder->ContainingBlock();

    for (auto *previous = PreviousColumnSetOrSpannerSiblingOf(placeholder->Spanner()); previous;
         previous = PreviousColumnSetOrSpannerSiblingOf(previous))
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*previous))
      {
        multiColumnSet->EndFlow(container, placeholder->LogicalTop());
        break;
      }
    }

    for (auto *next = NextColumnSetOrSpannerSiblingOf(placeholder->Spanner()); next;
         next = NextColumnSetOrSpannerSiblingOf(next))
    {
      if (auto *multiColumnSet = DynamicDowncast<RenderMultiColumnSet>(*next))
      {
        _lastSetWorkedOn = CreateWeakPtr(multiColumnSet);
        multiColumnSet->BeginFlow(container);
        break;
      }
    }
  }

  RenderBox::LogicalExtentComputedValues
    RenderMultiColumnFlow::ComputeLogicalHeight(LayoutUnit logicalHeight,
                                                LayoutUnit logicalTop) const noexcept
  {
    // We simply remain at our intrinsic height.
    return {logicalHeight, logicalTop, ComputedMarginValues()};
  }

  LayoutUnit RenderMultiColumnFlow::InitialLogicalWidth() const noexcept
  {
    return ColumnWidth();
  }

  void RenderMultiColumnFlow::SetPageBreak(const RenderBlock *block, LayoutUnit offset,
                                           LayoutUnit spaceShortage) noexcept
  {
    // Only positive values are interesting (and allowed) here. Zero space shortage may be reported
    // when we're at the top of a column and the element has zero height. Ignore this, and also
    // ignore any negative values, which may occur when we set an early break in order to honor
    // widows in the next column.
    if (spaceShortage <= 0)
    {
      return;
    }

    if (auto *multicolSet = Downcast<RenderMultiColumnSet>(FragmentAtBlockOffset(block, offset)))
    {
      multicolSet->RecordSpaceShortage(spaceShortage);
    }
  }

  void RenderMultiColumnFlow::UpdateMinimumPageHeight(const RenderBlock *block, LayoutUnit offset,
                                                      LayoutUnit minHeight) noexcept
  {
    if (!HasValidFragmentInfo())
    {
      return;
    }

    if (auto *multicolSet = Downcast<RenderMultiColumnSet>(FragmentAtBlockOffset(block, offset)))
    {
      multicolSet->UpdateMinimumColumnHeight(minHeight);
    }
  }

  void RenderMultiColumnFlow::UpdateSpaceShortageForSizeContainment(const RenderBlock *block,
                                                                    LayoutUnit offset,
                                                                    LayoutUnit shortage) noexcept
  {
    if (auto *multicolSet = Downcast<RenderMultiColumnSet>(FragmentAtBlockOffset(block, offset)))
    {
      multicolSet->UpdateSpaceShortageForSizeContainment(shortage);
    }
  }

  RenderFragmentContainer *
    RenderMultiColumnFlow::FragmentAtBlockOffset(const RenderBox *box, LayoutUnit offset,
                                                 bool extendLastFragment) const noexcept
  {
    if (!_inLayout)
    {
      return RenderFragmentedFlow::FragmentAtBlockOffset(box, offset, extendLastFragment);
    }

    // Layout in progress. We are calculating the set heights as we speak, so the fragment range
    // information is not up-to-date.

    if (_lastSetWorkedOn && _lastSetWorkedOn->FragmentedFlow() != this)
    {
      _lastSetWorkedOn = nullptr;
    }

    auto *columnSet = _lastSetWorkedOn ? _lastSetWorkedOn.get() : FirstMultiColumnSet();
    if (!columnSet)
    {
      // If there's no set, bail. This multicol is empty or only consists of spanners. There
      // are no fragments.
      return nullptr;
    }
    // The last set worked on is a good guess. But if we're not within the bounds, search for the
    // right one.
    if (offset < columnSet->LogicalTopInFragmentedFlow())
    {
      while (auto *previous = columnSet->PreviousSiblingMultiColumnSet())
      {
        columnSet = previous;
        if (offset >= columnSet->LogicalTopInFragmentedFlow())
        {
          break;
        }
      }
    }
    else
    {
      while (offset >= columnSet->LogicalBottomInFragmentedFlow())
      {
        auto *next = columnSet->NextSiblingMultiColumnSet();
        if (!next || !next->HasBeenFlowed())
        {
          break;
        }
        columnSet = next;
      }
    }
    return columnSet;
  }

  void RenderMultiColumnFlow::SetFragmentRangeForBox(const RenderBox &box,
                                                     RenderFragmentContainer *startFragment,
                                                     RenderFragmentContainer *endFragment) noexcept
  {
    // Some column sets may have zero height, which means that two or more sets may start at the
    // exact same flow thread position, which means that some parts of the code may believe that a
    // given box lives in sets that it doesn't really live in. Make some adjustments here and
    // include such sets if they are adjacent to the start and/or end fragments.
    for (auto *columnSet = Downcast<RenderMultiColumnSet>(*startFragment).PreviousSiblingMultiColumnSet();
         columnSet; columnSet = columnSet->PreviousSiblingMultiColumnSet())
    {
      if (columnSet->LogicalHeightInFragmentedFlow())
      {
        break;
      }
      startFragment = columnSet;
    }
    for (auto *columnSet = Downcast<RenderMultiColumnSet>(*startFragment).NextSiblingMultiColumnSet();
         columnSet; columnSet = columnSet->NextSiblingMultiColumnSet())
    {
      if (columnSet->LogicalHeightInFragmentedFlow())
      {
        break;
      }
      endFragment = columnSet;
    }

    RenderFragmentedFlow::SetFragmentRangeForBox(box, startFragment, endFragment);
  }

  bool RenderMultiColumnFlow::AddForcedFragmentBreak(const RenderBlock *block, LayoutUnit offset,
                                                     RenderBox * /*breakChild*/, bool /*isBefore*/,
                                                     LayoutUnit *offsetBreakAdjustment) noexcept
  {
    auto *multicolSet = Downcast<RenderMultiColumnSet>(FragmentAtBlockOffset(block, offset));
    if (!multicolSet)
    {
      return false;
    }
    multicolSet->AddForcedBreak(offset);
    if (offsetBreakAdjustment)
    {
      *offsetBreakAdjustment =
        PageLogicalHeightForOffset(offset)
          ? PageRemainingLogicalHeightForOffset(offset, PageBoundaryRule::IncludePageBoundary)
          : 0_lu;
    }
    return true;
  }

  LayoutSize RenderMultiColumnFlow::OffsetFromContainer(const RenderElement &enclosingContainer,
                                                        const LayoutPoint &physicalPoint,
                                                        bool *offsetDependsOnPoint) const noexcept
  {
    krys_debug_assert(&enclosingContainer == Container());

    if (offsetDependsOnPoint)
    {
      *offsetDependsOnPoint = true;
    }

    LayoutPoint translatedPhysicalPoint(physicalPoint);
    if (auto *fragment = PhysicalTranslationFromFlowToFragment(translatedPhysicalPoint))
    {
      translatedPhysicalPoint.MoveBy(fragment->TopLeftLocation());
    }

    LayoutSize offset(translatedPhysicalPoint.X(), translatedPhysicalPoint.Y());
    if (auto *enclosingBox = DynamicDowncast<RenderBox>(enclosingContainer))
    {
      offset -= ToLayoutSize(LayoutPoint(enclosingBox->GetScrollPosition()));
    }
    return offset;
  }

  void RenderMultiColumnFlow::MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                                      TransformState &transformState) const noexcept
  {
    // First get the transform state's point into the block flow thread's physical coordinate space.
    Parent()->MapAbsoluteToLocalPoint(mode, transformState);
    LayoutPoint transformPoint(transformState.MappedPoint());

    // Now walk through each fragment.
    const RenderMultiColumnSet *candidateColumnSet = nullptr;
    LayoutPoint candidatePoint;
    LayoutSize candidateContainerOffset;

    for (const auto &columnSet : ChildrenOfType<RenderMultiColumnSet>(*Parent()))
    {
      candidateContainerOffset = columnSet.OffsetFromContainer(*Parent(), LayoutPoint());

      candidatePoint = transformPoint - candidateContainerOffset;
      candidateColumnSet = &columnSet;

      // We really have no clue what to do with overflow. We'll just use the closest fragment to the point in
      // that case.
      LayoutUnit pointOffset = IsHorizontalWritingMode() ? candidatePoint.Y() : candidatePoint.X();
      LayoutUnit fragmentOffset =
        IsHorizontalWritingMode() ? columnSet.TopLeftLocation().Y() : columnSet.TopLeftLocation().X();
      if (pointOffset < fragmentOffset + columnSet.LogicalHeight())
      {
        break;
      }
    }

    if (!candidateColumnSet)
    {
      return;
    }

    // Once we have a good guess as to which fragment we hit tested through (and yes, this was just a
    // heuristic, but it's the best we could do), then we can map from the fragment into the flow thread.
    LayoutSize translationOffset =
      PhysicalTranslationFromFragmentToFlow(candidateColumnSet, candidatePoint) + candidateContainerOffset;
    PushOntoTransformState(transformState, mode, nullptr, Parent(), translationOffset, false);
  }

  LayoutSize RenderMultiColumnFlow::PhysicalTranslationFromFragmentToFlow(
    const RenderMultiColumnSet *columnSet, const LayoutPoint &physicalPoint) const noexcept
  {
    LayoutPoint logicalPoint = columnSet->FlipForWritingMode(physicalPoint);
    LayoutPoint translatedPoint = columnSet->TranslateFragmentPointToFragmentedFlow(logicalPoint);
    LayoutPoint physicalTranslatedPoint = columnSet->FlipForWritingMode(translatedPoint);
    return physicalPoint - physicalTranslatedPoint;
  }

  RenderFragmentContainer *
    RenderMultiColumnFlow::MapFromFlowToFragment(TransformState &transformState) const noexcept
  {
    if (!HasValidFragmentInfo())
    {
      return nullptr;
    }

    // Get back into our local flow thread space.
    LayoutRect boxRect = transformState.MappedQuad().EnclosingBoundingBox();
    FlipForWritingMode(boxRect);

    // FIXME: We need to refactor RenderObject::AbsoluteQuads to be able to split the quads across fragments,
    // for now we just take the center of the mapped enclosing box and map it to a column.
    LayoutPoint centerPoint = boxRect.Center();
    LayoutUnit centerLogicalOffset = IsHorizontalWritingMode() ? centerPoint.Y() : centerPoint.X();
    auto *fragmentContainer = FragmentAtBlockOffset(this, centerLogicalOffset, true);
    if (!fragmentContainer)
    {
      return nullptr;
    }
    transformState.Move(PhysicalTranslationOffsetFromFlowToFragment(fragmentContainer, centerLogicalOffset));
    return fragmentContainer;
  }

  LayoutSize RenderMultiColumnFlow::PhysicalTranslationOffsetFromFlowToFragment(
    const RenderFragmentContainer *fragmentContainer, LayoutUnit logicalOffset) const noexcept
  {
    // Now that we know which multicolumn set we hit, we need to get the appropriate translation offset for
    // the column.
    const auto *columnSet = Downcast<RenderMultiColumnSet>(fragmentContainer);
    LayoutPoint translationOffset = columnSet->ColumnTranslationForOffset(logicalOffset);

    // Now we know how we want the rect to be translated into the fragment. At this point we're converting
    // back to physical coordinates.
    if (GetWritingMode().IsBlockFlipped())
    {
      LayoutRect portionRect(columnSet->FragmentedFlowPortionRect());
      LayoutRect columnRect = columnSet->ColumnRectAt(0);
      LayoutUnit physicalDeltaFromPortionBottom =
        LogicalHeight() - columnSet->LogicalBottomInFragmentedFlow();
      if (IsHorizontalWritingMode())
      {
        columnRect.SetHeight(portionRect.Height());
      }
      else
      {
        columnRect.SetWidth(portionRect.Width());
      }
      columnSet->FlipForWritingMode(columnRect);
      if (IsHorizontalWritingMode())
      {
        translationOffset.Move(0_lu, columnRect.Y() - portionRect.Y() - physicalDeltaFromPortionBottom);
      }
      else
      {
        translationOffset.Move(columnRect.X() - portionRect.X() - physicalDeltaFromPortionBottom, 0_lu);
      }
    }

    return LayoutSize(translationOffset.X(), translationOffset.Y());
  }

  RenderFragmentContainer *
    RenderMultiColumnFlow::PhysicalTranslationFromFlowToFragment(LayoutPoint &physicalPoint) const noexcept
  {
    if (!HasValidFragmentInfo())
    {
      return nullptr;
    }

    // Put the physical point into the flow thread's coordinate space.
    LayoutPoint logicalPoint = FlipForWritingMode(physicalPoint);

    // Now get the fragment that we are in.
    LayoutUnit logicalOffset = IsHorizontalWritingMode() ? logicalPoint.Y() : logicalPoint.X();
    auto *fragmentContainer = FragmentAtBlockOffset(this, logicalOffset, true);
    if (!fragmentContainer)
    {
      return nullptr;
    }

    // Shift the physical point into the fragment's coordinate space.
    physicalPoint += PhysicalTranslationOffsetFromFlowToFragment(fragmentContainer, logicalOffset);

    return fragmentContainer;
  }

  bool RenderMultiColumnFlow::IsPageLogicalHeightKnown() const noexcept
  {
    if (auto *columnSet = LastMultiColumnSet())
    {
      return columnSet->ColumnHeightComputed();
    }
    return false;
  }

  bool RenderMultiColumnFlow::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                          const HitTestLocation &locationInContainer,
                                          const LayoutPoint &accumulatedOffset,
                                          HitTestAction hitTestAction) noexcept
  {
    // You cannot be inside an in-flow RenderFragmentedFlow without a corresponding DOM node. It's better to
    // just let the ancestor figure out where we are instead.
    if (hitTestAction == HitTestBlockBackground)
    {
      return false;
    }
    bool inside = RenderFragmentedFlow::NodeAtPoint(request, result, locationInContainer, accumulatedOffset,
                                                    hitTestAction);
    return inside && result.InnerNode();
  }
}
