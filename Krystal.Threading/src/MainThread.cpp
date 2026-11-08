#include "Krystal.Threading/MainThread.hpp"
#include <thread>

namespace krys::threads
{
  namespace
  {
    // Namespace-scope variables are initialised before `main`, on the thread the program starts on. This
    // file is always linked when `IsMainThread` is used, as it defines it.
    const std::thread::id MainThreadId = std::this_thread::get_id();
  }

  bool IsMainThread() noexcept
  {
    return std::this_thread::get_id() == MainThreadId;
  }
}
