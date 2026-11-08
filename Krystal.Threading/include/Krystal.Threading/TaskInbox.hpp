#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <mutex>
#include <thread>

namespace krys::threads
{
  /// @brief Somewhere other threads can send work back to the thread that owns it, such as a job's
  /// completion: a thread-safe queue of tasks that the owner runs, in the order they arrived, when it next
  /// drains it.
  /// @note The owner is the thread that creates it. It must outlive every job that posts to it.
  class TaskInbox final
  {
    KRYS_NON_COPY_MOVABLE(TaskInbox);

  private:
    mutable std::mutex _mutex;
    List<MoveOnlyFunc<void()>> _tasks;
    Func<void()> _wakeHandler;
    std::thread::id _owner {std::this_thread::get_id()};

  public:
    TaskInbox() noexcept = default;
    /// @brief Destroys the tasks that haven't run, unrun.
    ~TaskInbox() noexcept;

    /// @brief Adds @p task, to run when the owner next drains the inbox; callable from any thread.
    void Post(MoveOnlyFunc<void()> task) noexcept;

    /// @brief Runs the tasks posted before this was called, in the order they arrived; the ones they post run
    /// at the next drain. Only on the owner.
    /// @return How many tasks ran.
    uint32 Drain() noexcept;

    /// @brief Whether there are tasks waiting to run.
    KRYS_NODISCARD bool HasTasks() const noexcept;

    /// @brief Sets @p handler to be called after each post, on the posting thread, for an owner that sleeps
    /// when idle to wake up and drain; it must be safe to call from any thread. Only on the owner.
    void SetWakeHandler(Func<void()> handler) noexcept;

    /// @brief Whether this thread is the owner.
    KRYS_NODISCARD bool IsOwner() const noexcept
    {
      return std::this_thread::get_id() == _owner;
    }
  };
}
