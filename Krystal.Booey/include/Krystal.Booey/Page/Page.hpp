#pragma once

#include "Krystal.Booey/DOM/Internals/LayoutOptions.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterRenderingMode.hpp"
#include "Krystal.Booey/HTML/Types/DocumentVisibilityState.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/Drag/DragClient.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class DragCaretController;
  class DragController;
  class GraphicsContext;
  class LocalFrame;
  class PointerCaptureController;
  class PointerLockController;

  /// @brief The steps of the rendering update that something asked for.
  // FIXME: The steps for the features Booey doesn't have yet: revealing documents, fullscreen, paint and
  // event timing, video frame callbacks, validation messages and the scrolling tree update.
  enum class RenderingUpdateStep : uint8
  {
    FlushAutofocusCandidates,
    Resize,
    Scroll,
    MediaQueryEvaluation,
    Animations,
    AnimationFrameCallbacks,
    CaretAnimation,
    Images,
    ResizeObservations,
    FocusFixup,
    UpdateContentRelevancy,
    IntersectionObservations,
    SnapshottedScrollOffsets,
    CursorUpdate,
    PrepareCanvasesForDisplayOrFlush,
    LayerFlush,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderingUpdateStep, 16uz);

namespace krys::boo
{
  /// @brief A page: its main frame, and the rendering update that brings the page's documents up to date
  /// before the host paints them (the specification's "update the rendering"). Something that changes what
  /// the page shows schedules a rendering update; the host runs it at its next rendering opportunity.
  /// @see https://html.spec.whatwg.org/#update-the-rendering
  // FIXME: The rest of the page: settings, the focus controller, the rest of the chrome, page overlays.
  class Page final : public RefCounted<Page>, public CanMakeWeakPtr<Page>
  {
    KRYS_NON_COPY_MOVABLE(Page);

  private:
    UniquePtr<ChromeClient> _chromeClient;
    UniquePtr<EditorClient> _editorClient;
    UniquePtr<PointerCaptureController> _pointerCaptureController;
    UniquePtr<PointerLockController> _pointerLockController;
    UniquePtr<DragController> _dragController;
    UniquePtr<DragCaretController> _dragCaretController;
    RefPtr<LocalFrame> _mainFrame;
    /// @brief The steps of the rendering update in progress that haven't run yet.
    EnumSet<RenderingUpdateStep> _renderingUpdateRemainingSteps;
    /// @brief Steps asked for during a rendering update after they had run, for the next one.
    EnumSet<RenderingUpdateStep> _unfulfilledRequestedSteps;
    uint32 _renderingUpdateDepth {0};
    float _pageScaleFactor {1};
    bool _renderingUpdateIsScheduled {false};
    /// @brief Whether the rendering update in progress skipped a render-blocked document, which the next one
    /// tries again.
    mutable bool _skippedRenderBlockedDocument {false};
    bool _isVisible {true};
    bool _imageAnimationEnabled {true};
    bool _scriptedAnimationsSuspended {false};
    MonotonicTime::duration _lastRenderingUpdateTimestamp {};
    uint32 _displayNominalFramesPerSecond {FullSpeedFramesPerSecond};

    Page(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
         UniquePtr<DragClient> dragClient) noexcept;

  public:
    ~Page() noexcept;

    /// @brief A page with a main frame, which has no view or document yet.
    /// @param chromeClient The host's side of the page; without one, the page has an empty chrome client.
    /// @param editorClient The host's side of the page's editor; without one, the page has an empty editor
    /// client.
    /// @param mainFrameLoaderClient The host's side of the main frame's loader; without one, the main frame
    /// ignores navigations.
    /// @param dragClient The host's side of drags in the page; without one, no drag starts in the page.
    KRYS_NODISCARD static Ref<Page> Create(UniquePtr<ChromeClient> chromeClient = nullptr,
                                           UniquePtr<EditorClient> editorClient = nullptr,
                                           UniquePtr<LocalFrameLoaderClient> mainFrameLoaderClient = nullptr,
                                           UniquePtr<DragClient> dragClient = nullptr) noexcept;

    KRYS_NODISCARD ChromeClient &GetChromeClient() const noexcept
    {
      return *_chromeClient;
    }

    KRYS_NODISCARD EditorClient &GetEditorClient() const noexcept
    {
      return *_editorClient;
    }

    KRYS_NODISCARD LocalFrame &MainFrame() const noexcept
    {
      return *_mainFrame;
    }

    /// @brief Whether the page is drawn with the dark appearance: the host's appearance, while the page is
    /// shown on a screen.
    KRYS_NODISCARD bool UseDarkAppearance() const noexcept;

    /// @brief Shows or hides the page, as its host reports: hidden while its window is minimised or another
    /// view is shown in its place. A hidden page suspends its animation frame callbacks and SMIL animations,
    /// and its CSS animations unless `Settings::HiddenPageCSSAnimationSuspensionEnabled` is off; its
    /// documents' visibility state follows it, and its rendering updates wait until it's shown.
    void SetIsVisible(bool isVisible) noexcept;

    /// @brief Whether the page is shown; it is until its host hides it.
    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _isVisible;
    }

    /// @brief The visibility state of the page's documents.
    /// @see https://html.spec.whatwg.org/#visibility-state
    KRYS_NODISCARD html::DocumentVisibilityState VisibilityState() const noexcept
    {
      return _isVisible ? html::DocumentVisibilityState::Visible : html::DocumentVisibilityState::Hidden;
    }

