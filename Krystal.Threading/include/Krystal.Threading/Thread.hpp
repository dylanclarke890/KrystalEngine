#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <thread>

namespace krys
{
  /// @brief A thread of its own, named for debuggers and profilers, that runs a function until it returns:
  /// for work that blocks, has a deadline or never ends (audio, file reads), which doesn't suit a job.
  class Thread final
  {
    KRYS_NON_COPY_MOVABLE(Thread);

  private:
    utf8_string _name;
    std::jthread _thread;

  public:
    /// @brief Starts a thread named @p name that runs @p body.
    Thread(utf8_string name, MoveOnlyFunc<void()> body) noexcept;
    /// @brief Waits for the body to return.
    ~Thread() noexcept;

    KRYS_NODISCARD const utf8_string &Name() const noexcept
    {
      return _name;
    }

    /// @brief The thread's identifier, for checking which thread something runs on.
    KRYS_NODISCARD std::thread::id Id() const noexcept
    {
      return _thread.get_id();
    }
  };
}
