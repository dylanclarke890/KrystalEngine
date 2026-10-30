#include "Krystal.Booey/SVG/Animation/SMILTimeContainer.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/SVG/Animation/SVGSMILElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include <algorithm>
#include <chrono>

namespace krys::boo::svg
{
  namespace
  {
    constexpr Seconds SMILAnimationFrameDelay {1.0 / 60.};

    KRYS_NODISCARD SMILTime ToSMILTime(MonotonicTime::duration duration) noexcept
    {
      return std::chrono::duration_cast<Seconds>(duration).count();
    }

    KRYS_NODISCARD MonotonicTime::duration ToDuration(SMILTime time) noexcept
    {
      return std::chrono::duration_cast<MonotonicTime::duration>(Seconds(time.Value()));
    }
  }

  SMILTimeContainer::SMILTimeContainer(SVGSvgElement &owner) noexcept
      : _timer(*this, &SMILTimeContainer::TimerFired), _ownerSVGElement(CreateWeakPtr(&owner))
  {
  }

  void SMILTimeContainer::Schedule(SVGSMILElement *animation, SVGElement *target,
                                   const dom::QualifiedName &attributeName) noexcept
  {
    krys_debug_assert(animation->TimeContainer() == this);
    krys_debug_assert(target);
    krys_debug_assert(animation->HasValidAttributeName());

    ElementAttributePair key(target, attributeName);
    auto iterator =
      std::ranges::find_if(_scheduledAnimations, [&](const auto &entry) { return entry.first == key; });
    if (iterator == _scheduledAnimations.end())
    {
      _scheduledAnimations.emplace_back(key, AnimationsVector());
      iterator = _scheduledAnimations.end() - 1;
    }
    auto &animations = iterator->second;
    krys_debug_assert(
      std::ranges::none_of(animations, [&](const auto &item) { return item.get() == animation; }));
    animations.push_back(CreateWeakPtr(animation));

    SMILTime nextFireTime = animation->NextProgressTime();
    if (nextFireTime.IsFinite())
    {
      NotifyIntervalsChanged();
    }
  }

  void SMILTimeContainer::Unschedule(SVGSMILElement *animation, SVGElement *target,
                                     const dom::QualifiedName &attributeName) noexcept
  {
    krys_debug_assert(animation->TimeContainer() == this);

    ElementAttributePair key(target, attributeName);
    auto iterator =
      std::ranges::find_if(_scheduledAnimations, [&](const auto &entry) { return entry.first == key; });
    krys_debug_assert(iterator != _scheduledAnimations.end());
    auto &animations = iterator->second;
    auto animationIterator =
      std::ranges::find_if(animations, [&](const auto &item) { return item.get() == animation; });
    krys_debug_assert(animationIterator != animations.end());
    animations.erase(animationIterator);
  }

  void SMILTimeContainer::NotifyIntervalsChanged() noexcept
  {
    // Schedule updateAnimations() to be called asynchronously so multiple intervals
    // can change with updateAnimations() only called once at the end.
    StartTimer(Elapsed(), 0);
  }

  Seconds SMILTimeContainer::AnimationFrameDelay() const noexcept
  {
    // FIXME: The throttled delay (a 30th of a second) in low power mode or under thermal mitigation, once the
    // page knows about those.
    return SMILAnimationFrameDelay;
  }

  SMILTime SMILTimeContainer::Elapsed() const noexcept
  {
    if (_beginTime == MonotonicTime::duration {})
    {
      return 0;
    }
    if (IsPaused())
    {
      return ToSMILTime(_accumulatedActiveTime);
    }
    return ToSMILTime(MonotonicTime::Now() + _accumulatedActiveTime - LastResumeTime());
  }

  bool SMILTimeContainer::IsActive() const noexcept
  {
    return _beginTime != MonotonicTime::duration {} && !IsPaused();
  }

  bool SMILTimeContainer::IsPaused() const noexcept
  {
    return _pauseTime != MonotonicTime::duration {};
  }

  bool SMILTimeContainer::IsStarted() const noexcept
  {
    return _beginTime != MonotonicTime::duration {};
  }

  void SMILTimeContainer::Begin() noexcept
  {
    if (IsStarted())
    {
      return;
    }

    auto now = MonotonicTime::Now();

    // If '_presetStartTime' is set, the timeline was modified via setElapsed() before the document began.
    // In this case pass on 'seekToTime=true' to updateAnimations().
    _beginTime = _resumeTime = now - _presetStartTime;
    UpdateAnimations(ToSMILTime(_presetStartTime), _presetStartTime != MonotonicTime::duration {});
    _presetStartTime = {};

    if (_pauseTime != MonotonicTime::duration {})
    {
      _pauseTime = now;
      _timer.Stop();
    }
  }

  void SMILTimeContainer::Pause() noexcept
  {
    krys_debug_assert(!IsPaused());

    _pauseTime = MonotonicTime::Now();
    if (_beginTime != MonotonicTime::duration {})
    {
      _accumulatedActiveTime += _pauseTime - LastResumeTime();
      _timer.Stop();
    }
  }

  void SMILTimeContainer::Resume() noexcept
  {
    krys_debug_assert(IsPaused());

    _resumeTime = MonotonicTime::Now();
    _pauseTime = {};
    StartTimer(Elapsed(), 0);
  }

