#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Animation/Timelines/AnimationTimelinesController.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/Drag/DragController.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief The SVG images @p document has loaded, each with its own page.
    KRYS_NODISCARD List<Ref<svg::SVGImage>> SVGImagesOf(dom::Document &document) noexcept
    {
      List<Ref<svg::SVGImage>> images;
      for (const auto &[url, resource] : document.GetCachedResourceLoader().AllCachedResources())
      {
        auto *cachedImage = DynamicDowncast<CachedImage>(resource.get());
        if (auto *image = cachedImage ? DynamicDowncast<svg::SVGImage>(cachedImage->GetImage()) : nullptr)
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

  Page::Page(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
             UniquePtr<DragClient> dragClient) noexcept
      : _chromeClient(krys::move(chromeClient)), _editorClient(krys::move(editorClient)),
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

  Page::~Page() noexcept = default;

  Ref<Page> Page::Create(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
                         UniquePtr<LocalFrameLoaderClient> mainFrameLoaderClient,
                         UniquePtr<DragClient> dragClient) noexcept
  {
    Ref<Page> page =
      AdoptRef(*new Page(krys::move(chromeClient), krys::move(editorClient), krys::move(dragClient)));
    page->_mainFrame = LocalFrame::Create(page.get(), krys::move(mainFrameLoaderClient));
    return page;
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

  void Page::SetImageAnimationEnabled(bool enabled) noexcept
  {
    if (_imageAnimationEnabled == enabled)
    {
      return;
    }
    _imageAnimationEnabled = enabled;

    ForEachDocument(
      [enabled](dom::Document &document)
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

        // An SVG image's animations are its page's SMIL animations.
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

  void Page::SetDisplayNominalFramesPerSecond(uint32 framesPerSecond) noexcept
  {
    krys_debug_assert(framesPerSecond);
    _displayNominalFramesPerSecond = std::max(framesPerSecond, 1u);
  }

  MonotonicTime::duration Page::PreferredRenderingUpdateInterval() const noexcept
  {
    return std::chrono::duration_cast<MonotonicTime::duration>(
      std::chrono::duration<double>(1.0 / _displayNominalFramesPerSecond));
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
    _skippedRenderBlockedDocument = false;

    LayoutIfNeeded();

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

    // An SVG image's page is updated with the page that shows it, which its updates are scheduled with.
    RunProcessingStep(RenderingUpdateStep::Images,
                      [](dom::Document &document)
                      {
                        for (auto &image : SVGImagesOf(document))
                        {
                          if (RefPtr<Page> page = ShareRefPtr(image->InternalPage()))
                          {
                            page->UpdateRendering();
                          }
                        }
                      });

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
                        // (There's no navigation API, whose focus changed during ongoing navigation would be
                        // set to false.)
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

  void Page::FinalizeRenderingUpdate() noexcept
  {
    // FIXME: Images with asynchronous decodes, once there are any.
    _renderingUpdateRemainingSteps.Remove(RenderingUpdateStep::LayerFlush);

    if (RefPtr<LocalFrameView> view = ShareRefPtr(_mainFrame->View()))
    {
      view->FlushCompositingState();
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
    // A render-blocked document is updated once it's unblocked, or the block times out.
    if (!_unfulfilledRequestedSteps.IsEmpty() || _skippedRenderBlockedDocument)
    {
      _unfulfilledRequestedSteps = {};
      // As any request between updates is, so the chrome client hears of it.
      ScheduleRenderingUpdate();
    }
  }

  void Page::LayoutIfNeeded(EnumSet<dom::LayoutOptions> layoutOptions) noexcept
  {
    if (RefPtr<dom::Document> document = ShareRefPtr(_mainFrame->Document()); document && _mainFrame->View())
    {
      (void)document->UpdateLayout(layoutOptions);
    }
  }

  void Page::ForEachDocument(const Func<void(dom::Document &)> &functor) const noexcept
  {
    if (RefPtr<dom::Document> document = ShareRefPtr(_mainFrame->Document()))
    {
      functor(*document);
    }
  }

  void Page::SetIsVisible(bool isVisible) noexcept
  {
    if (_isVisible == isVisible)
    {
      return;
    }
    _isVisible = isVisible;

    if (isVisible)
    {
      ResumeScriptedAnimations();

      if (RefPtr<LocalFrameView> view = ShareRefPtr(_mainFrame->View()))
      {
        view->Show();
      }

      ForEachDocument(
        [](dom::Document &document)
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
    }
    else
    {
      ForEachDocument(
        [](dom::Document &document)
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
      if (RefPtr<LocalFrameView> view = ShareRefPtr(_mainFrame->View()))
      {
        view->Hide();
      }
    }

    ForEachDocument([](dom::Document &document) { document.VisibilityStateChanged(); });
  }

  void Page::SuspendScriptedAnimations() noexcept
  {
    _scriptedAnimationsSuspended = true;
    ForEachDocument([](dom::Document &document) { document.SuspendScriptedAnimationControllerCallbacks(); });
  }

  void Page::ResumeScriptedAnimations() noexcept
  {
    _scriptedAnimationsSuspended = false;
    ForEachDocument([](dom::Document &document) { document.ResumeScriptedAnimationControllerCallbacks(); });
  }

  void Page::ResumeAnimatingImages() noexcept
  {
    // Drawing models which cache painted content while out-of-window require that we repaint animated images
    // to kickstart the animation loop.
    if (RefPtr<LocalFrameView> view = ShareRefPtr(_mainFrame->View()))
    {
      view->ResumeVisibleImageAnimations();
    }
  }

  void Page::ForEachRenderableDocument(const Func<void(dom::Document &)> &functor) const noexcept
  {
    // https://html.spec.whatwg.org/#update-the-rendering
    // Remove from docs any Document doc for which doc is render-blocked.
    List<Ref<dom::Document>> documents;
    ForEachDocument(
      [&](dom::Document &document)
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

  void Page::RunProcessingStep(RenderingUpdateStep step,
                               const Func<void(dom::Document &)> &perDocumentFunction) noexcept
  {
    _renderingUpdateRemainingSteps.Remove(step);
    ForEachRenderableDocument(perDocumentFunction);
  }

  bool Page::UseDarkAppearance() const noexcept
  {
    auto *view = MainFrame().View();
    if (!view || view->MediaType() != css::mq::ScreenAtom())
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
