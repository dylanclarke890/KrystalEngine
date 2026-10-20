#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Platform/ThreadTimers.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>

namespace krys::boo::html
{
  EventLoop::EventLoop() noexcept = default;

  EventLoop::~EventLoop() noexcept = default;

  EventLoop &EventLoop::Current() noexcept
  {
    static NeverDestroyed<EventLoop> eventLoop;
    return eventLoop;
  }

  void EventLoop::QueueTask(TaskSource source, Steps &&steps) noexcept
  {
    _taskQueue.push({source, krys::move(steps)});
  }

  void EventLoop::QueueMicrotask(Steps &&steps) noexcept
  {
    _microtaskQueue.push(krys::move(steps));
  }

  void EventLoop::PerformMicrotaskCheckpoint() noexcept
  {
    // 1. If the event loop's performing a microtask checkpoint is true, then return.
    if (_performingMicrotaskCheckpoint)
    {
      return;
    }

    // 2. Set the event loop's performing a microtask checkpoint to true.
    _performingMicrotaskCheckpoint = true;

    // 3. While the event loop's microtask queue is not empty: dequeue the oldest microtask and run it.
    while (!_microtaskQueue.empty())
    {
      Steps microtask = krys::move(_microtaskQueue.front());
      _microtaskQueue.pop();
      microtask();
    }

    // NOTE: steps 4-6 (rejected promises, IndexedDB transactions, ClearKeptObjects) need scripting.

    // 7. Set the event loop's performing a microtask checkpoint to false.
    _performingMicrotaskCheckpoint = false;
  }

  bool EventLoop::RunNextTask() noexcept
  {
    if (_taskQueue.empty())
    {
      return false;
    }

    Task task = krys::move(_taskQueue.front());
    _taskQueue.pop();
    task.Steps();

    PerformMicrotaskCheckpoint();
    return true;
  }

  void EventLoop::ScheduleTask(MonotonicTime::duration timeout, TaskSource source, Steps &&steps) noexcept
  {
    auto timer = CreateUnique<Timer>([this, source, steps = krys::move(steps)]() mutable
                                     { QueueTask(source, krys::move(steps)); });
    timer->StartOneShot(timeout);
    _scheduledTasks.push_back(krys::move(timer));
  }

  void EventLoop::RunUntilIdle() noexcept
  {
    PerformMicrotaskCheckpoint();

    while (true)
    {
      bool fired = ThreadTimers::Current().FireTimersDue();
      if (fired)
      {
        PerformMicrotaskCheckpoint();
      }
      bool ranTask = false;
      while (RunNextTask())
      {
        ranTask = true;
      }
      if (!fired && !ranTask)
      {
        break;
      }
    }

    std::erase_if(_scheduledTasks, [](const UniquePtr<Timer> &timer) { return !timer->IsActive(); });
  }

  void EventLoop::Run(MonotonicTime::duration now, Maybe<MonotonicTime::duration> budget) noexcept
  {
    auto start = MonotonicTime::Now();
    auto &timers = ThreadTimers::Current();
    timers.SetCurrentTime(now);

    PerformMicrotaskCheckpoint();
    if (timers.FireTimersDue())
    {
      PerformMicrotaskCheckpoint();
    }

    while (!budget || MonotonicTime::Now() - start < *budget)
    {
      if (!RunNextTask())
      {
        break;
      }
    }

    std::erase_if(_scheduledTasks, [](const UniquePtr<Timer> &timer) { return !timer->IsActive(); });
  }

  MonotonicTime::duration EventLoop::Now() const noexcept
  {
    return ThreadTimers::Current().CurrentTime();
  }

  Maybe<MonotonicTime::duration> EventLoop::NextWakeTime() const noexcept
  {
    if (HasPendingTasks() || HasPendingMicrotasks())
    {
      return Now();
    }
    return ThreadTimers::Current().NextFireTime();
  }
}
