/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
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

#pragma once

#include "Krystal.Booey/DOM/Internals/LayoutOptions.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterRenderingMode.hpp"
#include "Krystal.Booey/HTML/Types/DocumentVisibilityState.hpp"
#include "Krystal.Booey/Input/EventHandler.hpp"
#include "Krystal.Booey/Loader/Cache/CachePolicy.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Page/Clients/ChromeClient.hpp"
#include "Krystal.Booey/Page/Clients/DragClient.hpp"
#include "Krystal.Booey/Page/Clients/EditorClient.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/RenderingUpdateStep.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief How the frame is presented, as an application manifest's `display` member (`display-mode`).
  /// @see https://w3c.github.io/manifest/#display-modes
  enum class ApplicationDisplayMode : uint8
  {
    Fullscreen,
    Standalone,
    MinimalUI,
    Browser,
  };
}

namespace krys::boo
{
  class Document;
  class DragCaretController;
  class DragController;
  class GraphicsContext;
  class LocalDOMWindow;
  class PointerCaptureController;
  class PointerLockController;
  class PopupOpeningObserver;
  class ScrollLatchingController;
  class VisiblePosition;
  class WheelEventDeltaFilter;

  /// @brief Shows a document in a view, and runs the rendering update that brings the document up to date
  /// before the host paints it (the specification's "update the rendering"). The host creates a frame, gives
  /// it a view (`CreateView`) and a document (`SetDocument`), and sets the zoom factors and the media
  /// environment. Something that changes what the frame shows schedules a rendering update; the host runs it
  /// at its next rendering opportunity.
  /// @note FIXME: Only what layout, style and navigation within a document need: no frame tree, scripting or
  /// printing. The rest of WebKit's page: the focus controller, the rest of the chrome, page overlays.
  /// @see https://html.spec.whatwg.org/#update-the-rendering
  class Frame final : public RefCounted<Frame>, public CanMakeWeakPtr<Frame>
  {
    KRYS_NON_COPY_MOVABLE(Frame);

  private:
    UniquePtr<ChromeClient> _chromeClient;
    UniquePtr<EditorClient> _editorClient;
    UniquePtr<PointerCaptureController> _pointerCaptureController;
    UniquePtr<PointerLockController> _pointerLockController;
    UniquePtr<ScrollLatchingController> _scrollLatchingController;
    UniquePtr<WheelEventDeltaFilter> _wheelEventDeltaFilter;
    UniquePtr<DragController> _dragController;
    UniquePtr<DragCaretController> _dragCaretController;
    List<PopupOpeningObserver *> _popupOpeningObservers;
    /// @brief The steps of the rendering update in progress that haven't run yet.
    EnumSet<RenderingUpdateStep> _renderingUpdateRemainingSteps;
    /// @brief Steps asked for during a rendering update after they had run, for the next one.
    EnumSet<RenderingUpdateStep> _unfulfilledRequestedSteps;
    uint32 _renderingUpdateDepth {0};
    float _pageScaleFactor {1};
    float _deviceScaleFactor {1};
    bool _renderingUpdateIsScheduled {false};
    /// @brief Whether the rendering update in progress skipped a render-blocked document, which the next one
    /// tries again.
    mutable bool _skippedRenderBlockedDocument {false};
    bool _isVisible {true};
    bool _imageAnimationEnabled {true};
    bool _prefersNonBlinkingCaret {false};
    bool _scriptedAnimationsSuspended {false};
    bool _useDarkAppearance {false};
    ApplicationDisplayMode _displayMode {ApplicationDisplayMode::Browser};
    MonotonicTime::duration _lastRenderingUpdateTimestamp {};
    uint32 _displayNominalFramesPerSecond {FullSpeedFramesPerSecond};
    RefPtr<boo::Document> _document;
    RefPtr<LocalFrameView> _view;
    UniquePtr<EventHandler> _eventHandler;
    UniquePtr<FrameLoader> _loader;
    float _pageZoomFactor {1};
    float _textZoomFactor {1};
    CachePolicy _subresourceCachePolicy {CachePolicy::Use};
    bool _documentIsBeingReplaced {false};

    Frame(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
          UniquePtr<LocalFrameLoaderClient> loaderClient, UniquePtr<DragClient> dragClient) noexcept;

  public:
    ~Frame() noexcept;

