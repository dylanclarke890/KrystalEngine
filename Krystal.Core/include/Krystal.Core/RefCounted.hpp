#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <atomic>

namespace krys::detail
{
  /// @brief The top bit of a reference count, set once its object's deletion has begun. Keeping it in the
  /// count costs no space, so the checks it backs stay in every build.
  constexpr uint32 DeletionHasBegunFlag = 1u << 31u;

  class RefCounted
  {
  private:
    mutable uint32 _refCount {1};

  protected:
    RefCounted() noexcept = default;

    ~RefCounted() noexcept
    {
      krys_assert_msg(_refCount & DeletionHasBegunFlag,
                      "RefCounted: Deletion has not begun, but destructor is being called.");
      krys_assert_msg((_refCount & ~DeletionHasBegunFlag) == 1,
                      "RefCounted: Attempt to destroy object when ref count is not one.");
    }

  public:
    void AddRef() const noexcept
    {
      krys_assert_msg(!(_refCount & DeletionHasBegunFlag),
                      "RefCounted: Attempt to add reference after deletion has begun.");
      ++_refCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return GetRefCount() == 1;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _refCount & ~DeletionHasBegunFlag;
    }

    KRYS_NODISCARD bool DeletionHasBegun() const noexcept
    {
      return _refCount & DeletionHasBegunFlag;
    }

  protected:
    /// @brief Returns true if the pointer should be deleted.
    bool SubRefBase() const noexcept
    {
      krys_assert_msg(!(_refCount & DeletionHasBegunFlag),
                      "RefCounted: Attempt to subtract reference after deletion has begun.");
      krys_assert_msg(_refCount > 0, "RefCounted: Attempt to subtract reference when ref count is zero.");

      if (_refCount == 1)
      {
        // The count stays at one, flagged, so that adding or subtracting a reference after deletion has begun
        // is caught.
        _refCount |= DeletionHasBegunFlag;
        return true;
      }

      --_refCount;
      return false;
    }

    /// @brief For an object that stays alive after its last reference was dropped, kept by something other
    /// than references (a document by the nodes that refer to it): its count resumes from zero, and it may
    /// gain references again.
    void ResumeFromNoRefs() const noexcept
    {
      _refCount = 0;
    }

    /// @brief Before deleting an object kept with no references: restores the count its destruction expects.
    void PrepareToDeleteWithNoRefs() const noexcept
    {
      krys_debug_assert(_refCount == 0);
      _refCount = 1 | DeletionHasBegunFlag;
    }
  };

  class ThreadSafeRefCounted
  {
    KRYS_NON_COPYABLE(ThreadSafeRefCounted);

  private:
    mutable std::atomic<uint32> _refCount {1};

  public:
    void AddRef() const noexcept
    {
      krys_assert_msg(!(_refCount.load(std::memory_order_relaxed) & DeletionHasBegunFlag),
                      "ThreadSafeRefCounted: Attempt to add reference after deletion has begun.");
      ++_refCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return GetRefCount() == 1;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _refCount & ~DeletionHasBegunFlag;
    }

    KRYS_NODISCARD bool DeletionHasBegun() const noexcept
    {
      return _refCount.load(std::memory_order_relaxed) & DeletionHasBegunFlag;
    }

  protected:
    constexpr ThreadSafeRefCounted() noexcept = default;

    ~ThreadSafeRefCounted() noexcept
    {
      krys_assert_msg(_refCount == (1 | DeletionHasBegunFlag),
                      "ThreadSafeRefCounted: Destroyed when its deletion hadn't begun or it had references.");
    }

    // Returns true if the pointer should be freed.
    bool SubRefBase() const noexcept
    {
      krys_assert_msg(!(_refCount.load(std::memory_order_relaxed) & DeletionHasBegunFlag),
                      "ThreadSafeRefCounted: Attempt to subtract reference after deletion has begun.");

      if (!--_refCount) KRYS_UNLIKELY
      {
        _refCount = 1 | DeletionHasBegunFlag;
        return true;
      }

      return false;
    }
  };

  class RefCountedWithInlineWeakPtrBase
  {
  private:
    mutable uint32 _strongCount {1};
    mutable uint32 _weakCount {1}; // The strong counts collectively share one weak count.

  protected:
    constexpr RefCountedWithInlineWeakPtrBase() noexcept = default;

    ~RefCountedWithInlineWeakPtrBase() noexcept
    {
      krys_assert_msg(
        _strongCount == (1 | DeletionHasBegunFlag),
        "RefCountedWithInlineWeakPtr: Destroyed when its deletion hadn't begun or it had references.");
      _strongCount = 0;
    }

  public:
    void AddRef() const noexcept
    {
      krys_assert_msg(!(_strongCount & DeletionHasBegunFlag),
                      "RefCountedWithInlineWeakPtr: Attempt to add reference after deletion has begun.");
      ++_strongCount;
    }

    void AddRefWeak() const noexcept
    {
      ++_weakCount;
    }

    KRYS_NODISCARD bool HasOneRef() const noexcept
    {
      return GetRefCount() == 1;
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _strongCount & ~DeletionHasBegunFlag;
    }

    KRYS_NODISCARD bool DeletionHasBegun() const noexcept
    {
      return _strongCount & DeletionHasBegunFlag;
    }

  protected:
    // Returns true if the pointer should be destroyed.
    KRYS_NODISCARD bool SubRefBase() const noexcept
    {
      krys_assert_msg(!(_strongCount & DeletionHasBegunFlag),
                      "RefCountedWithInlineWeakPtr: Attempt to subtract reference after deletion has begun.");
      krys_assert_msg(_strongCount > 0,
                      "RefCountedWithInlineWeakPtr: Attempt to subtract reference when ref count is zero.");

      if (_strongCount != 1)
      {
        --_strongCount;
        return false;
      }

      _strongCount |= DeletionHasBegunFlag;
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

namespace krys
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
  class KRYS_EMPTY_BASE_CLASS RefCounted : public ::krys::detail::RefCounted
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
  class KRYS_EMPTY_BASE_CLASS ThreadSafeRefCounted : public ::krys::detail::ThreadSafeRefCounted
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
      : public ::krys::detail::RefCountedWithInlineWeakPtrBase
  {
    KRYS_NON_COPYABLE(RefCountedWithInlineWeakPtr<T>);

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