#include "Krystal.Booey/Page/DOMTimer.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"

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

  // https://html.spec.whatwg.org/#timer-initialisation-steps
  // 4. If the surrounding agent's event loop's currently running task is a task that was created by this
  //    algorithm, then let nestingLevel be the task's timer nesting level. Otherwise, let nestingLevel be 0.
  // 6. If nestingLevel is greater than 5, and timeout is less than 4, then set timeout to 4.
  // (The document's timer nesting level is set while a timer's task runs. Timeouts are clamped only once
  // they're nested more deeply: see IntervalClampedToMinimum.)
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

  // https://html.spec.whatwg.org/#timer-initialisation-steps
  int32 DOMTimer::Install(dom::Document &document, Func<void()> &&action, Duration timeout,
                          Type type) noexcept
  {
    // 1.-1e. If handler is not a Function and previousId was not given: ...
    // (Handlers are C++ functions.)
    // 2. Let thisArg be global if that is a WorkerGlobalScope object; otherwise let thisArg be the
    //    WindowProxy that corresponds to global.
    // (There's no callback this value.)
    // 7. Let initiating script be the active script.
    // (There are no scripts.)
    // 8.-13. (The timer queues its task when it's due: see TimerFired and Fired.)
    Ref<DOMTimer> timer = AdoptRef(*new DOMTimer(document, krys::move(action), timeout, type));

    // 3. If previousId was given, let id be previousId; otherwise, let id be an implementation-defined
    //    integer that is greater than zero and does not already exist in global's map of setTimeout and
    //    setInterval IDs.
    // 14. Set global's map of setTimeout and setInterval IDs[id] to uniqueHandle.
    // (Keep asking for the next id until we're given one that we don't already have.)
    do
    {
      timer->_timeoutId = document.CircularSequentialID();
    } while (!document.AddTimeout(timer->_timeoutId, *timer));

    // A timer installed in a suspended document waits for it to resume.
    if (document.ActiveDOMObjectsAreSuspended())
    {
      timer->Suspend();
    }

    // 15. Return id.
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

    // 12. Let completionStep be an algorithm step which queues a global task on the timer task source given
    //     global to run task.
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

    // 9a.-9c. If id does not exist in global's map of setTimeout and setInterval IDs, or no longer maps to
    //         uniqueHandle, then abort these steps.
    // (A removed timer is stopped and its task does nothing.)
    // 9d.-9f. (Scripting being disabled doesn't stop C++ handlers, and there's no timing info.)
    RefPtr<dom::Document> document = ShareRefPtr(_document.get());
    if (!document || !_action)
    {
      return;
    }

    // 10. Increment nestingLevel by one.
    // 11. Set task's timer nesting level to nestingLevel.
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

      // 9g. If handler is a Function, then invoke handler given arguments and "report", and with callback
      //     this value set to thisArg.
      // (The action may clear the timer, releasing it: run a copy.)
      // 9k. If repeat is true, then perform the timer initialization steps again, given global, handler,
      //     timeout, arguments, true, and id.
      // (A repeating timer keeps firing at its interval.)
      auto action = _action;
      action();
      return;
    }

    // 9l. Otherwise, remove global's map of setTimeout and setInterval IDs[id].
    (void)document->TakeTimeout(_timeoutId);
    // 9g. If handler is a Function, then invoke handler given arguments and "report", and with callback this
    //     value set to thisArg.
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
