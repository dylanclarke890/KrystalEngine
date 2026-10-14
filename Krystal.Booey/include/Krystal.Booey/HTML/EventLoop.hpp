#pragma once

#include "Krystal.Booey/HTML/Enums/TaskSource.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Queue.hpp"

namespace krys::boo::html
{
  /// @brief A minimal event loop: one queue of tasks and a microtask queue, run by the host.
  /// @note There's no rendering step, no task document (every document is fully active) and no
  /// scripting, so a microtask checkpoint only runs microtasks.
  /// @see https://html.spec.whatwg.org/#event-loops
  class EventLoop
  {
    KRYS_NON_COPY_MOVABLE(EventLoop);

  public:
    using Steps = Func<void()>;

  private:
    /// @see https://html.spec.whatwg.org/#concept-task
    struct Task
    {
      TaskSource Source;
      EventLoop::Steps Steps;
    };

    // NOTE: the spec allows one task queue per source; tasks from every source share one queue here, which
    // keeps tasks from the same source in order.
    Queue<Task> _taskQueue;

    /// @see https://html.spec.whatwg.org/#microtask-queue
    Queue<Steps> _microtaskQueue;

    /// @see https://html.spec.whatwg.org/#performing-a-microtask-checkpoint
    bool _performingMicrotaskCheckpoint {false};

  public:
    EventLoop() noexcept = default;

    /// @brief The event loop of the surrounding agent.
    /// FIXME: process-wide until there are agents.
    KRYS_NODISCARD static EventLoop &Current() noexcept;

    /// @see https://html.spec.whatwg.org/#queue-a-task
    void QueueTask(TaskSource source, Steps &&steps) noexcept;

    /// @see https://html.spec.whatwg.org/#queue-a-microtask
    void QueueMicrotask(Steps &&steps) noexcept;

    /// @see https://html.spec.whatwg.org/#perform-a-microtask-checkpoint
    void PerformMicrotaskCheckpoint() noexcept;

    /// @brief Runs the oldest task, then performs a microtask checkpoint (one iteration of the processing
    /// model).
    /// @return Whether a task ran.
    /// @see https://html.spec.whatwg.org/#event-loop-processing-model
    bool RunNextTask() noexcept;

    /// @brief Runs tasks, including the ones they queue, until there are none left.
    void RunUntilIdle() noexcept;

    KRYS_NODISCARD bool HasPendingTasks() const noexcept
    {
      return !_taskQueue.empty();
    }

    KRYS_NODISCARD bool HasPendingMicrotasks() const noexcept
    {
      return !_microtaskQueue.empty();
    }
  };
}
