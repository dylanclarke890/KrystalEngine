/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001 George Staikos <staikos@kde.org>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2005 Alexey Proskuryakov <ap@nypop.com>
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008 Google Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Animation/Timelines/AnimationTimelinesController.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/RenderBlocking.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Input/EventHandler.hpp"
#include "Krystal.Booey/Input/PointerCaptureController.hpp"
#include "Krystal.Booey/Input/PointerLockController.hpp"
#include "Krystal.Booey/Input/WheelEventDeltaFilter.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/Clients/BooeyClient.hpp"
#include "Krystal.Booey/Page/Drag/DragController.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include "Krystal.Booey/Page/PopupOpeningObserver.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollLatchingController.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Memory/NeverDestroyed.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
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

  Frame::Frame(UniquePtr<BooeyClient> booeyClient, UniquePtr<EditorClient> editorClient,
               UniquePtr<FrameLoaderClient> loaderClient, UniquePtr<DragClient> dragClient) noexcept
      : _booeyClient(krys::move(booeyClient)), _editorClient(krys::move(editorClient)),
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
    if (!_booeyClient)
    {
      _booeyClient = CreateUnique<EmptyBooeyClient>();
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

  Ref<Frame> Frame::Create(UniquePtr<BooeyClient> booeyClient, UniquePtr<EditorClient> editorClient,
                           UniquePtr<FrameLoaderClient> loaderClient,
                           UniquePtr<DragClient> dragClient) noexcept
  {
    return AdoptRef(*new Frame(krys::move(booeyClient), krys::move(editorClient), krys::move(loaderClient),
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

  boo::Window *Frame::Window() const noexcept
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

  boo::Document *Frame::DocumentAtPoint(const PointI &point) const noexcept
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

  void Frame::SetDocument(const RefPtr<boo::Document> &newDocument) noexcept
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

    if (RefPtr<boo::Document> previousDocument = _document)
    {
      previousDocument->WillBeRemovedFromFrame();
    }

    _document = newDocument;

    // Don't use _document because it can be overwritten and we want to guarantee
    // that the document is not destroyed during this function call.
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

    RefPtr<boo::Document> document = _document;
    if (!document)
    {
      return;
    }

    document->ResolveStyle(boo::Document::ResolveStyleType::Rebuild);

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
    RefPtr<boo::Document> document = _document;
    RefPtr<FrameView> frameView = document ? ShareRefPtr(document->View()) : nullptr;

    if (scale == _pageScaleFactor)
    {
      if (frameView && frameView->GetScrollPosition() != origin)
      {
        (void)document->UpdateLayoutIgnorePendingStylesheets({dom::LayoutOptions::UpdateCompositingLayers});
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
          document->ResolveStyle(boo::Document::ResolveStyleType::Rebuild);
        }

        // Transform change on RenderView doesn't trigger repaint on non-composited contents.
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
      if (_booeyClient->ScheduleRenderingUpdate())
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
      if (!_renderingUpdateRemainingSteps.Contains(step))
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
      [enabled](boo::Document &document)
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

  // https://html.spec.whatwg.org/#update-the-rendering
  void Frame::UpdateRendering() noexcept
  {
    Ref protectedThis = ShareRef(*this);

    // This function is not reentrant, e.g. a requestAnimationFrame callback may force a rendering update.
    if (_renderingUpdateDepth)
    {
      LayoutIfNeeded({dom::LayoutOptions::UpdateCompositingLayers});
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
      [](boo::Document &document)
      {
        if (auto *window = document.Window())
        {
          window->FreezeNowTimestamp();
        }
      });

    RunProcessingStep(RenderingUpdateStep::FlushAutofocusCandidates, [](boo::Document &document)
                      { html::FocusAlgorithms::FlushAutofocusCandidates(document); });

    RunProcessingStep(RenderingUpdateStep::Resize,
                      [](boo::Document &document) { document.RunResizeSteps(); });

    RunProcessingStep(RenderingUpdateStep::Scroll,
                      [](boo::Document &document) { document.RunScrollSteps(); });

    RunProcessingStep(RenderingUpdateStep::MediaQueryEvaluation,
                      [](boo::Document &document) { document.EvaluateMediaQueriesAndReportChanges(); });

    // FIXME: Fullscreen events, once there's fullscreen.
    RunProcessingStep(RenderingUpdateStep::Animations,
                      [](boo::Document &document) { document.UpdateAnimationsAndSendEvents(); });

    RunProcessingStep(RenderingUpdateStep::AnimationFrameCallbacks,
                      [](boo::Document &document) { document.ServiceRequestAnimationFrameCallbacks(); });

    RunProcessingStep(RenderingUpdateStep::CaretAnimation,
                      [](boo::Document &document) { document.ServiceCaretAnimation(); });

    // An SVG image's frame is updated with the frame that shows it, which its updates are scheduled with.
    RunProcessingStep(RenderingUpdateStep::Images,
                      [](boo::Document &document)
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

    ForEachDocument([](boo::Document &document) { document.ProcessTopLayerRemovals(); });

    RunProcessingStep(RenderingUpdateStep::ResizeObservations,
                      [this](boo::Document &document) { document.UpdateResizeObservations(*this); });

    // https://drafts.csswg.org/scroll-animations-1/#event-loop
    ForEachDocument([](boo::Document &document) { document.RunPostRenderingUpdateAnimationTasks(); });

    // https://html.spec.whatwg.org/#focus-fixup-rule
    RunProcessingStep(RenderingUpdateStep::FocusFixup,
                      [](boo::Document &document)
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

    RunProcessingStep(RenderingUpdateStep::UpdateContentRelevancy, [](boo::Document &document)
                      { (void)document.UpdateRelevancyOfContentVisibilityElements(); });

    RunProcessingStep(RenderingUpdateStep::IntersectionObservations,
                      [](boo::Document &document) { document.UpdateIntersectionObservations(); });

    RunProcessingStep(RenderingUpdateStep::SnapshottedScrollOffsets,
                      [](boo::Document &document)
                      {
                        if (auto *renderView = document.GetRenderView())
                        {
                          style::AnchorPositionEvaluator::UpdateScrollAdjustments(*renderView);
                        }
                      });

    LayoutIfNeeded({dom::LayoutOptions::UpdateCompositingLayers});
    DoAfterUpdateRendering();

    RunProcessingStep(RenderingUpdateStep::PrepareCanvasesForDisplayOrFlush,
                      [](boo::Document &document) { document.PrepareCanvasesForDisplayOrFlushIfNeeded(); });

    ForEachDocument(
      [](boo::Document &document)
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
    // Code here should do once-per-frame work that needs to be done before painting, and requires
    // layout to be up-to-date. It should not run script, trigger layout, or dirty layout.
    RunProcessingStep(RenderingUpdateStep::CursorUpdate,
                      [](boo::Document &document)
                      {
                        if (auto *frame = document.Frame())
                        {
                          frame->GetEventHandler().UpdateCursorIfNeeded();
                        }
                      });

    // FIXME: Paint and event timing, once there are any.

    ForEachDocument([](boo::Document &document)
                    { document.Selection().UpdateAppearanceAfterUpdatingRendering(); });

    ForEachRenderableDocument([](boo::Document &document) { document.UpdateHighlightPositions(); });
  }

  void Frame::RenderingUpdateCompleted() noexcept
  {
    _renderingUpdateRemainingSteps = {};
    // The renderers destroyed during the update are deleted once it's over (upstream: when the page is idle).
    ForEachDocument(
      [](boo::Document &document)
      {
        if (auto *view = document.View())
        {
          view->LayoutContext().DeleteDetachedRenderersNow();
        }
      });
    // A render-blocked document is updated once it's unblocked, or the block times out.
    if (!_unfulfilledRequestedSteps.IsEmpty() || _skippedRenderBlockedDocument)
    {
      _unfulfilledRequestedSteps = {};
      // As any request between updates is, so the Booey client hears of it.
      ScheduleRenderingUpdate();
    }
  }

  void Frame::LayoutIfNeeded(EnumSet<dom::LayoutOptions> layoutOptions) noexcept
  {
    if (RefPtr<boo::Document> document = _document; document && _view)
    {
      (void)document->UpdateLayout(layoutOptions);
    }
  }

  void Frame::ForEachDocument(const Func<void(boo::Document &)> &functor) const noexcept
  {
    if (RefPtr<boo::Document> document = _document)
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
        [](boo::Document &document)
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
        [](boo::Document &document)
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

    ForEachDocument([](boo::Document &document) { document.VisibilityStateChanged(); });
  }

  void Frame::SuspendScriptedAnimations() noexcept
  {
    _scriptedAnimationsSuspended = true;
    ForEachDocument([](boo::Document &document) { document.SuspendScriptedAnimationControllerCallbacks(); });
  }

  void Frame::ResumeScriptedAnimations() noexcept
  {
    _scriptedAnimationsSuspended = false;
    ForEachDocument([](boo::Document &document) { document.ResumeScriptedAnimationControllerCallbacks(); });
  }

  void Frame::ResumeAnimatingImages() noexcept
  {
    // Drawing models which cache painted content while out-of-window require that we repaint animated images
    // to kickstart the animation loop.
    if (RefPtr<FrameView> view = _view)
    {
      view->ResumeVisibleImageAnimations();
    }
  }

  void Frame::ForEachRenderableDocument(const Func<void(boo::Document &)> &functor) const noexcept
  {
    // https://html.spec.whatwg.org/#update-the-rendering
    // Remove from docs any Document doc for which doc is render-blocked.
    List<Ref<boo::Document>> documents;
    ForEachDocument(
      [&](boo::Document &document)
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
                                const Func<void(boo::Document &)> &perDocumentFunction) noexcept
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
      [](boo::Document &document)
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
      [](boo::Document &document)
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
