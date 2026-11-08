#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Threading/Thread.hpp"
#include "Krystal.Core/Threading/WorkQueue.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <condition_variable>
#include <mutex>

namespace krys
{
  /// @brief A pool of worker threads that run jobs. Workers sleep while there's nothing to do; a free one
  /// takes the oldest Normal job, or else the oldest Background one.
  /// @note With one worker, jobs run one at a time, in the order they're dispatched within a priority: a
  /// thread of its own for work that blocks, such as file reads, which the application's workers mustn't do.
  class JobSystem final : public WorkQueue
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
    /// one), leaving a core for the main thread. They're named @p name, numbered if there's more than one.
    explicit JobSystem(Maybe<uint32> workerCount = null, utf8_stringview name = u8"Worker") noexcept;
    /// @brief Stops taking jobs, lets the Normal jobs already queued run, destroys the Background ones unrun,
    /// and waits for the workers to finish.
    ~JobSystem() noexcept override;

    using WorkQueue::Dispatch;
    /// @brief Runs @p job on a worker. While the system is shutting down, the job is destroyed unrun.
    void Dispatch(MoveOnlyFunc<void()> job, JobPriority priority = JobPriority::Normal) noexcept override;

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
