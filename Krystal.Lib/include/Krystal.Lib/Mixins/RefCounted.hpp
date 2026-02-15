#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/NonCopyable.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <atomic>
#include <cassert>

namespace Krys
{
  template <typename TSize>
  class RefCountedDebugger
  {
  private:
    mutable std::atomic<bool> _deletionHasBegun {false};

  public:
    RefCountedDebugger() noexcept = default;
    ~RefCountedDebugger() noexcept
    {
      assert(_deletionHasBegun.load(std::memory_order_relaxed)
             && "RefCountedDebugger: Deletion has not begun, but destructor is being called.");
    }

    void MarkDeletionHasBegun() const noexcept
    {
      _deletionHasBegun.store(true, std::memory_order_relaxed);
    }

    KRYS_NODISCARD bool DeletionHasBegun() const noexcept
    {
      return _deletionHasBegun.load(std::memory_order_relaxed);
    }

    void WillAddRef(KRYS_MAYBE_UNUSED TSize refCount) const noexcept
    {
      assert(!_deletionHasBegun.load(std::memory_order_relaxed)
             && "RefCountedDebugger: Attempt to add reference after deletion has begun.");
    }

    void WillSubRef(KRYS_MAYBE_UNUSED TSize refCount) const noexcept
    {
      assert(!_deletionHasBegun.load(std::memory_order_relaxed)
             && "RefCountedDebugger: Attempt to subtract reference after deletion has begun.");
      assert(refCount > 0 && "RefCountedDebugger: Attempt to subtract reference when ref count is zero.");
    }

    void WillDestroy(KRYS_MAYBE_UNUSED TSize refCount) const noexcept
    {
      assert(refCount == 1 && "RefCountedDebugger: Attempt to destroy object when ref count is not one.");
    }
  };
}

namespace Krys::detail
{
  class RefCounted
  {
    using Debugger = RefCountedDebugger<uint32>;

  private:
    mutable uint32 _refCount {1};
    KRYS_NO_UNIQUE_ADDRESS Debugger _debugger;

  protected:
    RefCounted() noexcept = default;

    ~RefCounted() noexcept
    {
      _debugger.WillDestroy(_refCount);
    }

  public:
    void AddRef() const noexcept
    {
      _debugger.WillAddRef(_refCount);
      ++_refCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return _refCount == 1;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _refCount;
    }

    KRYS_NODISCARD const Debugger &GetDebugger() const noexcept
    {
      return _debugger;
    }

    KRYS_NODISCARD Debugger &GetDebugger() noexcept
    {
      return _debugger;
    }

  protected:
    /// @brief Returns true if the pointer should be deleted.
    bool SubRefBase() const noexcept
    {
      _debugger.WillSubRef(_refCount);

      auto tempCount = _refCount - 1u;
      if (tempCount == 0)
      {
        _debugger.MarkDeletionHasBegun();
        // Don't save the decremented count, it allows us to detect if someone tries to AddRef or SubRef after
        // deletion has begun.
        return true;
      }

      _refCount = tempCount;
      return false;
    }
  };

  class ThreadSafeRefCounted : public NonCopyable<ThreadSafeRefCounted>
  {
    using Debugger = RefCountedDebugger<uint32>;

  private:
    mutable std::atomic<uint32> _refCount {1};
    KRYS_NO_UNIQUE_ADDRESS Debugger _debugger;

  public:
    void AddRef() const noexcept
    {
      _debugger.WillAddRef(_refCount);
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

    Debugger &GetDebugger() noexcept
    {
      return _debugger;
    }

  protected:
    constexpr ThreadSafeRefCounted() noexcept = default;

    ~ThreadSafeRefCounted() noexcept
    {
      _debugger.WillDestroy(_refCount);
      assert(_refCount == 1);
    }

    // Returns true if the pointer should be freed.
    bool SubRefBase() const noexcept
    {
      _debugger.WillSubRef(_refCount);

      if (!--_refCount) KRYS_UNLIKELY
      {
        _debugger.MarkDeletionHasBegun();
        _refCount = 1;
        return true;
      }

      return false;
    }
  };

  class RefCountedWithInlineWeakPtrBase
  {
    using Debugger = RefCountedDebugger<uint32>;

  private:
    mutable uint32 _strongCount {1};
    mutable uint32 _weakCount {1}; // The strong counts collectively share one weak count.
    KRYS_NO_UNIQUE_ADDRESS Debugger _debugger;

  protected:
    constexpr RefCountedWithInlineWeakPtrBase() noexcept = default;

    ~RefCountedWithInlineWeakPtrBase() noexcept
    {
      _debugger.WillDestroy(_strongCount);
      _strongCount = 0;
    }

  public:
    void AddRef() const noexcept
    {
      _debugger.WillAddRef(_strongCount);
      ++_strongCount;
    }

    void AddRefWeak() const noexcept
    {
      ++_weakCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return _strongCount == 1;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _strongCount;
    }

    KRYS_NODISCARD Debugger &GetDebugger() const noexcept
    {
      return const_cast<Debugger &>(_debugger);
    }

  protected:
    // Returns true if the pointer should be destroyed.
    KRYS_NODISCARD bool SubRefBase() const noexcept
    {
      _debugger.WillSubRef(_strongCount);

      if (_strongCount != 1)
      {
        --_strongCount;
        return false;
      }

      _debugger.MarkDeletionHasBegun();
      return true;
    }

    // Returns true if the pointer should be freed.
    KRYS_NODISCARD bool SubRefWeakBase() const noexcept
    {
      if (_weakCount != 1)
      {
        --_weakCount;
        return false;
      }

      return true;
    }
  };
}

namespace Krys
{
  class AbstractRefCounted
  {
  protected:
    virtual ~AbstractRefCounted() noexcept = default;

  public:
    virtual void AddRef() const noexcept = 0;
    virtual void SubRef() const noexcept = 0;
    virtual uint32 GetRefCount() const noexcept = 0;
  };

  template <typename T>
  class KRYS_EMPTY_BASE_CLASS RefCounted : public ::Krys::detail::RefCounted
  {
  protected:
    RefCounted() noexcept = default;
    ~RefCounted() noexcept = default;

  public:
    void SubRef() const noexcept
    {
      if (SubRefBase())
      {
        delete const_cast<T *>(static_cast<const T *>(this));
      }
    }
  };

  template <class T>
  class KRYS_EMPTY_BASE_CLASS ThreadSafeRefCounted : public ::Krys::detail::ThreadSafeRefCounted
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
    constexpr ThreadSafeRefCounted() noexcept = default;
    constexpr ~ThreadSafeRefCounted() noexcept = default;
  };

  template <typename T>
  class KRYS_EMPTY_BASE_CLASS RefCountedWithInlineWeakPtr
      : public ::Krys::detail::RefCountedWithInlineWeakPtrBase,
        public NonCopyable<RefCountedWithInlineWeakPtr<T>>
  {
  public:
    constexpr RefCountedWithInlineWeakPtr() noexcept = default;

    void SubRef() const noexcept
    {
      if (!SubRefBase()) KRYS_LIKELY
      {
        return;
      }

      T::operator delete(const_cast<T *>(static_cast<const T *>(this)));
    }

    void SubRefWeak() const noexcept
    {
      if (!SubRefWeakBase()) KRYS_LIKELY
      {
        return;
      }

      const_cast<T *>(static_cast<const T *>(this))->~T();
      SubRefWeak();
    }
  };
}