#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RefCountDebugger.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

#include <atomic>

namespace Krys
{
  class KRYS_EMPTY_BASE_CLASS ThreadSafeRefCountedBase : public NonCopyable<ThreadSafeRefCountedBase>
  {
    using Debugger = RefCountDebugger<uint32>;

  public:
    void AddRef() const
    {
      _refCountDebugger.WillAddRef(m_refCount);
      ++m_refCount;
    }

    bool hasOneRef() const
    {
      return m_refCount == 1;
    }

    uint32 GetRefCount() const
    {
      return m_refCount;
    }

    Debugger &refCountDebugger()
    {
      return _refCountDebugger;
    }

  protected:
    ThreadSafeRefCountedBase() = default;

    ~ThreadSafeRefCountedBase()
    {
      _refCountDebugger.WillDestroy(m_refCount);
      assert(m_refCount == 1);
    }

    // Returns true if the pointer should be freed.
    bool SubRefBase() const
    {
      _refCountDebugger.WillSubRef(m_refCount);

      if (!--m_refCount) [[unlikely]]
      {
        _refCountDebugger.MarkDeletionHasBegun();
        m_refCount = 1;
        return true;
      }

      return false;
    }

  private:
    mutable std::atomic<uint32_t> m_refCount {1};
    KRYS_NO_UNIQUE_ADDRESS Debugger _refCountDebugger;
  };

  template <class T>
  class ThreadSafeRefCounted : public ThreadSafeRefCountedBase
  {
  public:
    void SubRef() const
    {
      if (!SubRefBase())
        return;

      delete static_cast<const T *>(this);
    }

  protected:
    ThreadSafeRefCounted() = default;
    ~ThreadSafeRefCounted() = default;
  };
}
