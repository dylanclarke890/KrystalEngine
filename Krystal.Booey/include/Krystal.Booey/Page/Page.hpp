#pragma once

#include "Krystal.Booey/DOM/Enums/LayoutOptions.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterRenderingMode.hpp"
#include "Krystal.Booey/History/BackForwardClient.hpp"
#include "Krystal.Booey/Loader/LocalFrameLoaderClient.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Storage/StorageArea.hpp"
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
  class BackForwardController;
  class GraphicsContext;
  class HistoryItem;
  class LocalFrame;

  /// @brief The steps of the rendering update that something asked for.
  // FIXME: The steps for the features Booey doesn't have yet: revealing documents, fullscreen, view
  // transitions, intersection and resize observations, content relevancy, paint and event timing, video
  // frame callbacks, caret animations, validation messages and the scrolling tree update.
  enum class RenderingUpdateStep : uint8
  {
    RestoreScrollPositionAndViewState,
    FlushAutofocusCandidates,
    Resize,
    Scroll,
    MediaQueryEvaluation,
    Animations,
    AnimationFrameCallbacks,
    CaretAnimation,
    ResizeObservations,
    FocusFixup,
    SnapshottedScrollOffsets,
    CursorUpdate,
    LayerFlush,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderingUpdateStep, 13uz);

namespace krys::boo
{
  /// @brief A page: its main frame, and the rendering update that brings the page's documents up to date
  /// before the host paints them (the specification's "update the rendering"). Something that changes what
  /// the page shows schedules a rendering update; the host runs it at its next rendering opportunity.
  /// @see https://html.spec.whatwg.org/#update-the-rendering
  // FIXME: The rest of the page: settings, the focus controller, the rest of the chrome, page overlays,
  // frames other than the main frame.
  class Page final : public RefCounted<Page>, public CanMakeWeakPtr<Page>
  {
    KRYS_NON_COPY_MOVABLE(Page);

  private:
    UniquePtr<ChromeClient> _chromeClient;
    UniquePtr<EditorClient> _editorClient;
    UniquePtr<BackForwardController> _backForwardController;
    RefPtr<LocalFrame> _mainFrame;
    /// @brief The steps of the rendering update in progress that haven't run yet.
    EnumSet<RenderingUpdateStep> _renderingUpdateRemainingSteps;
    /// @brief Steps asked for during a rendering update after they had run, for the next one.
    EnumSet<RenderingUpdateStep> _unfulfilledRequestedSteps;
    uint32 _renderingUpdateDepth {0};
    bool _renderingUpdateIsScheduled {false};
    bool _isClosing {false};
    /// @brief The page's session storage (its top-level traversable's), by origin.
    HashMap<dom::DOMString, Ref<StorageArea>> _sessionStorageAreas;
    MonotonicTime::duration _lastRenderingUpdateTimestamp {};

    Page(UniquePtr<ChromeClient> chromeClient, UniquePtr<EditorClient> editorClient,
         RefPtr<BackForwardClient> &&backForwardClient) noexcept;

  public:
    ~Page() noexcept;

    /// @brief A page with a main frame, which has no view or document yet.
    /// @param chromeClient The host's side of the page; without one, the page has an empty chrome client.
    /// @param editorClient The host's side of the page's editor; without one, the page has an empty editor
    /// client.
    /// @param mainFrameLoaderClient The host's side of the main frame's loader; without one, the main frame
    /// ignores navigations.
    /// @param backForwardClient The page's back/forward list; without one, the page keeps it in memory (a
    /// `BackForwardList`).
    KRYS_NODISCARD static Ref<Page> Create(UniquePtr<ChromeClient> chromeClient = nullptr,
                                           UniquePtr<EditorClient> editorClient = nullptr,
                                           UniquePtr<LocalFrameLoaderClient> mainFrameLoaderClient = nullptr,
                                           RefPtr<BackForwardClient> backForwardClient = nullptr) noexcept;

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

