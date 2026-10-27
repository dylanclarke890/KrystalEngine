#pragma once

#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/SVG/Animation/SMILTime.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  class SVGElement;
  class SVGSMILElement;
  class SVGSvgElement;

  /// @brief The timeline of an `<svg>` element's animations: it schedules them by target and attribute,
  /// and samples them on a timer every animation frame while any is due.
  class SMILTimeContainer final : public RefCounted<SMILTimeContainer>,
                                  public CanMakeWeakPtr<SMILTimeContainer>
  {
    using ElementAttributePair = Pair<SVGElement *, dom::QualifiedName>;
    using AnimationsVector = List<WeakPtr<SVGSMILElement>>;
    using GroupedAnimationsMap = List<Pair<ElementAttributePair, AnimationsVector>>;

  private:
    MonotonicTime::duration _beginTime {};
    MonotonicTime::duration _pauseTime {};
    MonotonicTime::duration _accumulatedActiveTime {};
    MonotonicTime::duration _resumeTime {};
    MonotonicTime::duration _presetStartTime {};

    bool _documentOrderIndexesDirty {false};
    Timer _timer;
    GroupedAnimationsMap _scheduledAnimations;
    WeakPtr<SVGSvgElement> _ownerSVGElement;

  public:
    KRYS_NODISCARD static Ref<SMILTimeContainer> Create(SVGSvgElement &owner) noexcept
    {
      return AdoptRef(*new SMILTimeContainer(owner));
    }

    void Schedule(SVGSMILElement *, SVGElement *, const dom::QualifiedName &) noexcept;
    void Unschedule(SVGSMILElement *, SVGElement *, const dom::QualifiedName &) noexcept;
    void NotifyIntervalsChanged() noexcept;

    KRYS_NODISCARD Seconds AnimationFrameDelay() const noexcept;

    KRYS_NODISCARD SMILTime Elapsed() const noexcept;

    KRYS_NODISCARD bool IsActive() const noexcept;
    KRYS_NODISCARD bool IsPaused() const noexcept;

    void Begin() noexcept;
    void Pause() noexcept;
    void Resume() noexcept;
    void SetElapsed(SMILTime) noexcept;

    void SetDocumentOrderIndexesDirty() noexcept
    {
      _documentOrderIndexesDirty = true;
    }

  private:
    explicit SMILTimeContainer(SVGSvgElement &owner) noexcept;

    KRYS_NODISCARD bool IsStarted() const noexcept;
    void TimerFired() noexcept;
    void StartTimer(SMILTime elapsed, SMILTime fireTime, SMILTime minimumDelay = 0) noexcept;
    void UpdateAnimations(SMILTime elapsed, bool seekToTime = false) noexcept;

    template <typename TFunction>
    void ProcessScheduledAnimations(const TFunction &callback) noexcept;
    void UpdateDocumentOrderIndexes() noexcept;
    void SortByPriority(AnimationsVector &smilElements, SMILTime elapsed) noexcept;

    KRYS_NODISCARD MonotonicTime::duration LastResumeTime() const noexcept
    {
      return _resumeTime != MonotonicTime::duration {} ? _resumeTime : _beginTime;
    }
  };
}
