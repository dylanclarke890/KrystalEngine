#include "Krystal.Booey/Page/DOMTimer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr DOMTimer::Duration MinIntervalForOneShotTimers {0};
    constexpr DOMTimer::Duration MinIntervalForRepeatingTimers = std::chrono::milliseconds(1);
    constexpr int32 MaxTimerNestingLevel = 10;
    constexpr int32 MaxTimerNestingLevelForOneShotTimers = 10;
    constexpr int32 MaxTimerNestingLevelForRepeatingTimers = 5;

    /// @brief Sets the document's timer nesting level while a timer's action runs.
    class DOMTimerFireState
    {
      KRYS_NON_COPY_MOVABLE(DOMTimerFireState);

    private:
      Ref<dom::Document> _document;

    public:
      DOMTimerFireState(dom::Document &document, int32 nestingLevel) noexcept : _document(ShareRef(document))
      {
        _document->SetTimerNestingLevel(nestingLevel);
      }

      ~DOMTimerFireState() noexcept
      {
        _document->SetTimerNestingLevel(0);
      }
    };
  }

  DOMTimer::DOMTimer(dom::Document &document, Func<void()> &&action, Duration interval, Type type) noexcept
      : _document(CreateWeakPtr(&document)), _nestingLevel(document.TimerNestingLevel()),
        _timer([this] { TimerFired(); }), _action(krys::move(action)), _originalInterval(interval),
        _oneShot(type == Type::SingleShot)
  {
    _currentTimerInterval = IntervalClampedToMinimum();
    _hasReachedMaxNestingLevel = _nestingLevel >= (_oneShot ? MaxTimerNestingLevelForOneShotTimers
                                                            : MaxTimerNestingLevelForRepeatingTimers);
    if (_oneShot)
    {
      _timer.StartOneShot(_currentTimerInterval);
    }
    else
    {
      _timer.Start(_currentTimerInterval, _currentTimerInterval);
    }
  }

  DOMTimer::~DOMTimer() noexcept = default;

  int32 DOMTimer::Install(dom::Document &document, Func<void()> &&action, Duration timeout,
                          Type type) noexcept
  {
    Ref<DOMTimer> timer = AdoptRef(*new DOMTimer(document, krys::move(action), timeout, type));

    // Keep asking for the next id until we're given one that we don't already have.
    do
    {
      timer->_timeoutId = document.CircularSequentialID();
    } while (!document.AddTimeout(timer->_timeoutId, *timer));

    // A timer installed in a suspended document waits for it to resume.
    if (document.ActiveDOMObjectsAreSuspended())
    {
      timer->Suspend();
    }

    return timer->_timeoutId;
  }

  void DOMTimer::RemoveById(dom::Document &document, int32 timeoutId) noexcept
  {
    // Timeout IDs are positive.
    if (timeoutId <= 0)
    {
      return;
    }

    if (RefPtr<DOMTimer> timer = document.TakeTimeout(timeoutId))
    {
      timer->_timer.Stop();
    }
  }

  void DOMTimer::Stop() noexcept
  {
    // Release the action, which may hold references back to the document.
    _timer.Stop();
    _action = nullptr;
  }

  void DOMTimer::Suspend() noexcept
  {
    if (_suspendedNextFireInterval || !_timer.IsActive())
    {
      return;
    }
    _suspendedNextFireInterval = _timer.NextFireInterval();
    _timer.Stop();
  }

  void DOMTimer::Resume() noexcept
  {
    if (!_suspendedNextFireInterval)
    {
      return;
    }
    _timer.Start(*_suspendedNextFireInterval, _oneShot ? Duration {} : _currentTimerInterval);
    _suspendedNextFireInterval = null;
  }

  void DOMTimer::TimerFired() noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    if (!document)
    {
      return;
    }

    // Run steps after a timeout: queue a global task on the timer task source.
    html::EventLoop::Current().QueueTask(html::TaskSource::Timer, *document,
                                         [weakThis = CreateWeakPtr(this)]
                                         {
                                           if (RefPtr<DOMTimer> protectedThis = ShareRefPtr(weakThis.get()))
                                           {
                                             protectedThis->Fired();
                                           }
                                         });
  }

  void DOMTimer::Fired() noexcept
  {
    // Retain this: if the timer is cancelled while this function is on the stack (implicitly and always for
    // one-shot timers, or if RemoveById is called on itself from within an interval timer fire), wait until
    // the end of this function to delete the DOMTimer.
    Ref<DOMTimer> protectedThis = ShareRef(*this);

    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    if (!document || !_action)
    {
      return;
    }

    DOMTimerFireState fireState(*document, std::min(_nestingLevel + 1, MaxTimerNestingLevel));

    // Simple case for repeating timers.
    if (!_oneShot)
    {
      if (_nestingLevel < MaxTimerNestingLevel)
      {
        ++_nestingLevel;
        _hasReachedMaxNestingLevel = _nestingLevel >= MaxTimerNestingLevelForRepeatingTimers;
        UpdateTimerIntervalIfNecessary();
      }

      // The action may clear the timer, releasing it: run a copy.
      auto action = _action;
      action();
      return;
    }

    (void)document->TakeTimeout(_timeoutId);
    auto action = krys::move(_action);
    action();
  }

  void DOMTimer::UpdateTimerIntervalIfNecessary() noexcept
  {
    krys_debug_assert(_nestingLevel <= MaxTimerNestingLevel);

    auto previousInterval = _currentTimerInterval;
    _currentTimerInterval = IntervalClampedToMinimum();
    if (previousInterval == _currentTimerInterval || !_timer.IsActive())
    {
      return;
    }

    auto nextFireInterval = _timer.NextFireInterval() + (_currentTimerInterval - previousInterval);
    if (_oneShot)
    {
      _timer.StartOneShot(nextFireInterval);
    }
    else
    {
      _timer.Start(nextFireInterval, _currentTimerInterval);
    }
  }

  DOMTimer::Duration DOMTimer::IntervalClampedToMinimum() const noexcept
  {
    krys_debug_assert(_nestingLevel <= MaxTimerNestingLevel);

    Duration interval =
      std::max(_oneShot ? MinIntervalForOneShotTimers : MinIntervalForRepeatingTimers, _originalInterval);

    // Only clamp timers nested deeply enough.
    if (_nestingLevel
        < (_oneShot ? MaxTimerNestingLevelForOneShotTimers : MaxTimerNestingLevelForRepeatingTimers))
    {
      return interval;
    }

    // FIXME: The per-timer throttle for timers that make no changes the user sees, once there's throttling.
    return std::max(interval, DefaultMinimumInterval());
  }
}
