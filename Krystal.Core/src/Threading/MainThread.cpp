#include "Krystal.Core/Threading/MainThread.hpp"
#include <thread>

namespace krys
{
  namespace
  {
    KRYS_NODISCARD std::thread::id MainThreadId() noexcept
    {
      // The first call, which is during static initialisation at the latest (below), is on the thread the
      // program starts on.
      static const std::thread::id id = std::this_thread::get_id();
      return id;
    }

    // Records the main thread before `main`, even if nothing asks during static initialisation. This file is
    // linked whenever `IsMainThread` is used, as it defines it.
    [[maybe_unused]] const std::thread::id RecordedMainThread = MainThreadId();
  }

  bool IsMainThread() noexcept
  {
    return std::this_thread::get_id() == MainThreadId();
  }
}
