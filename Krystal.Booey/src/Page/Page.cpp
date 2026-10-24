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
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Core/Types/List.hpp"

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
      RenderingUpdateStep::LayerFlush};
  }

  Page::Page(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
             RefPtr<BackForwardClient> &&backForwardClient) noexcept
      : _chromeClient(krys::move(chromeClient)), _editorClient(krys::move(editorClient)),
        _backForwardController(CreateUnique<BackForwardController>(
          *this, backForwardClient ? ShareRef(*backForwardClient)
                                   : Ref<BackForwardClient>(BackForwardList::Create())))
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
                         RefPtr<BackForwardClient> backForwardClient) noexcept
  {
    Ref<Page> page =
      AdoptRef(*new Page(krys::move(chromeClient), krys::move(editorClient), krys::move(backForwardClient)));
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
    // FIXME: Software and accelerated filter rendering, once there's either.
    return FilterRenderingMode::GraphicsContext;
  }

  void Page::ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps) noexcept
  {
    if (!_renderingUpdateDepth)
    {
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
                      [](dom::Document &document) { document.UpdateResizeObservations(); });

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

    // FIXME: Content relevancy, view transitions and intersection observations, once there are any.

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
}
