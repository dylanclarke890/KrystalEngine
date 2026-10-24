#pragma once

#include "Krystal.Booey/Page/ResizeObservation.hpp"
#include "Krystal.Booey/Page/ResizeObserverCallback.hpp"
#include "Krystal.Booey/Page/ResizeObserverOptions.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <limits>

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo
{
  /// @brief The resize observers observing an element.
  struct ResizeObserverData
  {
    List<WeakPtr<ResizeObserver>> Observers;
  };

  /// @brief Reports changes to the sizes of the elements it observes, in the rendering update after layout.
  /// Whoever made it keeps it: it stops observing when it's destroyed.
  /// @see https://drafts.csswg.org/resize-observer/#resize-observer-interface
  class ResizeObserver : public RefCounted<ResizeObserver>, public CanMakeWeakPtr<ResizeObserver>
  {
    KRYS_NON_COPY_MOVABLE(ResizeObserver);

  private:
    WeakPtr<dom::Document> _document;
    ResizeObserverCallback _callback;
    // Whether the observer is the user agent's own (the document's, for the last remembered sizes), which the
    // document gathers and delivers itself.
    bool _isNative {false};
    bool _hasSkippedObservations {false};
    List<Ref<ResizeObservation>> _observations;
    List<Ref<ResizeObservation>> _activeObservations;
    List<Ref<dom::Element>> _activeObservationTargets;
    // An observed element is reported at least once, so it's kept until its first observation.
    List<Ref<dom::Element>> _targetsWaitingForFirstObservation;

    ResizeObserver(dom::Document &document, ResizeObserverCallback &&callback, bool isNative) noexcept;

  public:
    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserver-resizeobserver
    KRYS_NODISCARD static Ref<ResizeObserver> Create(dom::Document &document,
                                                     ResizeObserverCallback &&callback) noexcept;
    /// @brief An observer for the user agent's own use, gathered and delivered by the document rather than
    /// with the document's other observers.
    KRYS_NODISCARD static Ref<ResizeObserver>
      CreateNativeObserver(dom::Document &document, ResizeObserverCallback &&callback) noexcept;
    ~ResizeObserver() noexcept;

    KRYS_NODISCARD bool HasObservations() const noexcept
    {
      return !_observations.empty();
    }
    KRYS_NODISCARD bool HasActiveObservations() const noexcept
    {
      return !_activeObservations.empty();
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserver-observe
    void Observe(dom::Element &target, const ResizeObserverOptions &options = {}) noexcept;
    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserver-unobserve
    void Unobserve(dom::Element &target) noexcept;
    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserver-disconnect
    void Disconnect() noexcept;

    /// @brief An observed element is being destroyed.
    void TargetDestroyed(dom::Element &target) noexcept;

    KRYS_NODISCARD static size_t MaxElementDepth() noexcept
    {
      return std::numeric_limits<size_t>::max();
    }

    /// @brief Makes the observations whose targets changed size and are deeper than @p deeperThan active,
    /// noting whether any shallower ones were skipped.
    /// @return The depth of the shallowest active observation, or `MaxElementDepth()` if there's none.
    /// @see https://drafts.csswg.org/resize-observer/#gather-active-observations-h
    KRYS_NODISCARD size_t GatherObservations(size_t deeperThan) noexcept;
    /// @brief Calls the callback with the active observations' entries.
    /// @see https://drafts.csswg.org/resize-observer/#broadcast-active-resize-observations
    void DeliverObservations() noexcept;

    KRYS_NODISCARD bool HasSkippedObservations() const noexcept
    {
      return _hasSkippedObservations;
    }
    void SetHasSkippedObservations(bool skipped) noexcept
    {
      _hasSkippedObservations = skipped;
    }

    /// @brief Forgets the sizes last reported for @p target, so it's reported again.
    void ResetObservationSize(dom::Element &target) noexcept;

  private:
    bool RemoveTarget(dom::Element &target) noexcept;
    void RemoveAllTargets() noexcept;
    bool RemoveObservation(const dom::Element &target) noexcept;
    void ObserveInternal(dom::Element &target, ResizeObserverBoxOptions boxOptions) noexcept;
  };
}