    /// @brief A frame, which has no view or document yet.
    /// @param chromeClient The host's side of the frame; without one, the frame has an empty chrome client.
    /// @param editorClient The host's side of the frame's editor; without one, the frame has an empty editor
    /// client.
    /// @param loaderClient The host's side of the frame's loader; without one, the frame ignores navigations.
    /// @param dragClient The host's side of drags in the frame; without one, no drag starts in the frame.
    KRYS_NODISCARD static Ref<Frame> Create(UniquePtr<ChromeClient> chromeClient = nullptr,
                                            UniquePtr<EditorClient> editorClient = nullptr,
                                            UniquePtr<LocalFrameLoaderClient> loaderClient = nullptr,
                                            UniquePtr<DragClient> dragClient = nullptr) noexcept;

    /// @brief Calls @p function with each frame that's alive, for changes that reach every frame (the
    /// scrollbars' style).
    static void ForEachFrame(const Func<void(Frame &)> &function) noexcept;

    KRYS_NODISCARD ChromeClient &GetChromeClient() const noexcept
    {
      return *_chromeClient;
    }

    /// @brief Tells @p observer before each popup the frame opens, until it unregisters.
    void RegisterPopupOpeningObserver(PopupOpeningObserver &observer) noexcept;

    void UnregisterPopupOpeningObserver(PopupOpeningObserver &observer) noexcept;

    /// @brief Tells the popup opening observers a popup is about to open; whoever opens one calls it first.
    void NotifyPopupOpeningObservers() const noexcept;

    KRYS_NODISCARD EditorClient &GetEditorClient() const noexcept
    {
      return *_editorClient;
    }

    /// @brief Loads documents into the frame; links and form submissions go through it to the host.
    KRYS_NODISCARD FrameLoader &Loader() const noexcept
    {
      return *_loader;
    }

    /// @brief How the document's subresources use the memory cache, unless the load is a reload: the host
    /// sets `Reload` when it loads a page again, so edited files load again.
    KRYS_NODISCARD CachePolicy SubresourceCachePolicy() const noexcept
    {
      return _subresourceCachePolicy;
    }

    void SetSubresourceCachePolicy(CachePolicy policy) noexcept
    {
      _subresourceCachePolicy = policy;
    }

    /// @brief The document the frame shows, if any.
    KRYS_NODISCARD boo::Document *Document() const noexcept
    {
      return _document.get();
    }

    /// @brief The window of the frame's document, if any.
    KRYS_NODISCARD LocalDOMWindow *Window() const noexcept;

    /// @brief The view the frame's document is laid out in, if any.
    KRYS_NODISCARD LocalFrameView *View() const noexcept
    {
      return _view.get();
    }

    /// @brief Turns the host's mouse input into DOM events for this frame.
    KRYS_NODISCARD EventHandler &GetEventHandler() const noexcept
    {
      return *_eventHandler;
    }

    /// @brief Replaces the frame's view. The document's render tree is destroyed with the old view and
    /// created with the new one.
    void SetView(RefPtr<LocalFrameView> &&view) noexcept;

    /// @brief Gives the frame a new view of @p viewportSize, laid out at @p fixedLayoutSize if
    /// @p useFixedLayout.
    void CreateView(const SizeI &viewportSize, const SizeI &fixedLayoutSize = {},
                    bool useFixedLayout = false) noexcept;

    /// @brief Shows @p newDocument in the frame, in place of its current document. @p newDocument must not be
    /// in a frame already.
    // FIXME: The rest of what's reset when the document is replaced (the sampled top color, element targeting
    // and the like), once there are those.
    void SetDocument(const RefPtr<boo::Document> &newDocument) noexcept;

    /// @brief The caret position nearest @p framePoint (in the frame's contents).
    KRYS_NODISCARD VisiblePosition VisiblePositionForPoint(const PointI &framePoint) const noexcept;

    /// @brief The document under @p point (in the window).
    KRYS_NODISCARD boo::Document *DocumentAtPoint(const PointI &point) const noexcept;

    /// @brief The zoom applied to the whole document (the root style's `zoom`).
    KRYS_NODISCARD float PageZoomFactor() const noexcept
    {
      return _pageZoomFactor;
    }

    /// @brief The zoom applied to font sizes, on top of the page zoom.
    KRYS_NODISCARD float TextZoomFactor() const noexcept
    {
      return _textZoomFactor;
    }

