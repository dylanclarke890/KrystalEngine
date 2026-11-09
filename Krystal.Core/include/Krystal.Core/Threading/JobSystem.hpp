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
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace krys
{
  /// @brief A pool of worker threads that run jobs. Workers sleep while there's nothing to do; a free one
  /// takes the oldest Normal job, or else the oldest Background one.
  /// @note While jobs that may block run (`JobTraits::MayBlock`), the pool keeps its worker count free of
  /// them by starting more workers, up to `MaximumExtraWorkers`; an extra worker left idle for a while stops.
  class JobSystem final : public WorkQueue
  {
    KRYS_NON_COPY_MOVABLE(JobSystem);

    struct QueuedJob
    {
      MoveOnlyFunc<void()> Job;
      bool MayBlock {false};
    };

  private:
    mutable std::mutex _mutex;
    std::condition_variable _jobAvailable;
    Deque<QueuedJob> _normalJobs;
    Deque<QueuedJob> _backgroundJobs;
    bool _stopping {false};
    utf8_string _name;
    uint32 _workerCount {0};
    std::chrono::milliseconds _extraWorkerIdleTimeout;
    /// @brief How many workers have been started, to number them.
    uint32 _startedWorkers {0};
    /// @brief How many jobs that may block are running.
    uint32 _blockingJobs {0};
    List<UniquePtr<Thread>> _workers;
    /// @brief The workers that stopped, idle, waiting to be joined.
    List<UniquePtr<Thread>> _stoppedWorkers;

  public:
    /// @brief How many workers the pool starts beyond its count, at most, for the jobs that may block.
    static constexpr uint32 MaximumExtraWorkers = 32;

    /// @brief Starts @p workerCount workers, or by default one fewer than the machine has cores (at least
    /// one), leaving a core for the main thread. They're named @p name, numbered if there's more than one.
    /// @param extraWorkerIdleTimeout How long a worker started for a job that may block waits for another job
    /// before it stops.
    explicit JobSystem(Maybe<uint32> workerCount = null, utf8_stringview name = u8"Worker",
                       std::chrono::milliseconds extraWorkerIdleTimeout = std::chrono::seconds(10)) noexcept;
    /// @brief Stops taking jobs, lets the Normal jobs already queued run, destroys the Background ones unrun,
    /// and waits for the workers to finish.
    ~JobSystem() noexcept override;

    using WorkQueue::Dispatch;
    /// @brief Runs @p job on a worker. While the system is shutting down, the job is destroyed unrun.
    void Dispatch(MoveOnlyFunc<void()> job, JobTraits traits = {}) noexcept override;

    /// @brief How many workers the pool keeps for jobs that don't block.
    KRYS_NODISCARD uint32 WorkerCount() const noexcept
    {
      return _workerCount;
    }

    /// @brief How many workers are running now: the pool's count, and those started for jobs that may block.
    KRYS_NODISCARD uint32 RunningWorkerCount() const noexcept;

    /// @brief The workers' count by default: one fewer than the machine has cores, and at least one.
    KRYS_NODISCARD static uint32 DefaultWorkerCount() noexcept;

  private:
    /// @brief Starts a worker; under the lock.
    void StartWorker() noexcept;
    /// @brief What each worker runs: the next job, until the system stops and no Normal job is left, or the
    /// worker is an extra one left idle.
    void RunWorker() noexcept;
    /// @brief Whether more workers run than the pool's count and the jobs that may block need; under the
    /// lock.
    KRYS_NODISCARD bool HasSpareWorker() const noexcept;
  };
}
