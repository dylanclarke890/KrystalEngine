#pragma once

#include "Krystal.Booey/HTML/EventLoop/TaskSource.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Threading/TaskInbox.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class Timer;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  /// @brief What becomes of a document's task once the document's active objects are stopped.
  enum class StoppedDocumentTask : uint8
  {
    /// @brief It's dropped, as its document is gone.
    Drop,
    /// @brief It runs, as it finishes work other documents may share (a load in the memory cache).
    Run
  };

  /// @brief A minimal event loop: one queue of tasks and a microtask queue, with the timers, run by the host
  /// with its current time. A task may belong to a document: it waits while the document's tasks are
  /// suspended (in the back/forward cache), and is usually dropped once the document's active objects are
  /// stopped.
  /// @note There's no rendering step and no scripting, so a microtask checkpoint only runs microtasks.
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
      /// @see https://html.spec.whatwg.org/#concept-task-document
      WeakPtr<dom::Document> Document;
      bool HasDocument {false};
      StoppedDocumentTask WhenStopped {StoppedDocumentTask::Drop};
    };

    // NOTE: the spec allows one task queue per source; tasks from every source share one queue here, which
    // keeps tasks from the same source in order.
    Deque<Task> _taskQueue;

    /// @see https://html.spec.whatwg.org/#microtask-queue
    Queue<Steps> _microtaskQueue;

    /// @see https://html.spec.whatwg.org/#performing-a-microtask-checkpoint
    bool _performingMicrotaskCheckpoint {false};

    /// @brief The timers of the tasks queued after a timeout; fired ones are removed when the loop next runs.
    List<UniquePtr<Timer>> _scheduledTasks;

    /// @brief Where jobs post back to the loop's thread.
    TaskInbox _inbox;

  public:
    EventLoop() noexcept;
    ~EventLoop() noexcept;

    /// @brief The event loop of the surrounding agent: the process's, as there are no agents (see
    /// DIFFERENCES.md).
    KRYS_NODISCARD static EventLoop &Current() noexcept;

    /// @see https://html.spec.whatwg.org/#queue-a-task
    void QueueTask(TaskSource source, Steps &&steps) noexcept;

    /// @brief Queues a task of @p document (a global task of its window, or an element task of one of its
    /// elements), which waits while the document is suspended; @p whenStopped is what becomes of it once the
    /// document is stopped or gone.
    /// @see https://html.spec.whatwg.org/#queue-a-global-task
    void QueueTask(TaskSource source, dom::Document &document, Steps &&steps,
                   StoppedDocumentTask whenStopped = StoppedDocumentTask::Drop) noexcept;

    /// @brief Queues a task on @p source after @p timeout of the host's time.
    /// @see https://html.spec.whatwg.org/#run-steps-after-a-timeout
    // FIXME: The ordering identifier (timeouts with the same one complete in order), once there are script
    // timers.
    void ScheduleTask(MonotonicTime::duration timeout, TaskSource source, Steps &&steps) noexcept;

    /// @see https://html.spec.whatwg.org/#queue-a-microtask
    void QueueMicrotask(Steps &&steps) noexcept;

    /// @see https://html.spec.whatwg.org/#perform-a-microtask-checkpoint
    void PerformMicrotaskCheckpoint() noexcept;

    /// @brief Runs the oldest task, then performs a microtask checkpoint (one iteration of the processing
    /// model).
    /// @return Whether a task ran.
    /// @see https://html.spec.whatwg.org/#event-loop-processing-model
    bool RunNextTask() noexcept;

    /// @brief Runs tasks, including the ones they queue, and the timers due at the current time, until there
    /// are none left.
    void RunUntilIdle() noexcept;

    /// @brief Runs the loop at the host's time @p now: fires the timers that are due, then runs tasks (each
    /// followed by a microtask checkpoint) until there are none left or @p budget (of real time) runs out.
    /// @note The host calls this once a frame. Time never goes backwards: an earlier @p now than the last is
    /// taken as the last.
    void Run(MonotonicTime::duration now, Maybe<MonotonicTime::duration> budget = null) noexcept;

    /// @brief The host's time, as of the last time it ran the loop.
    KRYS_NODISCARD MonotonicTime::duration Now() const noexcept;

    /// @brief When the loop next has work: now if a task or a completion is waiting, or when the next timer
    /// fires; none if it's idle.
    KRYS_NODISCARD Maybe<MonotonicTime::duration> NextWakeTime() const noexcept;

    /// @brief Whether a task can run: one without a document, or whose document's tasks aren't suspended.
    KRYS_NODISCARD bool HasPendingTasks() const noexcept;

    KRYS_NODISCARD bool HasPendingMicrotasks() const noexcept
    {
      return !_microtaskQueue.empty();
    }

    /// @brief Where jobs on other threads post their completions: the loop runs them as it runs, before its
    /// timers and tasks. They're the way back from steps the specifications run in parallel, so a completion
    /// queues a task (on the right task source, for the right document) for whatever touches the DOM.
    KRYS_NODISCARD TaskInbox &Inbox() noexcept
    {
      return _inbox;
    }

  private:
    /// @brief Runs the completions posted to the inbox, then a microtask checkpoint if any ran.
    /// @return Whether any ran.
    bool DrainInbox() noexcept;
  };
}
