#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Threading/WorkQueue.hpp"

namespace krys::threads
{
  /// @brief A work queue that keeps its jobs until it's told to run them, on the thread that tells it: for
  /// tests of code that dispatches jobs, which can then check what happens before a job runs, after, and in
  /// between, with no timing involved. Normal jobs run before Background ones, as on the `JobSystem`.
  /// @note It isn't thread-safe: one thread dispatches and runs.
  class ManualWorkQueue final : public WorkQueue
  {
    KRYS_NON_COPY_MOVABLE(ManualWorkQueue);

  private:
    Deque<MoveOnlyFunc<void()>> _normalJobs;
    Deque<MoveOnlyFunc<void()>> _backgroundJobs;

  public:
    ManualWorkQueue() noexcept = default;

    using WorkQueue::Dispatch;
    void Dispatch(MoveOnlyFunc<void()> job, JobPriority priority = JobPriority::Normal) noexcept override;

    /// @brief Runs the next job, if there's one.
    /// @return Whether a job ran.
    bool RunNext() noexcept;

    /// @brief Runs jobs, including the ones they dispatch, until there are none.
    /// @return How many ran.
    uint32 RunAll() noexcept;

    /// @brief Destroys the jobs waiting, unrun.
    void Clear() noexcept;

    /// @brief How many jobs are waiting.
    KRYS_NODISCARD uint32 PendingCount() const noexcept
    {
      return static_cast<uint32>(_normalJobs.size() + _backgroundJobs.size());
    }
  };
}
