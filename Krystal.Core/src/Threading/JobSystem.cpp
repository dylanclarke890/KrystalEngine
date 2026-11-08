#include "Krystal.Core/Threading/JobSystem.hpp"
#include <algorithm>
#include <format>
#include <ranges>
#include <thread>

namespace krys
{
  uint32 JobSystem::DefaultWorkerCount() noexcept
  {
    // Unknown (0) is taken as two cores.
    auto cores = std::max(std::thread::hardware_concurrency(), 2u);
    return cores - 1;
  }

  JobSystem::JobSystem(Maybe<uint32> workerCount, utf8_stringview name) noexcept
  {
    auto count = std::max(workerCount.value_or(DefaultWorkerCount()), 1u);
    _workers.reserve(count);
    for (uint32 i = 0; i < count; ++i)
    {
      auto workerName = utf8_string(name);
      if (count > 1)
      {
        workerName += std::format(" {}", i + 1) | std::ranges::to<utf8_string>();
      }
      _workers.push_back(CreateUnique<Thread>(krys::move(workerName), [this]() { RunWorker(); }));
    }
  }

  JobSystem::~JobSystem() noexcept
  {
    Deque<MoveOnlyFunc<void()>> dropped;
    {
      std::scoped_lock lock(_mutex);
      _stopping = true;
      dropped.swap(_backgroundJobs);
    }
    _jobAvailable.notify_all();
    // The dropped jobs are destroyed outside the lock, as what they hold may dispatch or post.
    dropped.clear();
    // Each worker's thread waits for it to finish the Normal jobs left.
    _workers.clear();
  }

  void JobSystem::Dispatch(MoveOnlyFunc<void()> job, JobPriority priority) noexcept
  {
    {
      std::scoped_lock lock(_mutex);
      if (_stopping)
      {
        // The job, left in its parameter, is destroyed unrun as this returns, outside the lock.
        return;
      }
      (priority == JobPriority::Normal ? _normalJobs : _backgroundJobs).push_back(krys::move(job));
    }
    _jobAvailable.notify_one();
  }

  void JobSystem::RunWorker() noexcept
  {
    while (true)
    {
      MoveOnlyFunc<void()> job;
      {
        std::unique_lock lock(_mutex);
        _jobAvailable.wait(lock,
                           [&]() { return _stopping || !_normalJobs.empty() || !_backgroundJobs.empty(); });
        auto &queue = !_normalJobs.empty() ? _normalJobs : _backgroundJobs;
        if (queue.empty())
        {
          // Stopping, with no Normal jobs left (the Background ones were dropped).
          return;
        }
        job = krys::move(queue.front());
        queue.pop_front();
      }
      job();
    }
  }
}
