#include "Krystal.Booey/Platform/ThreadTimers.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <utility>

namespace krys::boo
{
  ThreadTimers &ThreadTimers::Current() noexcept
  {
    static NeverDestroyed<ThreadTimers> timers;
    return timers;
  }

  void ThreadTimers::SetCurrentTime(MonotonicTime::duration now) noexcept
  {
    if (now > _currentTime)
    {
      _currentTime = now;
    }
  }

  bool ThreadTimers::FireTimersDue() noexcept
  {
    // Do a re-entrancy check.
    if (_firingTimers)
    {
      return false;
    }

    _firingTimers = true;
    bool fired = false;

    auto fireTime = _currentTime;
    // Timers started from here on fire next time: the host's time doesn't move while they fire, so a timer
    // restarted with no delay would otherwise be due again at once.
    auto lastInsertionOrder = _currentHeapInsertionOrder;

    while (!_timerHeap.empty())
    {
      TimerBase &timer = *_timerHeap.front();
      if (timer._nextFireTime > fireTime || timer._heapInsertionOrder >= lastInsertionOrder)
      {
        break;
      }

      auto interval = timer._repeatInterval;
      if (interval > MonotonicTime::duration::zero())
      {
        SetNextFireTime(timer, fireTime + interval, true);
      }
      else
      {
        SetNextFireTime(timer, {}, false);
      }

      // Once the timer has been fired, it may be deleted, so do nothing else with it after this point.
      timer.Fired();
      fired = true;
    }

    _firingTimers = false;
    return fired;
  }

  Maybe<MonotonicTime::duration> ThreadTimers::NextFireTime() const noexcept
  {
    if (_timerHeap.empty())
    {
      return null;
    }
    return _timerHeap.front()->_nextFireTime;
  }

  void ThreadTimers::SetNextFireTime(TimerBase &timer, MonotonicTime::duration fireTime, bool active) noexcept
  {
    if (timer.IsActive())
    {
      HeapRemove(timer);
    }
    timer._nextFireTime = fireTime;
    if (!active)
    {
      return;
    }

    timer._heapInsertionOrder = _currentHeapInsertionOrder++;
    timer._heapIndex = _timerHeap.size();
    _timerHeap.push_back(&timer);
    HeapUp(timer._heapIndex);
  }

  bool ThreadTimers::FiresBefore(const TimerBase &a, const TimerBase &b) noexcept
  {
    if (a._nextFireTime != b._nextFireTime)
    {
      return a._nextFireTime < b._nextFireTime;
    }
    return a._heapInsertionOrder < b._heapInsertionOrder;
  }

  void ThreadTimers::HeapSwap(size_t a, size_t b) noexcept
  {
    std::swap(_timerHeap[a], _timerHeap[b]);
    _timerHeap[a]->_heapIndex = a;
    _timerHeap[b]->_heapIndex = b;
  }

  void ThreadTimers::HeapUp(size_t index) noexcept
  {
    while (index > 0)
    {
      size_t parent = (index - 1) / 2;
      if (!FiresBefore(*_timerHeap[index], *_timerHeap[parent]))
      {
        return;
      }
      HeapSwap(index, parent);
      index = parent;
    }
  }

  void ThreadTimers::HeapDown(size_t index) noexcept
  {
    size_t size = _timerHeap.size();
    while (true)
    {
      size_t first = index;
      for (size_t child : {index * 2 + 1, index * 2 + 2})
      {
        if (child < size && FiresBefore(*_timerHeap[child], *_timerHeap[first]))
        {
          first = child;
        }
      }
      if (first == index)
      {
        return;
      }
      HeapSwap(index, first);
      index = first;
    }
  }

  void ThreadTimers::HeapRemove(TimerBase &timer) noexcept
  {
    size_t index = timer._heapIndex;
    krys_debug_assert(index < _timerHeap.size() && _timerHeap[index] == &timer);
    size_t last = _timerHeap.size() - 1;
    if (index != last)
    {
      HeapSwap(index, last);
    }
    _timerHeap.pop_back();
    timer._heapIndex = TimerBase::NotInHeap;
    if (index < _timerHeap.size())
    {
      HeapUp(index);
      HeapDown(index);
    }
  }
}
