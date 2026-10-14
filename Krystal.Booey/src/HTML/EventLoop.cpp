#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::html
{
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

  void EventLoop::RunUntilIdle() noexcept
  {
    PerformMicrotaskCheckpoint();

    while (RunNextTask())
    {
    }
  }
}