    /// @brief The page's session history.
    KRYS_NODISCARD BackForwardController &BackForward() const noexcept
    {
      return *_backForwardController;
    }

    /// @brief Goes to @p item in @p frame (back or forward), once the frame's load stops, if it's for another
    /// document.
    void GoToItem(LocalFrame &frame, HistoryItem &item, FrameLoadType type) noexcept;

    /// @brief Goes to @p item in @p frame for the Navigation API (`traverseTo()`): if its `navigate` is
    /// canceled, @p tracker's promises are rejected.
    void GoToItemForNavigationAPI(LocalFrame &frame, HistoryItem &item, FrameLoadType type,
                                  LocalFrame &triggeringFrame,
                                  struct NavigationAPIMethodTracker *tracker) noexcept;

    /// @brief Whether the page's window asked to be closed (its top-level traversable's is closing).
    /// @see https://html.spec.whatwg.org/#is-closing
    KRYS_NODISCARD bool IsClosing() const noexcept
    {
      return _isClosing;
    }

    void SetIsClosing() noexcept
    {
      _isClosing = true;
    }

    /// @brief The session storage area of @p origin in this page, made on first use.
    /// @see https://html.spec.whatwg.org/#obtain-a-session-storage-bottle-map
    KRYS_NODISCARD Ref<StorageArea> SessionStorageArea(dom::DOMStringView origin) noexcept;

    /// @brief The host's network became reachable (@p isOnLine) or not: `online` or `offline` fires at each
    /// of the page's windows.
    /// @see https://html.spec.whatwg.org/#event-online
    void NetworkStateChanged(bool isOnLine) noexcept;

    /// @brief Starts an idle period in each of the page's documents that has idle callbacks, which runs them
    /// until @p deadline (in real time). The host calls this after a rendering update, with the time it can
    /// spare before its next frame.
    /// @see https://w3c.github.io/requestidlecallback/#start-an-idle-period-algorithm
    void OpportunisticallyRunIdleCallbacks(MonotonicTime::duration deadline) noexcept;

    /// @brief Asks for a rendering update at the next rendering opportunity, which runs (at least) the
    /// @p requestedSteps.
    void ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps = {}) noexcept;
    /// @brief How the page's filters are applied when drawn into @p context: through the graphics context
    /// (there's no software or accelerated filter rendering).
    KRYS_NODISCARD EnumSet<FilterRenderingMode>
      PreferredFilterRenderingModes(const GraphicsContext &context) const noexcept;

    KRYS_NODISCARD bool IsRenderingUpdateScheduled() const noexcept
    {
      return _renderingUpdateIsScheduled;
    }
    /// @brief The event loop's time at the start of the last rendering update.
    KRYS_NODISCARD MonotonicTime::duration LastRenderingUpdateTimestamp() const noexcept
    {
      return _lastRenderingUpdateTimestamp;
    }

    /// @brief Runs the rendering update: the steps the page's documents need, in the specification's order,
    /// with layout before and after the steps that read it, then flushes the composited layers' changes.
    void UpdateRendering() noexcept;

    /// @brief Brings the style and layout of the page's documents up to date.
    void LayoutIfNeeded(EnumSet<dom::LayoutOptions> layoutOptions = {}) noexcept;

    /// @brief Destroys the render trees of the page's documents, as the page enters the back/forward cache.
    void DestroyRenderTrees() noexcept;

    void ForEachDocument(const Func<void(dom::Document &)> &functor) const noexcept;
    /// @brief Calls @p functor for each document that's rendered.
    // FIXME: Documents whose visual updates are suppressed, or that a view transition hides, once there are
    // any.
    void ForEachRenderableDocument(const Func<void(dom::Document &)> &functor) const noexcept;

  private:
    void RunProcessingStep(RenderingUpdateStep step,
                           const Func<void(dom::Document &)> &perDocumentFunction) noexcept;
    void DoAfterUpdateRendering() noexcept;
    void FinalizeRenderingUpdate() noexcept;
    void RenderingUpdateCompleted() noexcept;
  };
}
