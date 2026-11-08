#include "Krystal.Core/Threading/ManualWorkQueue.hpp"

namespace krys
{
  void ManualWorkQueue::Dispatch(MoveOnlyFunc<void()> job, JobPriority priority) noexcept
  {
    (priority == JobPriority::Normal ? _normalJobs : _backgroundJobs).push_back(krys::move(job));
  }

  bool ManualWorkQueue::RunNext() noexcept
  {
    auto &queue = !_normalJobs.empty() ? _normalJobs : _backgroundJobs;
    if (queue.empty())
    {
      return false;
    }
    auto job = krys::move(queue.front());
    queue.pop_front();
    job();
    return true;
  }

  uint32 ManualWorkQueue::RunAll() noexcept
  {
    uint32 count = 0;
    while (RunNext())
    {
      ++count;
    }
    return count;
  }

  void ManualWorkQueue::Clear() noexcept
  {
    // Moved out first, as destroying a job may dispatch another.
    auto normalJobs = krys::move(_normalJobs);
    auto backgroundJobs = krys::move(_backgroundJobs);
    _normalJobs.clear();
    _backgroundJobs.clear();
  }
}
