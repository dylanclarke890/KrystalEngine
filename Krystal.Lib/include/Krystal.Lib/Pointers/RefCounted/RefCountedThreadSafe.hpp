#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCountDebugger.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

#include <atomic>

namespace Krys
{
  class KRYS_EMPTY_BASE_CLASS RefCountedThreadSafeBase : public NonCopyable<RefCountedThreadSafeBase>
  {
    using Debugger = RefCountDebugger<uint32>;

  private:
    mutable std::atomic<uint32> _refCount {1};
    KRYS_NO_UNIQUE_ADDRESS Debugger _refCountDebugger;

  public:
    void AddRef() const noexcept
    {
      _refCountDebugger.WillAddRef(_refCount);
      ++_refCount;
    }

    bool HasOneRef() const noexcept
    {
      return _refCount == 1;
    }

    uint32 GetRefCount() const noexcept
    {
      return _refCount;
    }

    Debugger &GetRefCountDebugger() noexcept
    {
      return _refCountDebugger;
    }

  protected:
    constexpr RefCountedThreadSafeBase() noexcept = default;

    ~RefCountedThreadSafeBase() noexcept
    {
      _refCountDebugger.WillDestroy(_refCount);
      assert(_refCount == 1);
    }

    // Returns true if the pointer should be freed.
    bool SubRefBase() const noexcept
    {
      _refCountDebugger.WillSubRef(_refCount);

      if (!--_refCount) KRYS_UNLIKELY
      {
        _refCountDebugger.MarkDeletionHasBegun();
        _refCount = 1;
        return true;
      }

      return false;
    }
  };

  template <class T>
  class RefCountedThreadSafe : public RefCountedThreadSafeBase
  {
  public:
    void SubRef() const noexcept
    {
      if (!SubRefBase())
      {
        return;
      }

      delete static_cast<const T *>(this);
    }

  protected:
    constexpr RefCountedThreadSafe() noexcept = default;
    constexpr ~RefCountedThreadSafe() noexcept = default;
  };
}
