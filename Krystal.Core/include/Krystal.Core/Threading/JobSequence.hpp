#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Threading/WorkQueue.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Queue.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <mutex>

namespace krys
{
  /// @brief A serial lane over a work queue: its jobs run one at a time, in the order they're dispatched,
  /// each on whichever of the queue's workers is free, so work that must stay in order (one file's writes, an
  /// asset's loading steps) needs no thread of its own. Each job sees everything the ones before it did.
  /// @note Every job takes the sequence's traits, given when it's made; those passed to `Dispatch` are
  /// ignored, as a job can't go ahead of those before it. The jobs dispatched run even if the sequence is
  /// destroyed first, as long as the queue does. It's thread-safe.
  class JobSequence final : public WorkQueue
  {
    KRYS_NON_COPY_MOVABLE(JobSequence);

    /// @brief What the sequence and the job that runs it share: its jobs, and whether that job is queued.
    struct State final : ThreadSafeRefCounted<State>
    {
      std::mutex Mutex;
      Deque<MoveOnlyFunc<void()>> Jobs;
      bool IsScheduled {false};
    };

  private:
    WorkQueue &_queue;
    JobTraits _traits;
    Ref<State> _state;

  public:
    /// @brief A sequence of jobs that run on @p queue, with @p traits.
    explicit JobSequence(WorkQueue &queue, JobTraits traits = {}) noexcept;
    ~JobSequence() noexcept override;

    using WorkQueue::Dispatch;
    /// @brief Runs @p job after the jobs dispatched before it, with the sequence's traits.
    void Dispatch(MoveOnlyFunc<void()> job, JobTraits traits = {}) noexcept override;

  private:
    /// @brief Queues the job that runs the sequence's next job; it runs one, then queues itself again while
    /// there are more, so other work isn't kept waiting behind a long sequence.
    static void Schedule(WorkQueue &queue, JobTraits traits, Ref<State> state) noexcept;
  };
}
