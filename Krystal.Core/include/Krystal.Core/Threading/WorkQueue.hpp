#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Threading/TaskInbox.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  /// @brief How soon a job is needed, which decides which waiting job starts next. A running job is never
  /// interrupted.
  enum class JobPriority : uint8
  {
    /// @brief Needed soon, such as an image about to be drawn.
    Normal,
    /// @brief Needed eventually, such as streaming or warming a cache: starts only when no Normal job is
    /// waiting.
    Background,
  };

  /// @brief What a job is, which decides when it starts and how the pool makes room for it.
  struct JobTraits
  {
    JobPriority Priority {JobPriority::Normal};
    /// @brief Whether the job may block, waiting rather than computing (a file read, a network request):
    /// while it runs, the pool starts another worker if it must, so the other jobs keep as many workers as
    /// the pool has without it.
    bool MayBlock {false};
  };

  /// @brief Somewhere to send jobs: short functions that finish, which take what they need when they're
  /// dispatched and touch nothing else. What code that dispatches jobs takes, so that it runs them on the
  /// `JobSystem`'s workers, one at a time on a `JobSequence`, or in a test, on its own thread when the test
  /// says (`ManualWorkQueue`).
  /// @note A job may use what it owns (moved into it), immutable data shared through a `Ref`, and services
  /// documented as thread-safe; never the dispatching thread's state, nor its stack (capture by value, not
  /// `[&]`). A job that blocks on IO says so (`JobTraits::MayBlock`); no job waits on another.
  class WorkQueue
  {
  public:
    virtual ~WorkQueue() noexcept = default;

    /// @brief Runs @p job some time after this returns.
    virtual void Dispatch(MoveOnlyFunc<void()> job, JobTraits traits = {}) noexcept = 0;

    /// @brief Runs @p job, then posts @p completion, with what the job returned (if anything), to @p inbox,
    /// whose owner runs it when it next drains the inbox. The completion is moved to the job's thread and
    /// back, never run or copied there.
    template <Callable TJob, typename TCompletion>
    void Dispatch(TJob job, TaskInbox &inbox, TCompletion completion, JobTraits traits = {}) noexcept
    {
      Dispatch(
        [job = krys::move(job), inbox = &inbox, completion = krys::move(completion)]() mutable
        {
          using Result = invoke_result_t<TJob &>;
          if constexpr (Void<Result>)
          {
            job();
            inbox->Post(krys::move(completion));
          }
          else
          {
            inbox->Post([completion = krys::move(completion), result = job()]() mutable
                        { completion(krys::move(result)); });
          }
        },
        traits);
    }
  };
}
