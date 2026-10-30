#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Page/IntersectionObserverCallback.hpp"
#include "Krystal.Booey/Page/IntersectionObserverMarginBox.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
}

namespace krys::boo
{
  class IntersectionObserver;
  class IntersectionObserverEntry;
  class LocalFrame;
  class LocalFrameView;

  struct IntersectionObserverRegistration
  {
    WeakPtr<IntersectionObserver> Observer;
    Maybe<size_t> PreviousThresholdIndex;
  };

  struct IntersectionObserverData
  {
    // IntersectionObservers for which the node that owns this IntersectionObserverData is the root.
    List<WeakPtr<IntersectionObserver>> Observers;

    // IntersectionObserverRegistrations for which the node that owns this IntersectionObserverData is the
    // target.
    List<IntersectionObserverRegistration> Registrations;
  };

  enum class IncludeObscuredInsets : bool
  {
    No,
    Yes
  };

  /// @brief Reports changes in how much of the elements it observes intersect its root (an element, a
  /// document, or the main frame's viewport), in the rendering update. Whoever made it keeps it: it stops
  /// observing when it's destroyed.
  /// @see https://w3c.github.io/IntersectionObserver/#intersection-observer-interface
  class IntersectionObserver : public RefCounted<IntersectionObserver>,
                               public CanMakeWeakPtr<IntersectionObserver>
  {
    KRYS_NON_COPY_MOVABLE(IntersectionObserver);

  public:
    /// @see https://w3c.github.io/IntersectionObserver/#dictdef-intersectionobserverinit
    struct Init
    {
      Maybe<Variant<RefPtr<dom::Element>, RefPtr<dom::Document>>> Root;
      dom::DOMString RootMargin {u8"0px"};
      dom::DOMString ScrollMargin {u8"0px"};
      Variant<double, List<double>> Threshold {0.0};
    };

    enum class NeedNotify : bool
    {
      No,
      Yes
    };

  private:
    struct IntersectionObservationState
    {
      FloatRect RootBounds;
      Maybe<FloatRect> AbsoluteIntersectionRect; // Only computed if intersecting.
      Maybe<FloatRect> AbsoluteTargetRect;       // Only computed if first observation, or intersecting.
      Maybe<FloatRect> AbsoluteRootBounds;       // Only computed if observationChanged.
      float IntersectionRatio {0};
      size_t ThresholdIndex {0};
      bool CanComputeIntersection {false};
      bool IsIntersecting {false};
      bool ObservationChanged {false};
    };

    enum class ApplyRootMargin : bool
    {
      No,
      Yes
    };

    WeakPtr<dom::Document> _document;
    WeakPtr<dom::Document> _implicitRootDocument;
    WeakPtr<dom::ContainerNode> _root;
    IntersectionObserverMarginBox _rootMargin;
    IntersectionObserverMarginBox _scrollMargin;
    List<double> _thresholds;
    IntersectionObserverCallback _callback;
    List<WeakPtr<dom::Element>> _observationTargets;
    List<Ref<IntersectionObserverEntry>> _queuedEntries;
    // An observed element is reported at least once, so it's kept until its first observation.
    List<Ref<dom::Element>> _targetsWaitingForFirstObservation;
    IncludeObscuredInsets _includeObscuredInsets {IncludeObscuredInsets::No};

    IntersectionObserver(dom::Document &document, IntersectionObserverCallback &&callback,
                         dom::ContainerNode *root, IntersectionObserverMarginBox &&parsedRootMargin,
                         IntersectionObserverMarginBox &&parsedScrollMargin, List<double> &&thresholds,
                         IncludeObscuredInsets includeObscuredInsets) noexcept;

  public:
    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-intersectionobserver
    KRYS_NODISCARD static dom::ExceptionOr<Ref<IntersectionObserver>>
      Create(dom::Document &document, IntersectionObserverCallback &&callback, Init &&init = {},
             IncludeObscuredInsets includeObscuredInsets = IncludeObscuredInsets::No) noexcept;

    ~IntersectionObserver() noexcept;

    /// @brief The document whose rendering updates compute the observations: the root's, or the main
    /// frame's for an implicit root.
    KRYS_NODISCARD dom::Document *TrackingDocument() const noexcept;

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-root
    KRYS_NODISCARD dom::ContainerNode *Root() const noexcept
    {
      return _root.get();
    }
    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-rootmargin
    KRYS_NODISCARD dom::DOMString RootMargin() const noexcept;
    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-scrollmargin
    KRYS_NODISCARD dom::DOMString ScrollMargin() const noexcept;
    KRYS_NODISCARD const IntersectionObserverMarginBox &RootMarginBox() const noexcept
    {
      return _rootMargin;
    }
    KRYS_NODISCARD const IntersectionObserverMarginBox &ScrollMarginBox() const noexcept
    {
      return _scrollMargin;
    }
    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-thresholds
    KRYS_NODISCARD const List<double> &Thresholds() const noexcept
    {
      return _thresholds;
    }
    KRYS_NODISCARD const List<WeakPtr<dom::Element>> &ObservationTargets() const noexcept
    {
      return _observationTargets;
    }
    KRYS_NODISCARD bool HasObservationTargets() const noexcept
    {
      return !_observationTargets.empty();
    }
    KRYS_NODISCARD bool IsObserving(const dom::Element &element) const noexcept;

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-observe
    void Observe(dom::Element &target) noexcept;
    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-unobserve
    void Unobserve(dom::Element &target) noexcept;
    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-disconnect
    void Disconnect() noexcept;

    /// @see https://w3c.github.io/IntersectionObserver/#dom-intersectionobserver-takerecords
    KRYS_NODISCARD List<Ref<IntersectionObserverEntry>> TakeRecords() noexcept;

    /// @brief An observed element is being destroyed.
    void TargetDestroyed(dom::Element &target) noexcept;
    /// @brief The root is being destroyed.
    void RootDestroyed() noexcept;

    /// @brief Queues an entry for each target whose intersection with the root crossed a threshold, or that
    /// hasn't been observed yet.
    /// @see https://w3c.github.io/IntersectionObserver/#update-intersection-observations-algo
    KRYS_NODISCARD NeedNotify UpdateObservations(const LocalFrame &hostFrame) noexcept;

    KRYS_NODISCARD Maybe<double> NowTimestamp() const noexcept;

    void AppendQueuedEntry(Ref<IntersectionObserverEntry> &&entry) noexcept;
    /// @brief Calls the callback with the queued entries.
    /// @see https://w3c.github.io/IntersectionObserver/#notify-intersection-observers-algo
    void Notify() noexcept;

  private:
    bool RemoveTargetRegistration(dom::Element &target) noexcept;
    void RemoveAllTargets() noexcept;

    KRYS_NODISCARD IntersectionObservationState ComputeIntersectionState(
      const IntersectionObserverRegistration &registration, LocalFrameView &hostFrameView,
      dom::Element &target, ApplyRootMargin applyRootMargin) const noexcept;
  };
}