    /// @brief Whether the page's animation frame callbacks are suspended; a document's controller made while
    /// they are starts suspended.
    KRYS_NODISCARD bool ScriptedAnimationsSuspended() const noexcept
    {
      return _scriptedAnimationsSuspended;
    }

    void SuspendScriptedAnimations() noexcept;
    void ResumeScriptedAnimations() noexcept;

    /// @brief Resumes the paused animated images visible in the main frame's view.
    void ResumeAnimatingImages() noexcept;

    /// @brief The page's pointers and the elements capturing them.
    KRYS_NODISCARD PointerCaptureController &GetPointerCaptureController() const noexcept
    {
      return *_pointerCaptureController;
    }

    /// @brief The page's pointer lock.
    KRYS_NODISCARD PointerLockController &GetPointerLockController() const noexcept
    {
      return *_pointerLockController;
    }

    /// @brief The page's drags and drops.
    KRYS_NODISCARD DragController &GetDragController() const noexcept
    {
      return *_dragController;
    }

    /// @brief The caret showing where a drag would drop.
    KRYS_NODISCARD DragCaretController &GetDragCaretController() const noexcept
    {
      return *_dragCaretController;
    }

    /// @brief How much the page is scaled, as by pinching: the main frame's content is drawn scaled by it
    /// (about the document's origin), and scrolls in the scaled content, without being laid out again.
    KRYS_NODISCARD float PageScaleFactor() const noexcept
    {
      return _pageScaleFactor;
    }
    /// @brief Scales the page by @p scale, and scrolls the main frame to @p origin (in the scaled content).
    // FIXME: Telling media elements in a stable state (inStableState, pageScaleFactorChanged), once there are
    // media elements.
    void SetPageScaleFactor(float scale, const IntPoint &origin) noexcept;

    /// @brief Called when the main frame's document is replaced by @p newDocument.
    // FIXME: The rest of what the page resets (the sampled top color, element targeting and the like), once
    // there are those.
    void DidChangeMainDocument(dom::Document *newDocument) noexcept;

    /// @brief Asks for a rendering update at the next rendering opportunity, which runs (at least) the
    /// @p requestedSteps.
    void ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps = {}) noexcept;
    /// @brief Restyles every document after something their styles depend on outside the style sheets
    /// changed, and evaluates their media queries at the next rendering update.
    void UpdateStyleAfterChangeInEnvironment() noexcept;
    /// @brief How the page's filters are applied when drawn into @p context: through the graphics context
    /// (there's no software or accelerated filter rendering).
    KRYS_NODISCARD EnumSet<FilterRenderingMode>
      PreferredFilterRenderingModes(const GraphicsContext &context) const noexcept;

    KRYS_NODISCARD bool IsRenderingUpdateScheduled() const noexcept
    {
      return _renderingUpdateIsScheduled;
    }

    /// @brief Lets animated images and SMIL animations play (true), or holds them (false): animated images
    /// stop at the frame they show, and SMIL animations pause, in the page's documents and its SVG images. A
    /// page starts with them playing.
    void SetImageAnimationEnabled(bool enabled) noexcept;
    KRYS_NODISCARD bool ImageAnimationEnabled() const noexcept
    {
      return _imageAnimationEnabled;
    }

    /// @brief The rate the page is shown at when nothing tells it otherwise.
    static constexpr uint32 FullSpeedFramesPerSecond {60};
    /// @brief Sets the rate the host runs the page's rendering updates at (its display's refresh rate, or the
    /// rate it caps its frames to), which animations align their updates to.
    void SetDisplayNominalFramesPerSecond(uint32 framesPerSecond) noexcept;
    KRYS_NODISCARD uint32 DisplayNominalFramesPerSecond() const noexcept
    {
      return _displayNominalFramesPerSecond;
    }
    /// @brief The time between the page's rendering updates, at the host's rate.
    KRYS_NODISCARD MonotonicTime::duration PreferredRenderingUpdateInterval() const noexcept;
    /// @brief The event loop's time at the start of the last rendering update.
    KRYS_NODISCARD MonotonicTime::duration LastRenderingUpdateTimestamp() const noexcept
    {
      return _lastRenderingUpdateTimestamp;
    }

    /// @brief Runs the rendering update: the steps the page's documents need, in the specification's order,
    /// with layout before and after the steps that read it, then flushes the composited layers' changes. A
    /// hidden page's update does nothing, and stays scheduled.
    void UpdateRendering() noexcept;

    /// @brief Brings the style and layout of the page's documents up to date.
    void LayoutIfNeeded(EnumSet<dom::LayoutOptions> layoutOptions = {}) noexcept;

    void ForEachDocument(const Func<void(dom::Document &)> &functor) const noexcept;
    /// @brief Calls @p functor for each document that's rendered: not render-blocked ones.
    void ForEachRenderableDocument(const Func<void(dom::Document &)> &functor) const noexcept;

  private:
    void RunProcessingStep(RenderingUpdateStep step,
                           const Func<void(dom::Document &)> &perDocumentFunction) noexcept;
    void DoAfterUpdateRendering() noexcept;
    void FinalizeRenderingUpdate() noexcept;
    void RenderingUpdateCompleted() noexcept;
  };
}
