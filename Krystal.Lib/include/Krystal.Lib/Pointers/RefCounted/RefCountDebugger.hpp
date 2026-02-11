#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include <atomic>
#include <cassert>

namespace Krys
{
  template <typename TRefSize>
  class RefCountDebugger
  {
  private:
    mutable std::atomic<bool> _deletionHasBegun {false};

  public:
    RefCountDebugger() noexcept = default;
    ~RefCountDebugger() noexcept
    {
      assert(_deletionHasBegun.load(std::memory_order_relaxed)
             && "RefCountDebugger: Deletion has not begun, but destructor is being called.");
    }

    void MarkDeletionHasBegun() const noexcept
    {
      _deletionHasBegun.store(true, std::memory_order_relaxed);
    }

    KRYS_NODISCARD bool DeletionHasBegun() const noexcept
    {
      return _deletionHasBegun.load(std::memory_order_relaxed);
    }

    void WillAddRef(KRYS_MAYBE_UNUSED TRefSize refCount) const noexcept
    {
      assert(!_deletionHasBegun.load(std::memory_order_relaxed)
             && "RefCountDebugger: Attempt to add reference after deletion has begun.");
    }

    void WillSubRef(KRYS_MAYBE_UNUSED TRefSize refCount) const noexcept
    {
      assert(!_deletionHasBegun.load(std::memory_order_relaxed)
             && "RefCountDebugger: Attempt to subtract reference after deletion has begun.");
      assert(refCount > 0 && "RefCountDebugger: Attempt to subtract reference when ref count is zero.");
    }

    void WillDestroy(KRYS_MAYBE_UNUSED TRefSize refCount) const noexcept
    {
      assert(refCount == 1 && "RefCountDebugger: Attempt to destroy object when ref count is not one.");
    }
  };
}