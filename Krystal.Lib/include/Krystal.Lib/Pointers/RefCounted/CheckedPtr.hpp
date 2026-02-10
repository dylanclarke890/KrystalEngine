#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/IntrusiveHandle.hpp"
#include <atomic>
#include <cassert>

namespace Krys
{
  /// @brief CheckedPtr is used to verify that the object being pointed to outlives the CheckedPtr.
  /// It does not affect the lifetime of the object being pointed to; it simply adds a runtime
  /// check (via assert) that when the object being pointed to is destroyed, there are
  /// no outstanding CheckedPtrs that reference it.
  /// @note Use is similar to WeakPtr, but CheckedPtr has less overhead and is used in cases where the target
  /// is never expected to become null.
  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedPtr = IntrusiveHandle<T, PtrTraits, CheckedPolicy<T>, CheckedAccess<T>, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> CreateCheckedPtr(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return CheckedPtr<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> AdoptCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits> ShareCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using CheckedRef = IntrusiveHandle<T, PtrTraits, CheckedPolicy<T>, CheckedAccess<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> CreateCheckedRef(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return CheckedRef<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> AdoptCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits> ShareCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits>::WithRef(ptr);
  }

  enum class DefaultedOperatorEqual : bool
  {
    No,
    Yes
  };

  template <typename TCount, bool ThreadSafe>
  class CanMakeCheckedPtrBase
  {
  public:
    using deletion_flag_type = conditional_t<ThreadSafe, std::atomic<bool>, bool>;
    using storage_type = conditional_t<ThreadSafe, std::atomic<TCount>, TCount>;

  private:
    mutable storage_type _checkedPtrCount {0};
#if KRYS_ENV(DEV)
    deletion_flag_type _didBeginDeletion {false};
#endif

  public:
    CanMakeCheckedPtrBase() noexcept = default;

    CanMakeCheckedPtrBase(const CanMakeCheckedPtrBase &) noexcept
    {
    }

    CanMakeCheckedPtrBase(CanMakeCheckedPtrBase &&) noexcept
    {
    }

    ~CanMakeCheckedPtrBase() noexcept
    {
      assert(_didBeginDeletion);
    }

    CanMakeCheckedPtrBase &operator=(const CanMakeCheckedPtrBase &) noexcept
    {
      return *this;
    }

    CanMakeCheckedPtrBase &operator=(CanMakeCheckedPtrBase &&) noexcept
    {
      return *this;
    }

    KRYS_ALWAYS_INLINE void AddRefChecked() const noexcept
    {
      ++_checkedPtrCount;
    }

    KRYS_ALWAYS_INLINE void SubRefChecked() const noexcept
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero CheckedPtrCount().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check CheckedPtrCount() here, we're checking for a scribbled
      // object.
      if (!CheckedPtrCount()) KRYS_UNLIKELY
      {
        CrashDueToCheckedPtrToDeadObject();
      }
      --_checkedPtrCount;
    }

    KRYS_NODISCARD TCount CheckedPtrCount() const noexcept
    {
      return _checkedPtrCount;
    }

    void SetDidBeginCheckedPtrDeletion() noexcept
    {
#if KRYS_ENV(DEV)
      _didBeginDeletion = true;
#endif
    }

  private:
    KRYS_NORETURN KRYS_NEVER_INLINE static void CrashDueToCheckedPtrToDeadObject() noexcept
    {
      std::terminate();
    }
  };

  template <typename T, DefaultedOperatorEqual DefaultOperatorEqual = DefaultedOperatorEqual::No>
  class CanMakeCheckedPtr : public CanMakeCheckedPtrBase<uint32, false>
  {
  public:
    ~CanMakeCheckedPtr() noexcept
    {
      static_assert(T::_delete_overridden_for_checkedptr,
                    "Objects that use CanMakeCheckedPtr must use KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR");
    }

    friend bool operator==(const CanMakeCheckedPtr &, const CanMakeCheckedPtr &) noexcept
    {
      static_assert(DefaultOperatorEqual == DefaultedOperatorEqual::Yes,
                    "Derived class should opt-in when defaulting operator==, or invalid/undefined comparison "
                    "should be reworked/defined");
      return true;
    }
  };

  template <typename T, DefaultedOperatorEqual DefaultOperatorEqual = DefaultedOperatorEqual::No>
  class CanMakeThreadSafeCheckedPtr : public CanMakeCheckedPtrBase<uint32, true>
  {
  public:
    ~CanMakeThreadSafeCheckedPtr() noexcept
    {
      static_assert(T::_delete_overridden_for_checkedptr,
                    "Objects that use CanMakeCheckedPtr must use KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR");
    }

    friend bool operator==(const CanMakeThreadSafeCheckedPtr &, const CanMakeThreadSafeCheckedPtr &) noexcept
    {
      static_assert(DefaultOperatorEqual == DefaultedOperatorEqual::Yes,
                    "Derived class should opt-in when defaulting operator==, or invalid/undefined comparison "
                    "should be reworked/defined");

      return true;
    }
  };

/// @brief delete(T*, std::destroying_delete_t, size_t) is preferred over delete(void*)
/// in overload resolution, so we can use it to interpose before calling delete(void*).
/// @note KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR must be declared in every subclass.
#define KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(T)                                                         \
  void operator delete(T *object, std::destroying_delete_t, size_t size)                                     \
  {                                                                                                          \
    assert(sizeof(T) == size);                                                                               \
                                                                                                             \
    object->SetDidBeginCheckedPtrDeletion();                                                                 \
                                                                                                             \
    /* Run destructor manually */                                                                            \
    object->T::~T();                                                                                         \
                                                                                                             \
    /* If CheckedPtrs still exist, poison and keep memory */                                                 \
    if (object->CheckedPtrCount()) KRYS_UNLIKELY                                                             \
    {                                                                                                        \
      ::Krys::ByteUtils::ZeroObject(*object);                                                                \
      return;                                                                                                \
    }                                                                                                        \
                                                                                                             \
    /* Free memory WITHOUT re-entering delete */                                                             \
    ::operator delete(static_cast<void *>(object));                                                          \
  }                                                                                                          \
  constexpr inline static bool _delete_overridden_for_checkedptr = true;

/// @note KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR must be declared in the most derived subclass.
#define KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ClassName)                                                      \
public:                                                                                                      \
  KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(ClassName)                                                       \
private:                                                                                                     \
  using _forceSemicolonAfterKrysOverrideDeleteCheckedPtr = int

#define KRYS_STRUCT_OVERRIDE_DELETE_FOR_CHECKED_PTR(ClassName)                                               \
  KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(ClassName)                                                       \
  using _forceSemicolonKrysOverrideDeleteCheckedPtr = int
}