    void SetPageZoomFactor(float factor) noexcept;

    void SetTextZoomFactor(float factor) noexcept;

    void SetPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor) noexcept;

    /// @brief How much the frame's content is scaled: the page scale.
    KRYS_NODISCARD float FrameScaleFactor() const noexcept
    {
      return _pageScaleFactor;
    }

    /// @brief Tells the frame's compositor the device or page scale changed.
    void DeviceOrPageScaleFactorChanged() noexcept;

    /// @brief Device pixels per CSS pixel.
    KRYS_NODISCARD float DeviceScaleFactor() const noexcept
    {
      return _deviceScaleFactor;
    }

    /// @brief Changes the device scale factor, as the host's screen reports, and restyles the document.
    void SetDeviceScaleFactor(float deviceScaleFactor) noexcept;

    /// @brief Whether the frame is drawn with the dark appearance: the host's appearance, while the frame is
    /// shown on a screen.
    KRYS_NODISCARD bool UseDarkAppearance() const noexcept;

    /// @brief Changes the host's appearance, and restyles the document.
    void SetUseDarkAppearance(bool useDarkAppearance) noexcept;

    /// @brief How the frame is presented.
    KRYS_NODISCARD ApplicationDisplayMode DisplayMode() const noexcept
    {
      return _displayMode;
    }

    /// @brief Changes how the frame is presented, and restyles the document.
    void SetDisplayMode(ApplicationDisplayMode displayMode) noexcept;

    /// @brief Shows or hides the frame, as its host reports: hidden while its window is minimised or another
    /// view is shown in its place. A hidden frame suspends its animation frame callbacks and SMIL animations,
    /// and its CSS animations unless `Settings::HiddenPageCSSAnimationSuspensionEnabled` is off; its
    /// document's visibility state follows it, and its rendering updates wait until it's shown.
    void SetIsVisible(bool isVisible) noexcept;

