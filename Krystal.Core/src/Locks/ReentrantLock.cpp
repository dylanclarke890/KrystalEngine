#include "Krystal.Core/Locks/ReentrantLock.hpp"

#include <atomic>
#include <cassert>
#include <thread>

namespace Krys
{
  ReentrantLock::ReentrantLock() noexcept : _state(0), _refCount(0)
  {
  }

  bool ReentrantLock::TryAcquire() noexcept
  {
    std::hash<std::thread::id> hasher;
    size_t tid = hasher(std::this_thread::get_id());
    bool acquired = false;

    if (_state.load(std::memory_order_relaxed) == tid)
    {
      acquired = true;
    }
    else
    {
      size_t unlockValue = 0;
      acquired = _state.compare_exchange_strong(unlockValue, tid,
                                                std::memory_order_relaxed, // fence below!
                                                std::memory_order_relaxed);
    }

    if (acquired)
    {
      _refCount++;
      std::atomic_thread_fence(std::memory_order_acquire);
    }

    return acquired;
  }

  void ReentrantLock::Acquire() noexcept
  {
    std::hash<std::thread::id> hasher;
    size_t tid = hasher(std::this_thread::get_id());

    // if this thread doesn't already hold the lock, spin wait until we do hold it
    if (_state.load(std::memory_order_relaxed) != tid)
    {
      size_t unlockValue = 0;
      while (!_state.compare_exchange_weak(unlockValue, tid,
                                           std::memory_order_relaxed, // fence below!
                                           std::memory_order_relaxed))
      {
        unlockValue = 0;
        std::this_thread::yield();
      }
    }

    // increment reference count so we can verify that Acquire() and Release() are called in pairs
    _refCount++; // use an acquire fence to ensure all subsequent reads by this thread will be valid
    std::atomic_thread_fence(std::memory_order_acquire);
  }

  void ReentrantLock::Release() noexcept
  {
    // use release semantics to ensure that all prior writes have been fully committed before we unlock
    std::atomic_thread_fence(std::memory_order_release);
    std::hash<std::thread::id> hasher;

    size_t tid = hasher(std::this_thread::get_id());
    size_t actual = _state.load(std::memory_order_relaxed);
    assert(actual == tid); // "expected actual to be the current thread"

    _refCount--;
    if (_refCount == 0)
      _state.store(0, std::memory_order_relaxed); // release lock, which is safe because we own it
  }
}