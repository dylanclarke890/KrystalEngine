#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
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
    _taskQueue.push_back({source, krys::move(steps)});
  }

  void EventLoop::QueueTask(TaskSource source, dom::Document &document, Steps &&steps) noexcept
  {
    _taskQueue.push_back({source, krys::move(steps), CreateWeakPtr(&document), true});
  }

  namespace
  {
    enum class TaskState : uint8
    {
      Runnable,
      Suspended,
      Dropped
    };

    template <typename TTask>
    KRYS_NODISCARD TaskState StateOf(const TTask &task) noexcept
    {
      if (!task.HasDocument)
      {
        return TaskState::Runnable;
      }
      auto *document = task.Document.get();
      if (!document || document->ActiveDOMObjectsAreStopped())
      {
        return TaskState::Dropped;
      }
      return document->ScheduledTasksAreSuspended() ? TaskState::Suspended : TaskState::Runnable;
    }
  }

  bool EventLoop::HasPendingTasks() const noexcept
  {
    return std::ranges::any_of(_taskQueue,
                               [](const Task &task) { return StateOf(task) == TaskState::Runnable; });
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
    // Let taskQueue be one of the event loop's task queues, containing at least one runnable task; the oldest
    // runnable task is taken. Tasks of documents that went away are dropped.
    for (auto iterator = _taskQueue.begin(); iterator != _taskQueue.end();)
    {
      auto state = StateOf(*iterator);
      if (state == TaskState::Suspended)
      {
        ++iterator;
        continue;
      }
      if (state == TaskState::Dropped)
      {
        iterator = _taskQueue.erase(iterator);
        continue;
      }

      Task task = krys::move(*iterator);
      _taskQueue.erase(iterator);
      task.Steps();
      PerformMicrotaskCheckpoint();
      return true;
    }
    return false;
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
