#include "Krystal.Threading/TaskInbox.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  TaskInbox::~TaskInbox() noexcept
  {
    krys_debug_assert(IsOwner());
  }

  void TaskInbox::Post(MoveOnlyFunc<void()> task) noexcept
  {
    Func<void()> wakeHandler;
    {
      std::scoped_lock lock(_mutex);
      _tasks.push_back(krys::move(task));
      wakeHandler = _wakeHandler;
    }
    // Outside the lock: the handler may post or drain itself.
    if (wakeHandler)
    {
      wakeHandler();
    }
  }

  uint32 TaskInbox::Drain() noexcept
  {
    krys_debug_assert(IsOwner());

    // The tasks run outside the lock, so posting from them or from other threads meanwhile doesn't wait.
    List<MoveOnlyFunc<void()>> tasks;
    {
      std::scoped_lock lock(_mutex);
      tasks.swap(_tasks);
    }
    for (auto &task : tasks)
    {
      task();
    }
    return static_cast<uint32>(tasks.size());
  }

  bool TaskInbox::HasTasks() const noexcept
  {
    std::scoped_lock lock(_mutex);
    return !_tasks.empty();
  }

  void TaskInbox::SetWakeHandler(Func<void()> handler) noexcept
  {
    krys_debug_assert(IsOwner());
    std::scoped_lock lock(_mutex);
    _wakeHandler = krys::move(handler);
  }
}
