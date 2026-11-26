#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.Web/Animation/Timelines/AnimationTimelinesController.hpp"
#include "Krystal.Web/Css/Queries/MediaQuery.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Editing/Editing.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/Algorithms/RenderBlocking.hpp"
#include "Krystal.Web/Html/DragAndDrop/DragController.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Input/Internals/EventHandler.hpp"
#include "Krystal.Web/Input/Internals/ScrollLatchingController.hpp"
#include "Krystal.Web/Input/Internals/WheelEventDeltaFilter.hpp"
#include "Krystal.Web/Input/PointerEvents/PointerCaptureController.hpp"
#include "Krystal.Web/Input/PointerLock/PointerLockController.hpp"
#include "Krystal.Web/Loader/Cache/CachedImage.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Web/Loader/FrameLoader.hpp"
#include "Krystal.Web/Page/Clients/WebClient.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/PopupOpeningObserver.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestSource.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayerCompositor.hpp"
#include "Krystal.Web/Rendering/RenderView.hpp"
#include "Krystal.Web/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include "Krystal.Web/Svg/Graphics/SVGImage.hpp"
#include "Krystal.Web/Svg/SVGDocumentExtensions.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief The frames alive.
    KRYS_NODISCARD List<Frame *> &AllFrames() noexcept
    {
      static MainThreadNeverDestroyed<List<Frame *>> frames;
      return frames;
    }

    /// @brief The SVG images @p document has loaded, each with its own frame.
    KRYS_NODISCARD List<Ref<SVGImage>> SVGImagesOf(Document &document) noexcept
    {
      List<Ref<SVGImage>> images;
      for (const auto &[url, resource] : document.GetCachedResourceLoader().AllCachedResources())
      {
        auto *cachedImage = DynamicDowncast<CachedImage>(resource.get());
        if (auto *image = cachedImage ? DynamicDowncast<SVGImage>(cachedImage->GetImage()) : nullptr)
        {
          images.push_back(ShareRef(*image));
        }
      }
      return images;
    }

    constexpr EnumSet<RenderingUpdateStep> AllRenderingUpdateSteps {
      RenderingUpdateStep::FlushAutofocusCandidates,
      RenderingUpdateStep::Resize,
      RenderingUpdateStep::Scroll,
      RenderingUpdateStep::MediaQueryEvaluation,
      RenderingUpdateStep::Animations,
      RenderingUpdateStep::AnimationFrameCallbacks,
      RenderingUpdateStep::CaretAnimation,
      RenderingUpdateStep::Images,
      RenderingUpdateStep::ResizeObservations,
      RenderingUpdateStep::FocusFixup,
      RenderingUpdateStep::SnapshottedScrollOffsets,
      RenderingUpdateStep::CursorUpdate,
      RenderingUpdateStep::PrepareCanvasesForDisplayOrFlush,
      RenderingUpdateStep::LayerFlush};
  }

  Frame::Frame(UniquePtr<WebClient> webClient, UniquePtr<EditorClient> editorClient,
               UniquePtr<FrameLoaderClient> loaderClient, UniquePtr<DragClient> dragClient) noexcept
      : _webClient(krys::move(webClient)), _editorClient(krys::move(editorClient)),
        _pointerCaptureController(CreateUnique<PointerCaptureController>(*this)),
        _pointerLockController(CreateUnique<PointerLockController>(*this)),
        _scrollLatchingController(CreateUnique<ScrollLatchingController>()),
        _wheelEventDeltaFilter(CreateUnique<WheelEventDeltaFilter>()),
        _dragController(CreateUnique<DragController>(
          *this,
          dragClient ? krys::move(dragClient) : UniquePtr<DragClient>(CreateUnique<EmptyDragClient>()))),
        _dragCaretController(CreateUnique<DragCaretController>()),
        _eventHandler(CreateUnique<EventHandler>(*this)),
        _loader(CreateUnique<FrameLoader>(*this, krys::move(loaderClient)))
  {
    if (!_webClient)
    {
      _webClient = CreateUnique<EmptyWebClient>();
    }
    if (!_editorClient)
    {
      _editorClient = CreateUnique<EmptyEditorClient>();
    }
    AllFrames().push_back(this);
  }

  Frame::~Frame() noexcept
  {
    std::erase(AllFrames(), this);
    if (_document)
    {
      _document->WillBeRemovedFromFrame();
    }
  }

  Ref<Frame> Frame::Create(UniquePtr<WebClient> webClient, UniquePtr<EditorClient> editorClient,
                           UniquePtr<FrameLoaderClient> loaderClient,
                           UniquePtr<DragClient> dragClient) noexcept
  {
    return AdoptRef(*new Frame(krys::move(webClient), krys::move(editorClient), krys::move(loaderClient),
                               krys::move(dragClient)));
  }

  void Frame::UpdateStyleForAllFramesAfterGlobalChangeInEnvironment() noexcept
  {
    ForEachFrame([](Frame &frame) { frame.UpdateStyleAfterChangeInEnvironment(); });
  }

  void Frame::ForEachFrame(const Func<void(Frame &)> &function) noexcept
  {
    // A copy, so the function can make and destroy frames.
    for (auto *frame : List<Frame *>(AllFrames()))
    {
      function(*frame);
    }
  }

  web::Window *Frame::Window() const noexcept
  {
    return _document ? _document->Window() : nullptr;
  }

  void Frame::SetView(RefPtr<FrameView> &&view) noexcept
  {
    if (_view)
    {
      _view->LayoutContext().UnscheduleLayout();
    }

    // The document's render tree belongs to the view it's laid out in.
    if (_document && _document->HasLivingRenderTree())
    {
      _document->DestroyRenderTree();
    }

    _view = krys::move(view);

    if (_document && _view)
    {
      _document->CreateRenderTree();
    }
  }

  void Frame::CreateView(const SizeI &viewportSize, const SizeI &fixedLayoutSize,
                         bool useFixedLayout) noexcept
  {
    SetView(nullptr);

    Ref frameView = FrameView::Create(*this, viewportSize);
    frameView->SetFixedLayoutSize(fixedLayoutSize);
    frameView->SetUseFixedLayout(useFixedLayout);

    SetView(krys::move(frameView));
  }

  VisiblePosition Frame::VisiblePositionForPoint(const PointI &framePoint) const noexcept
  {
    HitTestResult result = GetEventHandler().HitTestResultAtPoint(
      LayoutPoint(framePoint), {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                                HitTestRequest::Type::AllowVisibleChildFrameContentOnly});
    RefPtr<Node> node = ShareRefPtr(result.InnerNonSharedNode());
    if (!node)
    {
      return {};
    }
    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return {};
    }
    auto visiblePos = renderer->VisiblePositionForPoint(result.LocalPoint(), HitTestSource::User);
    if (visiblePos.IsNull())
    {
      visiblePos = VisiblePosition(FirstPositionInOrBeforeNode(node.get()));
    }
    return visiblePos;
  }

  web::Document *Frame::DocumentAtPoint(const PointI &point) const noexcept
  {
    if (!View())
    {
      return nullptr;
    }

    PointI pt = View()->WindowToContents(point);
    HitTestResult result = HitTestResult(LayoutPoint(pt));

    if (Document() && Document()->GetRenderView())
    {
      result = GetEventHandler().HitTestResultAtPoint(
        LayoutPoint(pt),
        {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
         HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent});
    }
    return result.InnerNode() ? &result.InnerNode()->NodeDocument() : nullptr;
  }

  void Frame::SetDocument(const RefPtr<web::Document> &newDocument) noexcept
  {
    krys_assert_msg(!newDocument || !newDocument->Frame(), "a document is in one frame at most");

    if (_documentIsBeingReplaced)
    {
      return;
    }

    SetForScope documentIsBeingReplaced(_documentIsBeingReplaced, true);

    _pointerCaptureController->Reset();

    // The mouse's state refers to the old document's nodes.
    _eventHandler->Clear();

    if (RefPtr<web::Document> previousDocument = _document)
    {
      previousDocument->WillBeRemovedFromFrame();
    }

    _document = newDocument;

    // newDocument rather than _document, which what this runs could replace, and which alone wouldn't keep
    // the document alive.
    if (newDocument)
    {
      newDocument->DidBecomeCurrentDocumentInFrame(*this);
    }
  }

  void Frame::DeviceOrPageScaleFactorChanged() noexcept
  {
    if (auto *root = _document ? _document->GetRenderView() : nullptr)
    {
      root->Compositor().DeviceOrPageScaleFactorChanged();
    }
  }

  void Frame::SetPageZoomFactor(float factor) noexcept
  {
    SetPageAndTextZoomFactors(factor, _textZoomFactor);
  }

  void Frame::SetTextZoomFactor(float factor) noexcept
  {
    SetPageAndTextZoomFactors(_pageZoomFactor, factor);
  }

  void Frame::SetPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor) noexcept
  {
    if (_pageZoomFactor == pageZoomFactor && _textZoomFactor == textZoomFactor)
    {
      return;
    }

    _pageZoomFactor = pageZoomFactor;
    _textZoomFactor = textZoomFactor;

    RefPtr<web::Document> document = _document;
    if (!document)
    {
      return;
    }

    document->ResolveStyle(web::Document::ResolveStyleType::Rebuild);

    if (RefPtr<FrameView> view = _view)
    {
      auto &layoutContext = view->LayoutContext();
      if (document->GetRenderView() && layoutContext.NeedsLayout() && layoutContext.DidFirstLayout())
      {
        layoutContext.Layout();
      }
    }
  }

  void Frame::SetPageScaleFactor(float scale, const PointI &origin) noexcept
  {
    RefPtr<web::Document> document = _document;
    RefPtr<FrameView> frameView = document ? ShareRefPtr(document->View()) : nullptr;

    if (scale == _pageScaleFactor)
    {
      if (frameView && frameView->GetScrollPosition() != origin)
      {
        (void)document->UpdateLayoutIgnorePendingStylesheets(
          {layout::LayoutOptions::UpdateCompositingLayers});
      }
    }
    else
    {
      _pageScaleFactor = scale;

      if (RefPtr<FrameView> view = _view)
      {
        view->LayoutContext().SetNeedsLayoutAfterViewConfigurationChange();
        view->SetNeedsCompositingGeometryUpdate();
        view->SetDescendantsNeedUpdateBackingAndHierarchyTraversal();

        if (document)
        {
          document->ResolveStyle(web::Document::ResolveStyleType::Rebuild);
        }

        // The render view's new transform doesn't repaint what isn't composited by itself.
        view->InvalidateRect(RectI(LayoutRect::InfiniteRect()));

        DeviceOrPageScaleFactorChanged();
      }

      if (frameView && frameView->GetScrollPosition() != origin && document->GetRenderView()
          && document->GetRenderView()->NeedsLayout() && frameView->LayoutContext().DidFirstLayout())
      {
        auto &layoutContext = frameView->LayoutContext();
        layoutContext.Layout();
        (void)layoutContext.UpdateCompositingLayersAfterLayoutIfNeeded();
      }
    }

    if (frameView && frameView->GetScrollPosition() != origin)
    {
      frameView->SetScrollPosition(origin);
    }
  }

  EnumSet<FilterRenderingMode> Frame::PreferredFilterRenderingModes(const GraphicsContext &) noexcept
  {
    // There's no software rendering: the drawing backend applies filters on the GPU.
    return {FilterRenderingMode::Accelerated, FilterRenderingMode::GraphicsContext};
  }

  void Frame::ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps) noexcept
  {
    if (!_renderingUpdateDepth)
    {
      if (_webClient->ScheduleRenderingUpdate())
      {
        return;
      }

      // The host's frames are the frame's rendering opportunities: it runs a scheduled update at its next
      // one.
      _renderingUpdateIsScheduled = true;
      return;
    }

    // Steps that already ran in this update run in the next one.
    for (auto step : requestedSteps)
    {
      if (!_renderingUpdateRemainingSteps.contains(step))
      {
        _unfulfilledRequestedSteps.Add(step);
      }
    }
  }

  void Frame::SetImageAnimationEnabled(bool enabled) noexcept
  {
    if (_imageAnimationEnabled == enabled)
    {
      return;
    }
    _imageAnimationEnabled = enabled;

    ForEachDocument(
      [enabled](web::Document &document)
      {
        if (auto *svgExtensions = document.SVGExtensionsIfExists())
        {
          if (enabled)
          {
            svgExtensions->UnpauseAnimations();
          }
          else
          {
            svgExtensions->PauseAnimations();
          }
        }

        // An SVG image's animations are its frame's SMIL animations.
        for (auto &image : SVGImagesOf(document))
        {
          if (enabled)
          {
            image->ResumeAnimation();
          }
          else
          {
            image->StopAnimation();
          }
        }

        // Animated images stop at their next frame, and start again as they're drawn.
        if (auto *renderView = enabled ? document.GetRenderView() : nullptr)
        {
          renderView->RepaintRootContents();
        }
      });
  }

  void Frame::SetDisplayNominalFramesPerSecond(uint32 framesPerSecond) noexcept
  {
    krys_debug_assert(framesPerSecond);
    _displayNominalFramesPerSecond = std::max(framesPerSecond, 1u);
  }

  MonotonicTime::duration Frame::PreferredRenderingUpdateInterval() const noexcept
  {
    return std::chrono::duration_cast<MonotonicTime::duration>(
      std::chrono::duration<double>(1.0 / _displayNominalFramesPerSecond));
  }

  // https://html.spec.whatwg.org/multipage/webappapis.html#update-the-rendering
  void Frame::UpdateRendering() noexcept
  {
    Ref protectedThis = ShareRef(*this);

    // An update asked for during one (from an animation frame callback, say) only brings layout up to date.
    if (_renderingUpdateDepth)
    {
      LayoutIfNeeded({layout::LayoutOptions::UpdateCompositingLayers});
      return;
    }

    // A hidden frame's document isn't rendered: the update stays scheduled until the frame is shown.
    if (!_isVisible)
    {
      return;
    }

    ++_renderingUpdateDepth;
    _unfulfilledRequestedSteps = {};
    _renderingUpdateRemainingSteps = AllRenderingUpdateSteps;
    _lastRenderingUpdateTimestamp = html::EventLoop::Current().Now();
    _renderingUpdateIsScheduled = false;
    _skippedRenderBlockedDocument = false;

    LayoutIfNeeded();

    // Revealing documents is skipped: it only fires pagereveal and starts view transitions, which there
    // aren't (see DIFFERENCES.md).

    ForEachDocument(
      [](web::Document &document)
      {
        if (auto *window = document.Window())
        {
          window->FreezeNowTimestamp();
        }
      });

    RunProcessingStep(RenderingUpdateStep::FlushAutofocusCandidates, [](web::Document &document)
                      { html::FocusAlgorithms::FlushAutofocusCandidates(document); });

    RunProcessingStep(RenderingUpdateStep::Resize,
                      [](web::Document &document) { document.RunResizeSteps(); });

    RunProcessingStep(RenderingUpdateStep::Scroll,
                      [](web::Document &document) { document.RunScrollSteps(); });

    RunProcessingStep(RenderingUpdateStep::MediaQueryEvaluation,
                      [](web::Document &document) { document.EvaluateMediaQueriesAndReportChanges(); });

    // FIXME: Fullscreen events, once there's fullscreen.
    RunProcessingStep(RenderingUpdateStep::Animations,
                      [](web::Document &document) { document.UpdateAnimationsAndSendEvents(); });

    RunProcessingStep(RenderingUpdateStep::AnimationFrameCallbacks,
                      [](web::Document &document) { document.RunAnimationFrameCallbacks(); });

    RunProcessingStep(RenderingUpdateStep::CaretAnimation,
                      [](web::Document &document) { document.ServiceCaretAnimation(); });

    // An SVG image's frame is updated with the frame that shows it, which its updates are scheduled with.
    RunProcessingStep(RenderingUpdateStep::Images,
                      [](web::Document &document)
                      {
                        for (auto &image : SVGImagesOf(document))
                        {
                          if (RefPtr<Frame> frame = ShareRefPtr(image->InternalFrame()))
                          {
                            frame->UpdateRendering();
                          }
                        }
                      });

    LayoutIfNeeded();

    ForEachDocument([](web::Document &document) { document.ProcessTopLayerRemovals(); });

    RunProcessingStep(RenderingUpdateStep::ResizeObservations,
                      [this](web::Document &document) { document.UpdateResizeObservations(*this); });

    // https://drafts.csswg.org/scroll-animations-1/#event-loop
    ForEachDocument([](web::Document &document) { document.RunPostRenderingUpdateAnimationTasks(); });

    // https://html.spec.whatwg.org/multipage/webappapis.html#focus-fixup-rule
    RunProcessingStep(RenderingUpdateStep::FocusFixup,
                      [](web::Document &document)
                      {
                        // If the focused area of the document is not a focusable area, run the focusing steps
                        // for the document's viewport.
                        // (There's no navigation API, whose focus changed during ongoing navigation would be
                        // set to false.)
                        if (RefPtr<Element> focusedElement = ShareRefPtr(document.FocusedElement()))
                        {
                          if (!html::FocusAlgorithms::IsFocusableArea(*focusedElement))
                          {
                            html::FocusAlgorithms::FocusingSteps(document);
                          }
                        }
                      });

    RunProcessingStep(RenderingUpdateStep::UpdateContentRelevancy, [](web::Document &document)
                      { (void)document.UpdateRelevancyOfContentVisibilityElements(); });

    RunProcessingStep(RenderingUpdateStep::IntersectionObservations,
                      [](web::Document &document) { document.UpdateIntersectionObservations(); });

    RunProcessingStep(RenderingUpdateStep::SnapshottedScrollOffsets,
                      [](web::Document &document)
                      {
                        if (auto *renderView = document.GetRenderView())
                        {
                          style::AnchorPositionEvaluator::UpdateScrollAdjustments(*renderView);
                        }
                      });

    LayoutIfNeeded({layout::LayoutOptions::UpdateCompositingLayers});
    DoAfterUpdateRendering();

    RunProcessingStep(RenderingUpdateStep::PrepareCanvasesForDisplayOrFlush,
                      [](web::Document &document) { document.PrepareCanvasesForDisplayOrFlushIfNeeded(); });

    ForEachDocument(
      [](web::Document &document)
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

  void Frame::FinalizeRenderingUpdate() noexcept
  {
    // FIXME: Invalidating images whose decodes are asynchronous (invalidateImagesWithAsyncDecodes), so a
    // host's snapshot draws them, once hosts take snapshots.
    _renderingUpdateRemainingSteps.Remove(RenderingUpdateStep::LayerFlush);

    if (RefPtr<FrameView> view = _view)
    {
      view->FlushCompositingState();
    }

    // FIXME: The scrolling tree, once there's asynchronous scrolling.

    RenderingUpdateCompleted();
  }

  void Frame::DoAfterUpdateRendering() noexcept
  {
    // Once-per-update work that needs layout up to date, before painting. It mustn't dirty style or layout:
    // nothing lays out again after it.
    RunProcessingStep(RenderingUpdateStep::CursorUpdate,
                      [](web::Document &document)
                      {
                        if (auto *frame = document.Frame())
                        {
                          frame->GetEventHandler().UpdateCursorIfNeeded();
                        }
                      });

    // FIXME: Paint and event timing, once there are any.

    ForEachDocument([](web::Document &document)
                    { document.Selection().UpdateAppearanceAfterUpdatingRendering(); });

    ForEachRenderableDocument([](web::Document &document) { document.UpdateHighlightPositions(); });
  }

  void Frame::RenderingUpdateCompleted() noexcept
  {
    _renderingUpdateRemainingSteps = {};
    // The renderers destroyed during the update are deleted once it's over.
    ForEachDocument(
      [](web::Document &document)
      {
        if (auto *view = document.View())
        {
          view->LayoutContext().DeleteDetachedRenderersNow();
        }
      });
    // A render-blocked document is updated once it's unblocked, or the block times out.
    if (!_unfulfilledRequestedSteps.empty() || _skippedRenderBlockedDocument)
    {
      _unfulfilledRequestedSteps = {};
      // As any request between updates is, so the Krystal.Web client hears of it.
      ScheduleRenderingUpdate();
    }
  }

  void Frame::LayoutIfNeeded(EnumSet<layout::LayoutOptions> layoutOptions) noexcept
  {
    if (RefPtr<web::Document> document = _document; document && _view)
    {
      (void)document->UpdateLayout(layoutOptions);
    }
  }

  void Frame::ForEachDocument(const Func<void(web::Document &)> &functor) const noexcept
  {
    if (RefPtr<web::Document> document = _document)
    {
      functor(*document);
    }
  }

  void Frame::SetIsVisible(bool isVisible) noexcept
  {
    if (_isVisible == isVisible)
    {
      return;
    }
    _isVisible = isVisible;

    if (isVisible)
    {
      ResumeScriptedAnimations();

      if (RefPtr<FrameView> view = _view)
      {
        view->Show();
        // Shown, the overlay scrollbars show, then fade.
        view->ForEachScrollableArea([](ScrollableArea &area) { area.ContentAreaDidShow(); });
      }

      ForEachDocument(
        [](web::Document &document)
        {
          if (document.GetSettings().HiddenPageCSSAnimationSuspensionEnabled)
          {
            if (auto *timelines = document.TimelinesController())
            {
              timelines->ResumeAnimations();
            }
          }
          if (auto *svgExtensions = document.SVGExtensionsIfExists())
          {
            svgExtensions->UnpauseAnimations();
          }
        });

      ResumeAnimatingImages();

      // Updates asked for while the frame was hidden were put off until now.
      ScheduleRenderingUpdate();
    }
    else
    {
      ForEachDocument(
        [](web::Document &document)
        {
          if (document.GetSettings().HiddenPageCSSAnimationSuspensionEnabled)
          {
            if (auto *timelines = document.TimelinesController())
            {
              timelines->SuspendAnimations();
            }
          }
          if (auto *svgExtensions = document.SVGExtensionsIfExists())
          {
            svgExtensions->PauseAnimations();
          }
        });

      SuspendScriptedAnimations();
      if (RefPtr<FrameView> view = _view)
      {
        view->Hide();
        // Hidden, the overlay scrollbars hide at once.
        view->ForEachScrollableArea([](ScrollableArea &area) { area.ContentAreaDidHide(); });
      }
    }

    ForEachDocument([](web::Document &document) { document.VisibilityStateChanged(); });
  }

  void Frame::SuspendScriptedAnimations() noexcept
  {
    _scriptedAnimationsSuspended = true;
    ForEachDocument([](web::Document &document) { document.SuspendAnimationFrameCallbacks(); });
  }

  void Frame::ResumeScriptedAnimations() noexcept
  {
    _scriptedAnimationsSuspended = false;
    ForEachDocument([](web::Document &document) { document.ResumeAnimationFrameCallbacks(); });
  }

  void Frame::ResumeAnimatingImages() noexcept
  {
    // An animated image only moves on to its next frame as it's drawn, so the visible ones are repainted.
    if (RefPtr<FrameView> view = _view)
    {
      view->ResumeVisibleImageAnimations();
    }
  }

  void Frame::ForEachRenderableDocument(const Func<void(web::Document &)> &functor) const noexcept
  {
    // https://html.spec.whatwg.org/multipage/webappapis.html#update-the-rendering
    // Remove from docs any Document doc for which doc is render-blocked.
    List<Ref<web::Document>> documents;
    ForEachDocument(
      [&](web::Document &document)
      {
        if (html::RenderBlocking::IsRenderBlocked(document))
        {
          _skippedRenderBlockedDocument = true;
          return;
        }
        documents.push_back(ShareRef(document));
      });
    for (auto &document : documents)
    {
      functor(*document);
    }
  }

  void Frame::RunProcessingStep(RenderingUpdateStep step,
                                const Func<void(web::Document &)> &perDocumentFunction) noexcept
  {
    _renderingUpdateRemainingSteps.Remove(step);
    ForEachRenderableDocument(perDocumentFunction);
  }

  bool Frame::UseDarkAppearance() const noexcept
  {
    if (!_view || _view->MediaType() != css::mq::ScreenAtom())
    {
      return false;
    }
    return _useDarkAppearance;
  }

  void Frame::SetDeviceScaleFactor(float deviceScaleFactor) noexcept
  {
    if (_deviceScaleFactor == deviceScaleFactor)
    {
      return;
    }
    _deviceScaleFactor = deviceScaleFactor;
    UpdateStyleAfterChangeInEnvironment();
    DeviceOrPageScaleFactorChanged();
  }

  void Frame::RegisterPopupOpeningObserver(PopupOpeningObserver &observer) noexcept
  {
    _popupOpeningObservers.push_back(&observer);
  }

  void Frame::UnregisterPopupOpeningObserver(PopupOpeningObserver &observer) noexcept
  {
    auto it = std::find(_popupOpeningObservers.begin(), _popupOpeningObservers.end(), &observer);
    krys_debug_assert(it != _popupOpeningObservers.end());
    if (it != _popupOpeningObservers.end())
    {
      _popupOpeningObservers.erase(it);
    }
  }

  void Frame::NotifyPopupOpeningObservers() const noexcept
  {
    // An observer may unregister as it's told.
    auto observers = _popupOpeningObservers;
    for (auto *observer : observers)
    {
      if (std::find(_popupOpeningObservers.begin(), _popupOpeningObservers.end(), observer)
          != _popupOpeningObservers.end())
      {
        observer->WillOpenPopup();
      }
    }
  }

  void Frame::ScreenPropertiesDidChange() noexcept
  {
    UpdateStyleAfterChangeInEnvironment();
  }

  void Frame::SetUseDarkAppearance(bool useDarkAppearance) noexcept
  {
    if (_useDarkAppearance == useDarkAppearance)
    {
      return;
    }
    _useDarkAppearance = useDarkAppearance;
    UpdateStyleAfterChangeInEnvironment();

    // Scrollbars that follow the frame's appearance change with it, though their styles don't.
    ForEachDocument(
      [](web::Document &document)
      {
        if (auto *view = document.View())
        {
          view->InvalidateScrollbarsForAllScrollableAreas();
        }
      });
  }

  void Frame::SetDisplayMode(ApplicationDisplayMode displayMode) noexcept
  {
    if (_displayMode == displayMode)
    {
      return;
    }
    _displayMode = displayMode;
    UpdateStyleAfterChangeInEnvironment();
  }

  void Frame::UpdateStyleAfterChangeInEnvironment() noexcept
  {
    ForEachDocument(
      [](web::Document &document)
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
