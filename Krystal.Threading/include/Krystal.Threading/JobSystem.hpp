#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Threading/TaskTarget.hpp"
#include "Krystal.Threading/Thread.hpp"
#include <condition_variable>
#include <mutex>

namespace krys::threads
{
  /// @brief How soon a job is needed, which decides which waiting job a free worker starts next. A running
  /// job is never interrupted.
  enum class JobPriority : uint8
  {
    /// @brief Needed soon, such as an image about to be drawn.
    Normal,
    /// @brief Needed eventually, such as streaming or warming a cache: starts only when no Normal job is
    /// waiting.
    Background,
  };

  /// @brief A pool of worker threads that run jobs: short functions that finish, which take what they need
  /// when they're dispatched and touch nothing else. Workers sleep while there's nothing to do.
  /// @note A job may use what it owns (moved into it), immutable data shared through a `Ref`, and services
  /// documented as thread-safe; never the dispatching thread's state, nor its stack (capture by value, not
  /// `[&]`). Jobs mustn't block on IO or wait on other jobs.
  class JobSystem final
  {
    KRYS_NON_COPY_MOVABLE(JobSystem);

  private:
    std::mutex _mutex;
    std::condition_variable _jobAvailable;
    Deque<MoveOnlyFunc<void()>> _normalJobs;
    Deque<MoveOnlyFunc<void()>> _backgroundJobs;
    bool _stopping {false};
    List<UniquePtr<Thread>> _workers;

  public:
    /// @brief Starts @p workerCount workers, or by default one fewer than the machine has cores (at least
    /// one), leaving a core for the main thread.
    explicit JobSystem(Maybe<uint32> workerCount = null) noexcept;
    /// @brief Stops taking jobs, lets the Normal jobs already queued run, destroys the Background ones unrun,
    /// and waits for the workers to finish.
    ~JobSystem() noexcept;

    /// @brief Runs @p job on a worker, some time after this returns. While the system is shutting down, the
    /// job is destroyed unrun.
    void Dispatch(MoveOnlyFunc<void()> job, JobPriority priority = JobPriority::Normal) noexcept;

    /// @brief Runs @p job on a worker, then posts @p completion, with what the job returned (if anything), to
    /// @p target, whose owner runs it when it next drains the target. The completion is moved to the worker
    /// and back, never run or copied there.
    template <Callable TJob, typename TCompletion>
    void Dispatch(TJob job, TaskTarget &target, TCompletion completion,
                  JobPriority priority = JobPriority::Normal) noexcept
    {
      Dispatch(
        [job = krys::move(job), target = &target, completion = krys::move(completion)]() mutable
        {
          using Result = invoke_result_t<TJob &>;
          if constexpr (Void<Result>)
          {
            job();
            target->Post(krys::move(completion));
          }
          else
          {
            target->Post([completion = krys::move(completion), result = job()]() mutable
                         { completion(krys::move(result)); });
          }
        },
        priority);
    }

    KRYS_NODISCARD uint32 WorkerCount() const noexcept
    {
      return static_cast<uint32>(_workers.size());
    }

    /// @brief The workers' count by default: one fewer than the machine has cores, and at least one.
    KRYS_NODISCARD static uint32 DefaultWorkerCount() noexcept;

  private:
    /// @brief What each worker runs: the next job, until the system stops and no Normal job is left.
    void RunWorker() noexcept;
  };
}
