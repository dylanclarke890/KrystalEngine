#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/History/BackForwardList.hpp"
#include "Krystal.Booey/History/HistoryItem.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/HistoryController.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/DragController.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr EnumSet<RenderingUpdateStep> AllRenderingUpdateSteps {
      RenderingUpdateStep::RestoreScrollPositionAndViewState,
      RenderingUpdateStep::FlushAutofocusCandidates,
      RenderingUpdateStep::Resize,
      RenderingUpdateStep::Scroll,
      RenderingUpdateStep::MediaQueryEvaluation,
      RenderingUpdateStep::Animations,
      RenderingUpdateStep::AnimationFrameCallbacks,
      RenderingUpdateStep::CaretAnimation,
      RenderingUpdateStep::ResizeObservations,
      RenderingUpdateStep::FocusFixup,
      RenderingUpdateStep::SnapshottedScrollOffsets,
      RenderingUpdateStep::CursorUpdate,
      RenderingUpdateStep::PrepareCanvasesForDisplayOrFlush,
      RenderingUpdateStep::LayerFlush};
  }

  Page::Page(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
             RefPtr<BackForwardClient> &&backForwardClient, UniquePtr<DragClient> dragClient) noexcept
      : _chromeClient(krys::move(chromeClient)), _editorClient(krys::move(editorClient)),
        _backForwardController(CreateUnique<BackForwardController>(
          *this, backForwardClient ? ShareRef(*backForwardClient)
                                   : Ref<BackForwardClient>(BackForwardList::Create()))),
        _pointerCaptureController(CreateUnique<PointerCaptureController>(*this)),
        _pointerLockController(CreateUnique<PointerLockController>(*this)),
        _dragController(CreateUnique<DragController>(
          *this,
          dragClient ? krys::move(dragClient) : UniquePtr<DragClient>(CreateUnique<EmptyDragClient>()))),
        _dragCaretController(CreateUnique<DragCaretController>())
  {
    if (!_chromeClient)
    {
      _chromeClient = CreateUnique<EmptyChromeClient>();
    }
    if (!_editorClient)
    {
      _editorClient = CreateUnique<EmptyEditorClient>();
    }
  }

  void Page::SetPageScaleFactor(float scale, const IntPoint &origin) noexcept
  {
    RefPtr<dom::Document> mainDocument = ShareRefPtr(_mainFrame->Document());
    RefPtr<LocalFrameView> mainFrameView = mainDocument ? ShareRefPtr(mainDocument->View()) : nullptr;

    if (scale == _pageScaleFactor)
    {
      if (mainFrameView && mainFrameView->GetScrollPosition() != origin)
      {
        (void)mainDocument->UpdateLayoutIgnorePendingStylesheets(
          {dom::LayoutOptions::UpdateCompositingLayers});
      }
    }
    else
    {
      _pageScaleFactor = scale;

      // FIXME: Every root frame, once there are subframes.
      if (RefPtr<LocalFrameView> view = ShareRefPtr(_mainFrame->View()))
      {
        view->LayoutContext().SetNeedsLayoutAfterViewConfigurationChange();
        view->SetNeedsCompositingGeometryUpdate();
        view->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();

        if (mainDocument)
        {
          mainDocument->ResolveStyle(dom::Document::ResolveStyleType::Rebuild);
        }

        // Transform change on RenderView doesn't trigger repaint on non-composited contents.
        view->InvalidateRect(IntRect(LayoutRect::InfiniteRect()));

        _mainFrame->DeviceOrPageScaleFactorChanged();
      }

      if (mainFrameView && mainFrameView->GetScrollPosition() != origin && mainDocument->GetRenderView()
          && mainDocument->GetRenderView()->NeedsLayout() && mainFrameView->LayoutContext().DidFirstLayout())
      {
        auto &layoutContext = mainFrameView->LayoutContext();
        layoutContext.Layout();
        (void)layoutContext.UpdateCompositingLayersAfterLayoutIfNeeded();
      }
    }

    if (mainFrameView && mainFrameView->GetScrollPosition() != origin)
    {
      mainFrameView->SetScrollPosition(origin);
    }
  }

  void Page::DidChangeMainDocument(dom::Document *newDocument) noexcept
  {
    (void)newDocument;
    _pointerCaptureController->Reset();
  }

  Page::~Page() noexcept
  {
    BackForwardCache::Singleton().RemoveAllItemsForPage(*this);
    _backForwardController->Close();
  }

  void Page::DestroyRenderTrees() noexcept
  {
    ForEachDocument(
      [](dom::Document &document)
      {
        if (document.HasLivingRenderTree())
        {
          document.DestroyRenderTree();
        }
      });
  }

  Ref<Page> Page::Create(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
                         UniquePtr<LocalFrameLoaderClient> mainFrameLoaderClient,
                         RefPtr<BackForwardClient> backForwardClient,
                         UniquePtr<DragClient> dragClient) noexcept
  {
    Ref<Page> page = AdoptRef(*new Page(krys::move(chromeClient), krys::move(editorClient),
                                        krys::move(backForwardClient), krys::move(dragClient)));
    page->_mainFrame = LocalFrame::Create(page.get(), krys::move(mainFrameLoaderClient));
    return page;
  }

  void Page::GoToItem(LocalFrame &frame, HistoryItem &item, FrameLoadType type) noexcept
  {
    // stopAllLoaders may end up running onload handlers, which could cause further history traversals that
    // may lead to the passed in HistoryItem being deref()-ed. Make sure we can still use it with
    // HistoryController::goToItem later.
    Ref<HistoryItem> protectedItem = ShareRef(item);

    if (frame.Loader().History().ShouldStopLoadingForHistoryItem(item))
    {
      frame.Loader().StopAllLoadersAndCheckCompleteness();
    }
    frame.Loader().History().GoToItem(item, type);
  }

  void Page::GoToItemForNavigationAPI(LocalFrame &frame, HistoryItem &item, FrameLoadType type,
                                      LocalFrame &triggeringFrame,
                                      NavigationAPIMethodTracker *tracker) noexcept
  {
    Ref<HistoryItem> protectedItem = ShareRef(item);

    if (frame.Loader().History().ShouldStopLoadingForHistoryItem(item))
    {
      frame.Loader().StopAllLoadersAndCheckCompleteness();
    }
    frame.Loader().History().GoToItemForNavigationAPI(item, type, triggeringFrame, tracker);
  }

  EnumSet<FilterRenderingMode> Page::PreferredFilterRenderingModes(const GraphicsContext &) const noexcept
  {
    // There's no software rendering: the drawing backend applies filters on the GPU.
    return {FilterRenderingMode::Accelerated, FilterRenderingMode::GraphicsContext};
  }

  void Page::ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps) noexcept
  {
    if (!_renderingUpdateDepth)
    {
      if (_chromeClient->ScheduleRenderingUpdate())
      {
        return;
      }

      // FIXME: The rendering update scheduler (display refresh), once the host reports refreshes: the host
      // runs a scheduled update at its next frame.
      _renderingUpdateIsScheduled = true;
      return;
    }

    // Steps that already ran in this update run in the next one.
    for (auto step : requestedSteps)
    {
      if (!_renderingUpdateRemainingSteps.Contains(step))
      {
        _unfulfilledRequestedSteps.Add(step);
      }
    }
  }

  // https://html.spec.whatwg.org/#update-the-rendering
  void Page::UpdateRendering() noexcept
  {
    Ref protectedThis = ShareRef(*this);

    // This function is not reentrant, e.g. a requestAnimationFrame callback may force a rendering update.
    if (_renderingUpdateDepth)
    {
      LayoutIfNeeded({dom::LayoutOptions::UpdateCompositingLayers});
      return;
    }

    ++_renderingUpdateDepth;
    _unfulfilledRequestedSteps = {};
    _renderingUpdateRemainingSteps = AllRenderingUpdateSteps;
    _lastRenderingUpdateTimestamp = html::EventLoop::Current().Now();
    _renderingUpdateIsScheduled = false;

    LayoutIfNeeded();

    RunProcessingStep(RenderingUpdateStep::RestoreScrollPositionAndViewState,
                      [](dom::Document &document)
                      {
                        if (RefPtr<LocalFrame> frame = ShareRefPtr(document.Frame()))
                        {
                          frame->Loader().RestoreScrollPositionAndViewStateNowIfNeeded();
                        }
                      });

    // FIXME: Revealing documents, once there's visibility.

    ForEachDocument(
      [](dom::Document &document)
      {
        if (auto *window = document.Window())
        {
          window->FreezeNowTimestamp();
        }
      });

    RunProcessingStep(RenderingUpdateStep::FlushAutofocusCandidates, [](dom::Document &document)
                      { html::FocusAlgorithms::FlushAutofocusCandidates(document); });

    RunProcessingStep(RenderingUpdateStep::Resize,
                      [](dom::Document &document) { document.RunResizeSteps(); });

    RunProcessingStep(RenderingUpdateStep::Scroll,
                      [](dom::Document &document) { document.RunScrollSteps(); });

    RunProcessingStep(RenderingUpdateStep::MediaQueryEvaluation,
                      [](dom::Document &document) { document.EvaluateMediaQueriesAndReportChanges(); });

    // FIXME: Fullscreen events, once there's fullscreen.
    RunProcessingStep(RenderingUpdateStep::Animations,
                      [](dom::Document &document) { document.UpdateAnimationsAndSendEvents(); });

    RunProcessingStep(RenderingUpdateStep::AnimationFrameCallbacks,
                      [](dom::Document &document) { document.ServiceRequestAnimationFrameCallbacks(); });

    RunProcessingStep(RenderingUpdateStep::CaretAnimation,
                      [](dom::Document &document) { document.ServiceCaretAnimation(); });

    LayoutIfNeeded();

    ForEachDocument([](dom::Document &document) { document.ProcessTopLayerRemovals(); });

    RunProcessingStep(RenderingUpdateStep::ResizeObservations,
                      [this](dom::Document &document) { document.UpdateResizeObservations(*this); });

    // https://drafts.csswg.org/scroll-animations-1/#event-loop
    ForEachDocument([](dom::Document &document) { document.RunPostRenderingUpdateAnimationTasks(); });

    // https://html.spec.whatwg.org/#focus-fixup-rule
    RunProcessingStep(RenderingUpdateStep::FocusFixup,
                      [](dom::Document &document)
                      {
                        // If the focused area of the document is not a focusable area, run the focusing steps
                        // for the document's viewport.
                        if (RefPtr<dom::Element> focusedElement = ShareRefPtr(document.FocusedElement()))
                        {
                          if (!html::FocusAlgorithms::IsFocusableArea(*focusedElement))
                          {
                            html::FocusAlgorithms::FocusingSteps(document);
                          }
                        }
                      });

    RunProcessingStep(RenderingUpdateStep::UpdateContentRelevancy, [](dom::Document &document)
                      { (void)document.UpdateRelevancyOfContentVisibilityElements(); });

    // FIXME: View transitions, once there are any.

    RunProcessingStep(RenderingUpdateStep::IntersectionObservations,
                      [](dom::Document &document) { document.UpdateIntersectionObservations(); });

    RunProcessingStep(RenderingUpdateStep::SnapshottedScrollOffsets,
                      [](dom::Document &document)
                      {
                        if (auto *renderView = document.GetRenderView())
                        {
                          style::AnchorPositionEvaluator::UpdateScrollAdjustments(*renderView);
                        }
                      });

    LayoutIfNeeded({dom::LayoutOptions::UpdateCompositingLayers});
    DoAfterUpdateRendering();

    RunProcessingStep(RenderingUpdateStep::PrepareCanvasesForDisplayOrFlush,
                      [](dom::Document &document) { document.PrepareCanvasesForDisplayOrFlushIfNeeded(); });

    ForEachDocument(
      [](dom::Document &document)
      {
        if (auto *window = document.Window())
        {
          window->UnfreezeNowTimestamp();
        }
      });

    --_renderingUpdateDepth;

    // There's no drawing area to finalize the update after painting, so the update finalizes itself.
    FinalizeRenderingUpdate();
  }

  Ref<StorageArea> Page::SessionStorageArea(dom::DOMStringView origin) noexcept
  {
    dom::DOMString key(origin);
    auto it = _sessionStorageAreas.find(key);
    if (it != _sessionStorageAreas.end())
    {
      return it->second;
    }
    // Session storage has no quota beyond the local storage's, as upstream.
    Ref<StorageArea> area =
      MemoryStorageArea::Create(StorageType::Session, MemoryStorageArea::DefaultLocalStorageQuota);
    _sessionStorageAreas.emplace(krys::move(key), area);
    return area;
  }

  void Page::NetworkStateChanged(bool isOnLine) noexcept
  {
    List<Ref<dom::Document>> documents;
    ForEachDocument([&](dom::Document &document) { documents.push_back(ShareRef(document)); });

    auto eventName = dom::DOMStringAtom(isOnLine ? EventNames::Online : EventNames::Offline);
    for (auto &document : documents)
    {
      Ref<dom::Event> event = dom::Event::Create(eventName, false, false);
      document->DispatchWindowEvent(*event);
    }
  }

  void Page::OpportunisticallyRunIdleCallbacks(MonotonicTime::duration deadline) noexcept
  {
    if (MonotonicTime::Now() >= deadline)
    {
      return;
    }

    ForEachDocument(
      [deadline](dom::Document &document)
      {
        if (auto *controller = document.GetIdleCallbackController();
            controller && controller->HasPendingIdleCallback())
        {
          controller->StartIdlePeriod(deadline);
        }
      });
  }

  void Page::FinalizeRenderingUpdate() noexcept
  {
    // FIXME: Images with asynchronous decodes, once there are any.
    _renderingUpdateRemainingSteps.Remove(RenderingUpdateStep::LayerFlush);

    if (RefPtr<LocalFrameView> view = ShareRefPtr(_mainFrame->View()))
    {
      view->FlushCompositingStateIncludingSubframes();
    }

    // FIXME: The scrolling tree, once there's asynchronous scrolling.

    RenderingUpdateCompleted();
  }

  void Page::DoAfterUpdateRendering() noexcept
  {
    // Code here should do once-per-frame work that needs to be done before painting, and requires
    // layout to be up-to-date. It should not run script, trigger layout, or dirty layout.
    RunProcessingStep(RenderingUpdateStep::CursorUpdate,
                      [](dom::Document &document)
                      {
                        if (auto *frame = document.Frame())
                        {
                          frame->GetEventHandler().UpdateCursorIfNeeded();
                        }
                      });

    // FIXME: Paint and event timing, once there are any.

    ForEachDocument([](dom::Document &document)
                    { document.Selection().UpdateAppearanceAfterUpdatingRendering(); });

    ForEachRenderableDocument([](dom::Document &document) { document.UpdateHighlightPositions(); });
  }

  void Page::RenderingUpdateCompleted() noexcept
  {
    _renderingUpdateRemainingSteps = {};
    if (!_unfulfilledRequestedSteps.IsEmpty())
    {
      _renderingUpdateIsScheduled = true;
      _unfulfilledRequestedSteps = {};
    }
  }

  void Page::LayoutIfNeeded(EnumSet<dom::LayoutOptions> layoutOptions) noexcept
  {
    // FIXME: Child frames, once there are frames.
    if (RefPtr<dom::Document> document = ShareRefPtr(_mainFrame->Document()); document && _mainFrame->View())
    {
      (void)document->UpdateLayout(layoutOptions);
    }
  }

  void Page::ForEachDocument(const Func<void(dom::Document &)> &functor) const noexcept
  {
    // FIXME: Child frames, once there are frames.
    if (RefPtr<dom::Document> document = ShareRefPtr(_mainFrame->Document()))
    {
      functor(*document);
    }
  }

  void Page::ForEachRenderableDocument(const Func<void(dom::Document &)> &functor) const noexcept
  {
    List<Ref<dom::Document>> documents;
    ForEachDocument([&](dom::Document &document) { documents.push_back(ShareRef(document)); });
    for (auto &document : documents)
    {
      functor(*document);
    }
  }

  void Page::RunProcessingStep(RenderingUpdateStep step,
                               const Func<void(dom::Document &)> &perDocumentFunction) noexcept
  {
    _renderingUpdateRemainingSteps.Remove(step);
    ForEachRenderableDocument(perDocumentFunction);
  }

  KRYS_NODISCARD static const css::CSSOMStringAtom &ScreenAtom() noexcept
  {
    static MainThreadNeverDestroyed<css::CSSOMStringAtom> screen {u8"screen"};
    return screen;
  }

  bool Page::UseDarkAppearance() const noexcept
  {
    auto *view = MainFrame().View();
    if (!view || view->MediaType() != ScreenAtom())
    {
      return false;
    }
    return MainFrame().GetMediaEnvironment().UseDarkAppearance;
  }

  void Page::UpdateStyleAfterChangeInEnvironment() noexcept
  {
    ForEachDocument(
      [](dom::Document &document)
      {
        if (auto *styleResolver = document.StyleScope().ResolverIfExists())
        {
          styleResolver->InvalidateMatchedDeclarationsCache();
        }
        document.ScheduleFullStyleRebuild();
        document.StyleScope().DidChangeStyleSheetEnvironment();
        document.ScheduleRenderingUpdate(RenderingUpdateStep::MediaQueryEvaluation);
      });
  }
}