  void SMILTimeContainer::SetElapsed(SMILTime time) noexcept
  {
    // If the document didn't begin yet, record a new start time, we'll seek to once its possible.
    if (_beginTime == MonotonicTime::duration {})
    {
      _presetStartTime = ToDuration(time);
      return;
    }

    _timer.Stop();

    auto now = MonotonicTime::Now();
    _beginTime = now - ToDuration(time);

    _resumeTime = {};
    if (_pauseTime != MonotonicTime::duration {})
    {
      _pauseTime = now;
      _accumulatedActiveTime = ToDuration(time);
    }
    else
    {
      _accumulatedActiveTime = {};
    }

    ProcessScheduledAnimations([](SVGSMILElement &animation) { animation.Reset(); });

    UpdateAnimations(time, true);
  }

  void SMILTimeContainer::StartTimer(SMILTime elapsed, SMILTime fireTime, SMILTime minimumDelay) noexcept
  {
    if (_beginTime == MonotonicTime::duration {} || IsPaused())
    {
      return;
    }

    if (!fireTime.IsFinite())
    {
      return;
    }

    SMILTime delay = std::max(fireTime - elapsed, minimumDelay);
    _timer.StartOneShot(ToDuration(delay));
  }

  void SMILTimeContainer::TimerFired() noexcept
  {
    krys_debug_assert(_beginTime != MonotonicTime::duration {});
    krys_debug_assert(_pauseTime == MonotonicTime::duration {});
    UpdateAnimations(Elapsed());
  }

  void SMILTimeContainer::UpdateDocumentOrderIndexes() noexcept
  {
    uint32 timingElementCount = 0;

    Ref owner = ShareRef(*_ownerSVGElement);
    for (auto *node = owner->FirstChild(); node; node = dom::TreeQueries::Next(*node, owner.get()))
    {
      if (auto *smilElement = DynamicDowncast<SVGSMILElement>(*node))
      {
        smilElement->SetDocumentOrderIndex(timingElementCount++);
      }
    }

    _documentOrderIndexesDirty = false;
  }

  void SMILTimeContainer::SortByPriority(AnimationsVector &animations, SMILTime elapsed) noexcept
  {
    if (_documentOrderIndexesDirty)
    {
      UpdateDocumentOrderIndexes();
    }
    std::ranges::sort(animations,
                      [elapsed](const auto &a, const auto &b)
                      {
                        // FIXME: This should also consider possible timing relations between the elements.
                        SMILTime aBegin = a->IntervalBegin();
                        SMILTime bBegin = b->IntervalBegin();
                        // Frozen elements need to be prioritized based on their previous interval.
                        aBegin = a->IsFrozen() && elapsed < aBegin ? a->PreviousIntervalBegin() : aBegin;
                        bBegin = b->IsFrozen() && elapsed < bBegin ? b->PreviousIntervalBegin() : bBegin;
                        if (aBegin == bBegin)
                        {
                          return a->DocumentOrderIndex() < b->DocumentOrderIndex();
                        }
                        return aBegin < bBegin;
                      });
  }

  template <typename TFunction>
  void SMILTimeContainer::ProcessScheduledAnimations(const TFunction &callback) noexcept
  {
    auto scheduledAnimations = _scheduledAnimations;
    for (auto &[key, animations] : scheduledAnimations)
    {
      for (auto &weakAnimation : animations)
      {
        if (RefPtr animation = ShareRefPtr(weakAnimation.get()))
        {
          callback(*animation);
        }
      }
    }
  }

  void SMILTimeContainer::UpdateAnimations(SMILTime elapsed, bool seekToTime) noexcept
  {
    // FIXME: Don't mutate the DOM while updating the animations (EventQueueScope), once there's a scoped
    // event queue.

    ProcessScheduledAnimations(
      [](SVGSMILElement &animation)
      {
        if (!animation.HasConditionsConnected())
        {
          animation.ConnectConditions();
        }
      });

    List<Ref<SVGSMILElement>> animationsToApply;
    SMILTime earliestFireTime = SMILTime::Unresolved();

    auto scheduledAnimations = _scheduledAnimations;
    for (auto &[key, animations] : scheduledAnimations)
    {
      std::erase_if(animations, [](const auto &weakAnimation) { return !weakAnimation; });

      // Sort according to priority. Elements with later begin time have higher priority.
      // In case of a tie, document order decides.
      // FIXME: This should also consider timing relationships between the elements. Dependents
      // have higher priority.
      SortByPriority(animations, elapsed);

      RefPtr<SVGSMILElement> firstAnimation;
      for (auto &weakAnimation : animations)
      {
        Ref animation = ShareRef(*weakAnimation);
        krys_debug_assert(animation->TimeContainer() == this);
        krys_debug_assert(animation->TargetElement());
        krys_debug_assert(animation->HasValidAttributeName());

        // Results are accumulated to the first animation that animates and contributes to a particular
        // element/attribute pair.
        if (!firstAnimation)
        {
          if (!animation->HasValidAttributeType())
          {
            return;
          }
          firstAnimation = animation;
        }

        // This will calculate the contribution from the animation and add it to the resultsElement.
        if (!animation->Progress(elapsed, *firstAnimation, seekToTime) && firstAnimation == animation.get())
        {
          firstAnimation = nullptr;
        }

        SMILTime nextFireTime = animation->NextProgressTime();
        if (nextFireTime.IsFinite())
        {
          earliestFireTime = std::min(nextFireTime, earliestFireTime);
        }
      }

      if (firstAnimation)
      {
        animationsToApply.push_back(ShareRef(*firstAnimation));
      }
    }

    // Apply results to target elements.
    for (auto &animation : animationsToApply)
    {
      animation->ApplyResultsToTarget();
    }

    StartTimer(elapsed, earliestFireTime, AnimationFrameDelay().count());
  }
}
