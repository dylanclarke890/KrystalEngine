#include "Krystal.Core/Locks/ReadersWriterLock.hpp"

#include "Krystal.Core/Core.hpp"

#include <thread>

namespace Krys
{
  ReadersWriterLock::ReadersWriterLock() noexcept : _state(0)
  {
  }

  bool ReadersWriterLock::TryAcquire() noexcept
  {
    uint32 expected = 0;
    // Try to acquire the writer lock by setting the high bit
    return _state.compare_exchange_strong(expected, WRITER_LOCK, std::memory_order_acquire);
  }

  void ReadersWriterLock::Acquire() noexcept
  {
    while (!TryAcquire())
      std::this_thread::yield();
  }

  void ReadersWriterLock::Release() noexcept
  {
    // If we are a writer, clear the high bit.
    uint32 current = _state.load(std::memory_order_acquire);
    if (current == WRITER_LOCK)
      _state.store(0, std::memory_order_release);
    else
      _state.fetch_sub(1, std::memory_order_release);
  }

  void ReadersWriterLock::AcquireRead() noexcept
  {
    while (true)
    {
      uint32 current = _state.load(std::memory_order_acquire);

      // Check if the writer lock is not held.
      if (current < WRITER_LOCK
          && _state.compare_exchange_weak(current, current + 1, std::memory_order_acquire))
        return; // Acquired read lock successfully.

      std::this_thread::yield(); // Allow other threads a chance to release the writer lock.
    }
  }

  bool ReadersWriterLock::TryAcquireRead() noexcept
  {
    uint32 current = _state.load(std::memory_order_acquire);

    if (current < WRITER_LOCK)
      return _state.compare_exchange_strong(current, current + 1, std::memory_order_acquire);

    return false; // Failed to acquire read lock because a writer holds the lock.
  }
}