    /// @brief Whether the frame is shown; it is until its host hides it.
    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _isVisible;
    }

    /// @brief The visibility state of the frame's document.
    /// @see https://html.spec.whatwg.org/#visibility-state
    KRYS_NODISCARD html::DocumentVisibilityState VisibilityState() const noexcept
    {
      return _isVisible ? html::DocumentVisibilityState::Visible : html::DocumentVisibilityState::Hidden;
    }

    /// @brief Whether the frame's animation frame callbacks are suspended; a document's controller made while
    /// they are starts suspended.
    KRYS_NODISCARD bool ScriptedAnimationsSuspended() const noexcept
    {
      return _scriptedAnimationsSuspended;
    }

    void SuspendScriptedAnimations() noexcept;

    void ResumeScriptedAnimations() noexcept;

    /// @brief Resumes the paused animated images visible in the frame's view.
    void ResumeAnimatingImages() noexcept;

    /// @brief The frame's pointers and the elements capturing them.
    KRYS_NODISCARD PointerCaptureController &GetPointerCaptureController() const noexcept
    {
      return *_pointerCaptureController;
    }

    /// @brief The element and scroller a touchpad's gesture keeps to.
    KRYS_NODISCARD ScrollLatchingController &GetScrollLatchingController() const noexcept
    {
      return *_scrollLatchingController;
    }

    /// @brief Keeps a touchpad's gesture to one axis.
    KRYS_NODISCARD WheelEventDeltaFilter &GetWheelEventDeltaFilter() const noexcept
    {
      return *_wheelEventDeltaFilter;
    }

    /// @brief The frame's pointer lock.
    KRYS_NODISCARD PointerLockController &GetPointerLockController() const noexcept
    {
      return *_pointerLockController;
    }

    /// @brief The frame's drags and drops.
    KRYS_NODISCARD DragController &GetDragController() const noexcept
    {
      return *_dragController;
    }

    /// @brief The caret showing where a drag would drop.
    KRYS_NODISCARD DragCaretController &GetDragCaretController() const noexcept
    {
      return *_dragCaretController;
    }

    /// @brief How much the frame is scaled, as by pinching: its content is drawn scaled by it (about the
    /// document's origin), and scrolls in the scaled content, without being laid out again.
    KRYS_NODISCARD float PageScaleFactor() const noexcept
    {
      return _pageScaleFactor;
    }

    /// @brief Scales the frame by @p scale, and scrolls it to @p origin (in the scaled content).
    // FIXME: Telling media elements in a stable state (inStableState, pageScaleFactorChanged), once there are
    // media elements.
    void SetPageScaleFactor(float scale, const PointI &origin) noexcept;

    /// @brief Asks for a rendering update at the next rendering opportunity, which runs (at least) the
    /// @p requestedSteps.
    void ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps = {}) noexcept;

    /// @brief Restyles the document after something its styles depend on outside the style sheets changed,
    /// and evaluates its media queries at the next rendering update.
    void UpdateStyleAfterChangeInEnvironment() noexcept;

    /// @brief Restyles the frame's document after what the chrome client says of the screen (its colours,
    /// depth and the like) changed, so its media queries are evaluated again.
    void ScreenPropertiesDidChange() noexcept;

    /// @brief How the frame's filters are applied when drawn into @p context: through the graphics context
    /// (there's no software or accelerated filter rendering).
    KRYS_NODISCARD static EnumSet<FilterRenderingMode>
      PreferredFilterRenderingModes(const GraphicsContext &context) noexcept;

    KRYS_NODISCARD bool IsRenderingUpdateScheduled() const noexcept
    {
      return _renderingUpdateIsScheduled;
    }

    /// @brief Lets animated images and SMIL animations play (true), or holds them (false): animated images
    /// stop at the frame they show, and SMIL animations pause, in the frame's document and its SVG images. A
    /// frame starts with them playing.
    void SetImageAnimationEnabled(bool enabled) noexcept;

    /// @brief Keeps carets shown rather than blinking, for users who prefer a caret that doesn't blink (an
    /// accessibility preference the host reports). A frame starts with carets blinking.
    void SetPrefersNonBlinkingCaret(bool prefers) noexcept
    {
      _prefersNonBlinkingCaret = prefers;
    }

    KRYS_NODISCARD bool PrefersNonBlinkingCaret() const noexcept
    {
      return _prefersNonBlinkingCaret;
    }

    KRYS_NODISCARD bool ImageAnimationEnabled() const noexcept
    {
      return _imageAnimationEnabled;
    }

    /// @brief The rate the frame is shown at when nothing tells it otherwise.
    constexpr static uint32 FullSpeedFramesPerSecond {60};

    /// @brief Sets the rate the host runs the frame's rendering updates at (its display's refresh rate, or
    /// the rate it caps its frames to), which animations align their updates to.
    void SetDisplayNominalFramesPerSecond(uint32 framesPerSecond) noexcept;

    KRYS_NODISCARD uint32 DisplayNominalFramesPerSecond() const noexcept
    {
      return _displayNominalFramesPerSecond;
    }

    /// @brief The time between the frame's rendering updates, at the host's rate.
    KRYS_NODISCARD MonotonicTime::duration PreferredRenderingUpdateInterval() const noexcept;

    /// @brief The event loop's time at the start of the last rendering update.
    KRYS_NODISCARD MonotonicTime::duration LastRenderingUpdateTimestamp() const noexcept
    {
      return _lastRenderingUpdateTimestamp;
    }

    /// @brief Runs the rendering update: the steps the frame's document needs, in the specification's order,
    /// with layout before and after the steps that read it, then flushes the composited layers' changes. A
    /// hidden frame's update does nothing, and stays scheduled.
    void UpdateRendering() noexcept;

    /// @brief Brings the style and layout of the frame's document up to date.
    void LayoutIfNeeded(EnumSet<dom::LayoutOptions> layoutOptions = {}) noexcept;

    void ForEachDocument(const Func<void(boo::Document &)> &functor) const noexcept;

    /// @brief Restyles every frame's document (UpdateStyleAfterChangeInEnvironment), after a change in the
    /// environment they all share, such as the theme's colours.
    static void UpdateStyleForAllFramesAfterGlobalChangeInEnvironment() noexcept;

    /// @brief Calls @p functor for each document that's rendered: not render-blocked ones.
    void ForEachRenderableDocument(const Func<void(boo::Document &)> &functor) const noexcept;

  private:
    void RunProcessingStep(RenderingUpdateStep step,
                           const Func<void(boo::Document &)> &perDocumentFunction) noexcept;

    void DoAfterUpdateRendering() noexcept;

    void FinalizeRenderingUpdate() noexcept;

    void RenderingUpdateCompleted() noexcept;
  };
}
