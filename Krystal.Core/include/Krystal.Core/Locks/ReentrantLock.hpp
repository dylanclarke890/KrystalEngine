#pragma once

#include "Krystal.Core/Core.hpp"

#include <atomic>

namespace Krys
{
  class ReentrantLock
  {
    std::atomic<size_t> _state;
    int32 _refCount;

  public:
    ReentrantLock() noexcept;

    bool TryAcquire() noexcept;
    void Acquire() noexcept;
    void Release() noexcept;
  };
}