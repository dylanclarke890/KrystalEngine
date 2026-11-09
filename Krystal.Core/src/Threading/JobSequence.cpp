#include "Krystal.Core/Threading/JobSequence.hpp"

namespace krys
{
  JobSequence::JobSequence(WorkQueue &queue, JobTraits traits) noexcept
      : _queue(queue), _traits(traits), _state(CreateRef<State>())
  {
  }

  JobSequence::~JobSequence() noexcept = default;

  void JobSequence::Dispatch(MoveOnlyFunc<void()> job, JobTraits) noexcept
  {
    {
      std::scoped_lock lock(_state->Mutex);
      _state->Jobs.push_back(krys::move(job));
      if (_state->IsScheduled)
      {
        return;
      }
      _state->IsScheduled = true;
    }
    Schedule(_queue, _traits, _state);
  }

  void JobSequence::Schedule(WorkQueue &queue, JobTraits traits, Ref<State> state) noexcept
  {
    queue.Dispatch(
      [&queue, traits, state = krys::move(state)]() mutable
      {
        MoveOnlyFunc<void()> job;
        {
          std::scoped_lock lock(state->Mutex);
          job = krys::move(state->Jobs.front());
          state->Jobs.pop_front();
        }
        job();
        job = nullptr;

        bool more = false;
        {
          std::scoped_lock lock(state->Mutex);
          more = !state->Jobs.empty();
          state->IsScheduled = more;
        }
        if (more)
        {
          Schedule(queue, traits, krys::move(state));
        }
      },
      traits);
  }
}
