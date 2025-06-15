#include "Krystal.Core/Locks/SpinLock.hpp"

#include <thread>

namespace Krys
{
  SpinLock::SpinLock() noexcept : _stateFlag()
  {
  }

  bool SpinLock::TryAcquire() noexcept
  {
    // use an acquire fence to ensure all subsequent reads by this thread will be valid
    bool alreadyLocked = _stateFlag.test_and_set(std::memory_order_acquire);
    return !alreadyLocked;
  }

  void SpinLock::Acquire() noexcept
  {
    while (!TryAcquire())        // Spin until successful acquire
      std::this_thread::yield(); // Reduces CPU consumption
  }

  void SpinLock::Release() noexcept
  {
    // use release semantics to ensure that all prior writes have been fully committed before we unlock
    _stateFlag.clear(std::memory_order_release);
  }
}