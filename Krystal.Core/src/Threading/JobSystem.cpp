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

  JobSystem::JobSystem(Maybe<uint32> workerCount, utf8_stringview name,
                       std::chrono::milliseconds extraWorkerIdleTimeout) noexcept
      : _name(name), _workerCount(std::max(workerCount.value_or(DefaultWorkerCount()), 1u)),
        _extraWorkerIdleTimeout(extraWorkerIdleTimeout)
  {
    std::scoped_lock lock(_mutex);
    _workers.reserve(_workerCount);
    for (uint32 i = 0; i < _workerCount; ++i)
    {
      StartWorker();
    }
  }

  JobSystem::~JobSystem() noexcept
  {
    Deque<QueuedJob> dropped;
    {
      std::scoped_lock lock(_mutex);
      _stopping = true;
      dropped.swap(_backgroundJobs);
    }
    _jobAvailable.notify_all();
    // The dropped jobs are destroyed outside the lock, as what they hold may dispatch or post.
    dropped.clear();

    // Each worker's thread waits for it to finish the Normal jobs left. A worker may stop, idle, while
    // others are joined, so the lists are taken until both are empty.
    while (true)
    {
      List<UniquePtr<Thread>> workers;
      {
        std::scoped_lock lock(_mutex);
        workers = krys::move(_workers);
        _workers.clear();
        for (auto &worker : _stoppedWorkers)
        {
          workers.push_back(krys::move(worker));
        }
        _stoppedWorkers.clear();
      }
      if (workers.empty())
      {
        break;
      }
      workers.clear();
    }
  }

  uint32 JobSystem::RunningWorkerCount() const noexcept
  {
    std::scoped_lock lock(_mutex);
    return static_cast<uint32>(_workers.size());
  }

  void JobSystem::StartWorker() noexcept
  {
    ++_startedWorkers;
    auto workerName = _name;
    if (_workerCount > 1 || _startedWorkers > 1)
    {
      workerName += std::format(" {}", _startedWorkers) | std::ranges::to<utf8_string>();
    }
    // The worker waits for the lock this is called under before it looks for a job.
    _workers.push_back(CreateUnique<Thread>(krys::move(workerName), [this]() { RunWorker(); }));
  }

  void JobSystem::Dispatch(MoveOnlyFunc<void()> job, JobTraits traits) noexcept
  {
    {
      std::scoped_lock lock(_mutex);
      if (_stopping)
      {
        // The job, left in its parameter, is destroyed unrun as this returns, outside the lock.
        return;
      }
      (traits.Priority == JobPriority::Normal ? _normalJobs : _backgroundJobs)
        .push_back(QueuedJob {krys::move(job), traits.MayBlock});
    }
    _jobAvailable.notify_one();
  }

  bool JobSystem::HasSpareWorker() const noexcept
  {
    return _workers.size() > _workerCount + _blockingJobs;
  }

  void JobSystem::RunWorker() noexcept
  {
    auto hasWork = [this]()
    {
      return _stopping || !_normalJobs.empty() || !_backgroundJobs.empty();
    };
    std::unique_lock lock(_mutex);
    while (true)
    {
      // A spare worker stops if no job comes for it in a while: it moves its thread to be joined by
      // another, as a thread can't join itself.
      if (HasSpareWorker())
      {
        if (!_jobAvailable.wait_for(lock, _extraWorkerIdleTimeout, hasWork) && HasSpareWorker())
        {
          auto self = std::ranges::find_if(_workers, [](const UniquePtr<Thread> &worker)
                                           { return worker->Id() == std::this_thread::get_id(); });
          if (self != _workers.end())
          {
            _stoppedWorkers.push_back(krys::move(*self));
            _workers.erase(self);
          }
          return;
        }
      }
      else
      {
        _jobAvailable.wait(lock, hasWork);
      }

      auto &queue = !_normalJobs.empty() ? _normalJobs : _backgroundJobs;
      if (queue.empty())
      {
        if (_stopping)
        {
          // Stopping, with no Normal jobs left (the Background ones were dropped).
          return;
        }
        continue;
      }
      auto queued = krys::move(queue.front());
      queue.pop_front();

      // While the job may block, the others keep the pool's count of workers.
      if (queued.MayBlock)
      {
        ++_blockingJobs;
        if (!_stopping && _workers.size() < _workerCount + _blockingJobs
            && _workers.size() < _workerCount + MaximumExtraWorkers)
        {
          StartWorker();
        }
      }

      // The workers that stopped are joined here, outside the lock, by one that's carrying on.
      auto stopped = krys::move(_stoppedWorkers);
      _stoppedWorkers.clear();
      lock.unlock();
      stopped.clear();
      queued.Job();
      // The job is destroyed before the lock is taken again, as what it holds may dispatch or post.
      queued.Job = nullptr;
      lock.lock();
      if (queued.MayBlock)
      {
        --_blockingJobs;
      }
    }
  }
}
