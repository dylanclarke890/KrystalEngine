#pragma once

#include <concepts>

namespace Krys
{
  template <typename TLock>
  concept Lockable = requires(TLock l) {
    { l.Acquire() } -> std::same_as<void>;
    { l.Release() } -> std::same_as<void>;
  };

  template <class Lockable>
  class ScopedLock
  {
    Lockable *_lock;

  public:
    explicit ScopedLock(Lockable &lock) noexcept : _lock(lock)
    {
      _lock->Acquire();
    }

    ~ScopedLock() noexcept
    {
      _lock->Release();
    }
  };
}
