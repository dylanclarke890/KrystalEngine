/*
 * Copyright (C) 2017 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <algorithm>

namespace krys::boo
{
  LocalFrameViewLayoutContext::LocalFrameViewLayoutContext(LocalFrameView &frameView) noexcept
      : _frameView(frameView), _layoutTimer(*this, &LocalFrameViewLayoutContext::LayoutTimerFired)
  {
  }

  LocalFrameViewLayoutContext::~LocalFrameViewLayoutContext() noexcept = default;

  LocalFrame &LocalFrameViewLayoutContext::Frame() const noexcept
  {
    return _frameView.Frame();
  }

  dom::Document *LocalFrameViewLayoutContext::Document() const noexcept
  {
    return Frame().Document();
  }

  RenderView *LocalFrameViewLayoutContext::GetRenderView() const noexcept
  {
    auto *document = Document();
    return document ? document->GetRenderView() : nullptr;
  }

  const AnchorScrollAdjuster *
    LocalFrameViewLayoutContext::AnchorScrollAdjusterFor(const RenderBox &anchored) const noexcept
  {
    auto it = std::ranges::find_if(_anchorScrollAdjusters,
                                   [&](const auto &item) { return item.Anchored() == &anchored; });
    if (it == _anchorScrollAdjusters.end())
    {
      return {};
    }
    return &*it;
  }

  AnchorScrollAdjuster::Diff
    LocalFrameViewLayoutContext::RegisterAnchorScrollAdjuster(AnchorScrollAdjuster &&scrollAdjuster) noexcept
  {
    auto it = std::ranges::find_if(_anchorScrollAdjusters, [&](const auto &item)
                                   { return item.Anchored() == scrollAdjuster.Anchored(); });

    bool recaptureDiffers = false;
    if (it == _anchorScrollAdjusters.end())
    {
      _anchorScrollAdjusters.push_back(krys::move(scrollAdjuster));
      return AnchorScrollAdjuster::Diff::New;
    }

    recaptureDiffers = it->RecaptureDiffers(scrollAdjuster);
    *it = krys::move(scrollAdjuster);
    return recaptureDiffers ? AnchorScrollAdjuster::Diff::SnapshotsDiffer
                            : AnchorScrollAdjuster::Diff::SnapshotsMatch;
  }

  void LocalFrameViewLayoutContext::UnregisterAnchorScrollAdjusterFor(const RenderBox &anchored) noexcept
  {
    if (auto it = std::ranges::find_if(_anchorScrollAdjusters,
                                       [&](const auto &item) { return item.Anchored() == &anchored; });
        it != _anchorScrollAdjusters.end())
    {
      _anchorScrollAdjusters.erase(it);
    }
    krys_debug_assert(std::ranges::none_of(_anchorScrollAdjusters,
                                           [&](const auto &item) { return item.Anchored() == &anchored; }));

    if (anchored.Layer())
    {
      anchored.Layer()->ClearAnchorScrollAdjustment();
    }
  }

  void
    LocalFrameViewLayoutContext::InvalidateAnchorDependenciesForScroller(const RenderBox &scroller) noexcept
  {
    for (auto &adjuster : _anchorScrollAdjusters)
    {
      adjuster.InvalidateForScroller(scroller);
    }
  }

  void
    LocalFrameViewLayoutContext::RemoveScrollerFromAnchorScrollAdjusters(const RenderBox &scroller) noexcept
  {
    if (!GetRenderView() || GetRenderView()->RenderTreeBeingDestroyed())
    {
      _anchorScrollAdjusters.clear();
    }
    else
    {
      // Unregistering erases from the list, so the boxes to unregister are collected first.
      List<RenderBox *> invalidated;
      for (auto &adjuster : _anchorScrollAdjusters)
      {
        if (adjuster.InvalidateForScroller(scroller))
        {
          invalidated.push_back(adjuster.Anchored());
        }
      }
      for (auto *anchored : invalidated)
      {
        UnregisterAnchorScrollAdjusterFor(*anchored);
      }
    }
  }

  void LocalFrameViewLayoutContext::Layout() noexcept
  {
    Ref protectedView = ShareRef(_frameView);

    PerformLayout();

    if (protectedView->HasOneRef())
    {
      return;
    }

    style::Scope::LayoutDependencyUpdateContext layoutDependencyUpdateContext;
    while (Document()
           && Document()->StyleScope().InvalidateForLayoutDependencies(layoutDependencyUpdateContext))
    {
      Document()->UpdateStyleIfNeeded();

      if (!NeedsLayout())
      {
        break;
      }

      PerformLayout();

      if (protectedView->HasOneRef())
      {
        return;
      }
    }
  }

  void LocalFrameViewLayoutContext::InterleavedLayout() noexcept
  {
    Ref protectedView = ShareRef(_frameView);

    PerformLayout();

    style::Scope::LayoutDependencyUpdateContext layoutDependencyUpdateContext;
    Document()->StyleScope().InvalidateForLayoutDependencies(layoutDependencyUpdateContext);
  }

  void LocalFrameViewLayoutContext::PerformLayout() noexcept
  {
    krys_assert(Document());
    krys_assert(!Document()->InRenderTreeUpdate());
    krys_debug_assert(Frame().View() == &_frameView);
    if (!CanPerformLayout())
    {
      return;
    }

    SetForScope nestedState(_layoutNestedState, _layoutNestedState == LayoutNestedState::NotInLayout
                                                  ? LayoutNestedState::NotNested
                                                  : LayoutNestedState::Nested);
    SetForScope schedulingIsEnabled(_layoutSchedulingIsEnabled, false);

    _layoutTimer.Stop();

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InPreLayout);

      // FIXME: Run the remaining post-layout tasks first, once there are those.
      if (!Document()->IsInStyleInterleavedLayout())
      {
        UpdateStyleForLayout();
      }

      if (!GetRenderView())
      {
        return;
      }
    }

    WeakPtr<RenderElement> layoutRoot;
    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InPreLayout);
      layoutRoot = CreateWeakPtr(SubtreeLayoutRoot() ? SubtreeLayoutRoot()
                                                     : static_cast<RenderElement *>(GetRenderView()));
      _needsFullRepaint = Is<RenderView>(*layoutRoot) && _firstLayout;
      _frameView.WillDoLayout(*layoutRoot);
      _firstLayout = false;
    }

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InRenderTreeLayout);
      SubtreeLayoutStateMaintainer subtreeLayoutStateMaintainer(SubtreeLayoutRoot());
      RenderView::RepaintRegionAccumulator repaintRegionAccumulator(GetRenderView());
      layoutRoot->Layout();
      // FIXME: The layout areas the inspector reports, once there's an inspector.
      ClearSubtreeLayoutRoot();
    }

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InViewSizeAdjust);
      if (Is<RenderView>(*layoutRoot))
      {
        // This is to protect _needsFullRepaint's value when layout() is getting re-entered through
        // adjustViewSize().
        SetForScope needsFullRepaint(_needsFullRepaint);
        _frameView.AdjustViewSize();
      }
    }

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InPostLayout);
      if (_needsFullRepaint)
      {
        GetRenderView()->RepaintRootContents();
      }
      krys_debug_assert(!layoutRoot->NeedsLayout());
      // FIXME: Defer the layer position update (canDeferUpdateLayerPositions), compare the repaint rect
      // environment and run the post-layout tasks, once there are any.
      if (auto *renderView = GetRenderView(); renderView && renderView->Layer())
      {
        renderView->Layer()->UpdateLayerPositionsAfterLayout(NeedsFullRepaint(), false);
      }
      _updateCompositingLayersIsPending = true;
      krys_debug_assert(_percentHeightIgnoreList.empty());
    }

    // The post-layout tasks: the animations layout brought into view resume, the fragment's target is
    // scrolled back into view, and the window is told if the viewport resized.
    _frameView.ViewportContentsChanged();
    _frameView.ScheduleScrollToAnchor();
    _frameView.ScheduleResizeEventIfNeeded();
    _frameView.LayoutOrVisualViewportChanged();
    _frameView.UpdateSnapOffsetsAndResnap();
  }

  bool LocalFrameViewLayoutContext::UpdateCompositingLayersAfterStyleChange() noexcept
  {
    // If we expect to update compositing after an incipient layout, don't do so here.
    auto *view = GetRenderView();
    if (!view)
    {
      return false;
    }

    if (NeedsLayout() || IsInLayout())
    {
      return false;
    }

    view->Layer()->UpdateLayerPositionsAfterStyleChange(false);

    return view->Compositor().DidRecalcStyleWithNoPendingLayout();
  }

  void LocalFrameViewLayoutContext::UpdateCompositingLayersAfterLayout() noexcept
  {
    auto *renderView = GetRenderView();
    if (!renderView)
    {
      return;
    }

    renderView->Compositor().UpdateCompositingLayers(CompositingUpdateType::AfterLayout);
    _updateCompositingLayersIsPending = false;
  }

  bool LocalFrameViewLayoutContext::UpdateCompositingLayersAfterLayoutIfNeeded() noexcept
  {
    if (_updateCompositingLayersIsPending)
    {
      UpdateCompositingLayersAfterLayout();
      return true;
    }

    return false;
  }

  bool LocalFrameViewLayoutContext::NeedsLayout() const noexcept
  {
    return NeedsLayoutInternal();
  }

  bool LocalFrameViewLayoutContext::NeedsLayoutInternal() const noexcept
  {
    // This can return true in cases where the document does not have a body yet.
    // Document::shouldScheduleLayout takes care of preventing us from scheduling
    // layout in that case.
    auto *renderView = GetRenderView();
    return IsLayoutPending() || (renderView && renderView->NeedsLayout()) || SubtreeLayoutRoot();
  }

  void LocalFrameViewLayoutContext::SetNeedsLayoutAfterViewConfigurationChange() noexcept
  {
    if (auto *renderView = GetRenderView())
    {
      renderView->SetNeedsLayout();
      ScheduleLayout();
    }
  }

  void LocalFrameViewLayoutContext::ScheduleLayout() noexcept
  {
    krys_debug_assert(Frame().View() == &_frameView);
    auto *document = Document();
    if (!document)
    {
      return;
    }

    if (SubtreeLayoutRoot())
    {
      ConvertSubtreeLayoutToFullLayout();
    }

    if (IsLayoutPending())
    {
      return;
    }

    if (!IsLayoutSchedulingEnabled() || !document->ShouldScheduleLayout())
    {
      return;
    }
    if (!NeedsLayout())
    {
      return;
    }

    _layoutTimer.StartOneShot({});
    // FIXME: Scheduled by the repaints instead (the layer flush), once there's repaint tracking.
    document->ScheduleRenderingUpdate({});
  }

  void LocalFrameViewLayoutContext::LayoutTimerFired() noexcept
  {
    Layout();
  }

  void LocalFrameViewLayoutContext::UnscheduleLayout() noexcept
  {
    _layoutTimer.Stop();
  }

  bool LocalFrameViewLayoutContext::CanPerformLayout() const noexcept
  {
    if (IsInRenderTreeLayout())
    {
      return false;
    }

    // FIXME: Not while the view is painting.

    if (!SubtreeLayoutRoot() && !GetRenderView())
    {
      return false;
    }

    return true;
  }

  static bool IsObjectAncestorContainerOf(RenderElement &ancestor, RenderElement &descendant) noexcept
  {
    for (auto *renderer = &descendant; renderer; renderer = renderer->Container())
    {
      if (renderer == &ancestor)
      {
        return true;
      }
    }
    return false;
  }

  void LocalFrameViewLayoutContext::ScheduleSubtreeLayout(RenderElement &layoutRoot) noexcept
  {
    krys_debug_assert(GetRenderView());
    auto &renderView = *GetRenderView();

    // Try to catch unnecessary work during render tree teardown.
    krys_debug_assert(!renderView.RenderTreeBeingDestroyed());
    krys_debug_assert(Frame().View() == &_frameView);

    if (renderView.NeedsLayout() && !SubtreeLayoutRoot())
    {
      (void)layoutRoot.MarkContainingBlocksForLayout(&renderView);
      return;
    }

    if (!IsLayoutPending() && IsLayoutSchedulingEnabled())
    {
      krys_debug_assert(!layoutRoot.Container() || Is<RenderView>(layoutRoot.Container())
                        || !layoutRoot.Container()->NeedsLayout());
      SetSubtreeLayoutRoot(layoutRoot);
      _layoutTimer.StartOneShot({});
      if (auto *document = Document())
      {
        document->ScheduleRenderingUpdate({});
      }
      return;
    }

    auto *subtreeLayoutRoot = SubtreeLayoutRoot();
    if (subtreeLayoutRoot == &layoutRoot)
    {
      return;
    }

    if (!subtreeLayoutRoot)
    {
      // We already have a pending (full) layout. Just mark the subtree for layout.
      (void)layoutRoot.MarkContainingBlocksForLayout(&renderView);
      return;
    }

    if (IsObjectAncestorContainerOf(*subtreeLayoutRoot, layoutRoot))
    {
      // Keep the current root.
      (void)layoutRoot.MarkContainingBlocksForLayout(subtreeLayoutRoot);
      krys_debug_assert(!subtreeLayoutRoot->Container() || Is<RenderView>(subtreeLayoutRoot->Container())
                        || !subtreeLayoutRoot->Container()->NeedsLayout());
      return;
    }

    if (IsObjectAncestorContainerOf(layoutRoot, *subtreeLayoutRoot))
    {
      // Re-root at newRelayoutRoot.
      (void)subtreeLayoutRoot->MarkContainingBlocksForLayout(&layoutRoot);
      SetSubtreeLayoutRoot(layoutRoot);
      krys_debug_assert(!layoutRoot.Container() || Is<RenderView>(layoutRoot.Container())
                        || !layoutRoot.Container()->NeedsLayout());
      return;
    }
    // Two disjoint subtrees need layout. Mark both of them and issue a full layout instead.
    ConvertSubtreeLayoutToFullLayout();
    (void)layoutRoot.MarkContainingBlocksForLayout(&renderView);
  }

  RenderElement *LocalFrameViewLayoutContext::SubtreeLayoutRoot() const noexcept
  {
    return _subtreeLayoutRoot.get();
  }

  void LocalFrameViewLayoutContext::ConvertSubtreeLayoutToFullLayout() noexcept
  {
    krys_debug_assert(SubtreeLayoutRoot());
    (void)SubtreeLayoutRoot()->MarkContainingBlocksForLayout(GetRenderView());
    ClearSubtreeLayoutRoot();
  }

  void LocalFrameViewLayoutContext::SetSubtreeLayoutRoot(RenderElement &layoutRoot) noexcept
  {
    _subtreeLayoutRoot = CreateWeakPtr(&layoutRoot);
  }

  void LocalFrameViewLayoutContext::UpdateStyleForLayout() noexcept
  {
    Ref document = ShareRef(*Document());

    if (!document->StyleScope().ResolverIfExists())
    {
      document->StyleScope().DidChangeStyleSheetEnvironment();
    }

    // Viewport-dependent media queries may cause us to need completely different style information.
    document->StyleScope().EvaluateMediaQueriesForViewportChange();

    // FIXME: Update the elements affected by media queries (the active theme colour meta element, and images
    // whose sources depend on media queries), once they're tracked.

    // Always ensure our style info is up-to-date. This can happen in situations where
    // the layout beats any sort of style recalc update that needs to occur.
    document->UpdateStyleIfNeeded();
  }

  void LocalFrameViewLayoutContext::DisablePercentHeightResolveFor(const RenderBox &flexItem) noexcept
  {
    _percentHeightIgnoreList.insert(&flexItem);
  }

  void LocalFrameViewLayoutContext::EnablePercentHeightResolveFor(const RenderBox &flexItem) noexcept
  {
    _percentHeightIgnoreList.erase(&flexItem);
  }

  bool
    LocalFrameViewLayoutContext::IsPercentHeightResolveDisabledFor(const RenderBox &flexItem) const noexcept
  {
    return _percentHeightIgnoreList.contains(&flexItem);
  }

  bool LocalFrameViewLayoutContext::IsSkippedContentForLayout(const RenderElement &renderer) const noexcept
  {
    if (IsVisiblityHiddenIgnored() || IsVisiblityAutoIgnored()
        || (IsRevealedWhenFoundIgnored() && renderer.Style().AutoRevealsWhenFound()))
    {
      // In theory we should only descend into a hidden/auto subree when hidden/auto root is ignored (see
      // IsSkippedContentRootForLayout below).
      return false;
    }
    return renderer.IsSkippedContent();
  }

  bool LocalFrameViewLayoutContext::IsSkippedContentRootForLayout(const RenderBox &renderBox) const noexcept
  {
    if (!IsSkippedContentRoot(renderBox))
    {
      return false;
    }

    auto contentVisibility = renderBox.Style().GetContentVisibility();
    if (contentVisibility == ContentVisibility::Hidden && IsVisiblityHiddenIgnored())
    {
      return false;
    }

    if (contentVisibility == ContentVisibility::Auto && IsVisiblityAutoIgnored())
    {
      return false;
    }

    if (renderBox.Style().AutoRevealsWhenFound() && IsRevealedWhenFoundIgnored())
    {
      return false;
    }

    return true;
  }

  LayoutSize LocalFrameViewLayoutContext::LayoutDelta() const noexcept
  {
    if (auto *layoutState = this->LayoutState())
    {
      return layoutState->LayoutDelta();
    }
    return {};
  }

  void LocalFrameViewLayoutContext::AddLayoutDelta(const LayoutSize &delta) noexcept
  {
    if (auto *layoutState = this->LayoutState())
    {
      layoutState->AddLayoutDelta(delta);
    }
  }

#if KRYS_DEBUG
  bool LocalFrameViewLayoutContext::LayoutDeltaMatches(const LayoutSize &delta) const noexcept
  {
    if (auto *layoutState = this->LayoutState())
    {
      return layoutState->LayoutDeltaMatches(delta);
    }
    return false;
  }
#endif

  RenderLayoutState *LocalFrameViewLayoutContext::LayoutState() const noexcept
  {
    if (_layoutStateStack.empty())
    {
      return nullptr;
    }
    return _layoutStateStack.back().get();
  }

  void LocalFrameViewLayoutContext::PushLayoutState(RenderElement &root) noexcept
  {
    krys_debug_assert(!_paintOffsetCacheDisableCount);
    krys_debug_assert(!LayoutState());

    _layoutStateStack.push_back(CreateUnique<RenderLayoutState>(root));
  }

  bool LocalFrameViewLayoutContext::PushLayoutState(RenderBox &renderer, const LayoutSize &offset,
                                                    LayoutUnit pageHeight, bool pageHeightChanged) noexcept
  {
    // We push LayoutState even if layoutState is disabled because it stores layoutDelta too.
    auto *layoutState = this->LayoutState();
    if (!layoutState || !NeedsFullRepaint() || layoutState->IsPaginated()
        || renderer.EnclosingFragmentedFlow())
    {
      _layoutStateStack.push_back(
        CreateUnique<RenderLayoutState>(_layoutStateStack, renderer, offset, pageHeight, pageHeightChanged,
                                        layoutState ? layoutState->GetLineClamp() : null));
      return true;
    }
    return false;
  }

  void LocalFrameViewLayoutContext::PopLayoutState() noexcept
  {
    if (!LayoutState())
    {
      return;
    }

    _layoutStateStack.pop_back();
  }

#if KRYS_DEBUG
  void LocalFrameViewLayoutContext::CheckLayoutState() const noexcept
  {
    krys_debug_assert(LayoutDeltaMatches(LayoutSize()));
    krys_debug_assert(!_paintOffsetCacheDisableCount);
  }
#endif
}
