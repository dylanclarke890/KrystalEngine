#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterRenderingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class GraphicsContext;
  class LocalFrame;

  /// @brief The steps of the rendering update that something asked for.
  // FIXME: The steps for the features Booey doesn't have yet: revealing documents, fullscreen, view
  // transitions, intersection and resize observations, content relevancy, paint and event timing, video
  // frame callbacks, caret animations, validation messages and the layer and scrolling tree updates.
  enum class RenderingUpdateStep : uint8
  {
    FlushAutofocusCandidates,
    Resize,
    Scroll,
    MediaQueryEvaluation,
    Animations,
    AnimationFrameCallbacks,
    FocusFixup,
    CursorUpdate,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::RenderingUpdateStep, 8uz);

namespace krys::boo
{
  /// @brief A page: its main frame, and the rendering update that brings the page's documents up to date
  /// before the host paints them (the specification's "update the rendering"). Something that changes what
  /// the page shows schedules a rendering update; the host runs it at its next rendering opportunity.
  /// @see https://html.spec.whatwg.org/#update-the-rendering
  // FIXME: The rest of the page: settings, the focus controller, the chrome, the back/forward list, page
  // overlays, frames other than the main frame.
  class Page final : public RefCounted<Page>, public CanMakeWeakPtr<Page>
  {
    KRYS_NON_COPY_MOVABLE(Page);

  private:
    RefPtr<LocalFrame> _mainFrame;
    /// @brief The steps of the rendering update in progress that haven't run yet.
    EnumSet<RenderingUpdateStep> _renderingUpdateRemainingSteps;
    /// @brief Steps asked for during a rendering update after they had run, for the next one.
    EnumSet<RenderingUpdateStep> _unfulfilledRequestedSteps;
    uint32 _renderingUpdateDepth {0};
    bool _renderingUpdateIsScheduled {false};
    MonotonicTime::duration _lastRenderingUpdateTimestamp {};

    Page() noexcept;

  public:
    ~Page() noexcept;

    /// @brief A page with a main frame, which has no view or document yet.
    KRYS_NODISCARD static Ref<Page> Create() noexcept;

    KRYS_NODISCARD LocalFrame &MainFrame() const noexcept
    {
      return *_mainFrame;
    }

    /// @brief Asks for a rendering update at the next rendering opportunity, which runs (at least) the
    /// @p requestedSteps.
    void ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps = {}) noexcept;
    /// @brief How the page's filters are applied when drawn into @p context: through the graphics context (there's
    /// no software or accelerated filter rendering).
    KRYS_NODISCARD EnumSet<FilterRenderingMode> PreferredFilterRenderingModes(const GraphicsContext &context) const noexcept;

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
    /// with layout before and after the steps that read it.
    void UpdateRendering() noexcept;

    /// @brief Brings the style and layout of the page's documents up to date.
    void LayoutIfNeeded() noexcept;

    void ForEachDocument(const Func<void(dom::Document &)> &functor) const noexcept;
    /// @brief Calls @p functor for each document that's rendered.
    // FIXME: Documents whose visual updates are suppressed, or that a view transition hides, once there are
    // any.
    void ForEachRenderableDocument(const Func<void(dom::Document &)> &functor) const noexcept;

  private:
    void RunProcessingStep(RenderingUpdateStep step,
                           const Func<void(dom::Document &)> &perDocumentFunction) noexcept;
    void DoAfterUpdateRendering() noexcept;
    void RenderingUpdateCompleted() noexcept;
  };
}
