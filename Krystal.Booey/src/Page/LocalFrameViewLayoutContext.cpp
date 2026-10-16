#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
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

    // FIXME: Lay out the render tree (or the subtree layout root), adjust the view's size and run the
    // post-layout tasks, once there's a render tree.
    GetRenderView()->ClearNeedsLayout();
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
    return IsLayoutPending() || (renderView && renderView->NeedsLayout())
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

    if (!GetRenderView())
    {
      return false;
    }

    return true;
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
