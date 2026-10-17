#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  LocalFrameViewLayoutContext::LocalFrameViewLayoutContext(LocalFrameView &frameView) noexcept
      : _frameView(frameView)
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

  void LocalFrameViewLayoutContext::Layout() noexcept
  {
    Ref protectedView = ShareRef(_frameView);

    PerformLayout();

    // FIXME: Invalidate the styles that depend on layout (Scope::invalidateForLayoutDependencies: anchor
    // positioning and the interleaved container query layouts) and lay out again, once there's layout.
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

    _layoutPending = false;
    _setNeedsLayoutWasDeferred = false;

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InPreLayout);

      // FIXME: Skip this in style-interleaved layout and run the remaining post-layout tasks first, once
      // there's layout.
      UpdateStyleForLayout();

      if (!GetRenderView())
      {
        return;
      }

      _firstLayout = false;
    }

    WeakPtr<RenderElement> layoutRoot;
    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InPreLayout);
      layoutRoot = CreateWeakPtr(SubtreeLayoutRoot() ? SubtreeLayoutRoot()
                                                     : static_cast<RenderElement *>(GetRenderView()));
      // FIXME: The full repaint of a first layout, once there's painting.
    }

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InRenderTreeLayout);
      // FIXME: The subtree layout state, and the repaint region accumulator, once there's painting.
      layoutRoot->Layout();
      // FIXME: The layout areas the inspector reports, once there's an inspector.
      ClearSubtreeLayoutRoot();
    }

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InViewSizeAdjust);
      // FIXME: Adjust the view's size to the document's (adjustViewSize), once there are scroll views.
    }

    {
      SetForScope layoutPhase(_layoutPhase, LayoutPhase::InPostLayout);
      krys_debug_assert(!layoutRoot->NeedsLayout());
      // FIXME: Update the layer positions (didLayout) and run the post-layout tasks, once there are layers.
    }
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
    return IsLayoutPending() || (renderView && renderView->NeedsLayout()) || SubtreeLayoutRoot()
           || (_disableSetNeedsLayoutCount && _setNeedsLayoutWasDeferred);
  }

  void LocalFrameViewLayoutContext::SetNeedsLayoutAfterViewConfigurationChange() noexcept
  {
    if (_disableSetNeedsLayoutCount)
    {
      _setNeedsLayoutWasDeferred = true;
      return;
    }

    if (auto *renderView = GetRenderView())
    {
      renderView->SetNeedsLayout();
      ScheduleLayout();
    }
  }

  void LocalFrameViewLayoutContext::EnableSetNeedsLayout() noexcept
  {
    krys_debug_assert(_disableSetNeedsLayoutCount);
    if (!--_disableSetNeedsLayoutCount)
    {
      _setNeedsLayoutWasDeferred = false; // FIXME: Find a way to make the deferred layout actually happen.
    }
  }

  void LocalFrameViewLayoutContext::DisableSetNeedsLayout() noexcept
  {
    ++_disableSetNeedsLayoutCount;
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

    // FIXME: Start the layout timer, once there's an event loop. Until then the host runs the pending layout
    // (Layout or Document::UpdateLayout).
    _layoutPending = true;
  }

  void LocalFrameViewLayoutContext::UnscheduleLayout() noexcept
  {
    _layoutPending = false;
  }

  bool LocalFrameViewLayoutContext::CanPerformLayout() const noexcept
  {
    if (IsInRenderTreeLayout())
    {
      return false;
    }

    // FIXME: Not while the view is painting, once there's painting.

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
      // FIXME: Start the layout timer, once there's an event loop.
      _layoutPending = true;
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
    // FIXME: Apply the pagination of the document element's or <body>'s `overflow` to the view
    // (applyPaginationToViewport), once there are renderers.

    // Always ensure our style info is up-to-date. This can happen in situations where
    // the layout beats any sort of style recalc update that needs to occur.
    document->UpdateStyleIfNeeded();
  }
}
