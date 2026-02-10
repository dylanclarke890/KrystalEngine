#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include <atomic>
#include <cassert>

namespace Krys
{
  template <typename T>
  struct CheckedRefDerefTraits
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefChecked();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      ref.AddRefChecked();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefChecked();
      }
    }
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class CheckedPtr;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  class CheckedRef
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename>
    friend class CheckedRef;

  public:
    using pointer = typename PtrTraits::storage_type;
    using element_type = T;
    constexpr static bool nullable = false;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr CheckedRef NoRef(T &ref) noexcept
    {
      return CheckedRef(ref);
    }

    KRYS_NODISCARD static constexpr CheckedRef WithRef(T &ref) noexcept
    {
      return CheckedRef(RefDerefTraits::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE CheckedRef(const CheckedRef &o) noexcept : _ptr(RefDerefTraits::AddRef(o.get()))
    {
      assert(_ptr);
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr CheckedRef(const CheckedRef<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
      assert(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr CheckedRef(CheckedRef &&o) noexcept : _ptr(o.release())
    {
      assert(_ptr);
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr CheckedRef(CheckedRef<X, Y, Z> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      assert(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr ~CheckedRef() noexcept
    {
      reset();
    }

    CheckedRef &operator=(const CheckedRef &o) noexcept
    {
      CheckedRef ref = o;
      swap(ref);
      assert(_ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr CheckedRef &operator=(const CheckedRef<X, Y, Z> &o) noexcept
    {
      CheckedRef ref = o;
      swap(ref);
      assert(_ptr);
      return *this;
    }

    constexpr CheckedRef &operator=(CheckedRef &&o) noexcept
    {
      CheckedRef ref = Krys::Move(o);
      swap(ref);
      assert(_ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr CheckedRef &operator=(CheckedRef<X, Y, Z> &&o) noexcept
    {
      CheckedRef ref = Krys::Move(o);
      swap(ref);
      assert(_ptr);
      return *this;
    }

    KRYS_ALWAYS_INLINE constexpr T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }

    template <typename TMember>
    constexpr TMember &operator->*(TMember T::*memptr) const noexcept
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr)->*memptr;
    }

    constexpr bool operator!() const noexcept
    {
      return !_ptr;
    }

    explicit constexpr operator bool() const noexcept
    {
      return !!_ptr;
    }

    KRYS_NODISCARD constexpr RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND KRYS_RETURNS_NONNULL
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero CheckedPtrCount().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check CheckedPtrCount() here, we're checking for a scribbled
      // object.
      assert(PtrTraits::unwrap(_ptr)->CheckedPtrCount());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD constexpr RawPtr<T> release() noexcept
    {
      return PtrTraits::exchange(_ptr, nullptr);
    }

    KRYS_ALWAYS_INLINE constexpr void reset() noexcept
    {
      RefDerefTraits::SubRef(PtrTraits::exchange(_ptr, nullptr));
    }

    KRYS_ALWAYS_INLINE constexpr void swap(CheckedRef &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(CheckedRef &a, CheckedRef &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr CheckedRef(T &object) noexcept : _ptr(&object)
    {
    }
  };

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  constexpr inline bool operator==(const CheckedRef<T, U, V> &a, const CheckedRef<X, Y, Z> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const CheckedRef<T, U, V> &a, RawPtr<X> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const CheckedRef<T, U, V> &a, std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits>
  struct GetPtrHelper<CheckedRef<T, PtrTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const CheckedRef<T, PtrTraits> &ptr) noexcept
    {
      return const_cast<pointer_type>(ptr.get());
    }
  };

  template <typename T, typename U>
  struct IsSmartPtr<CheckedRef<T, U>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits, RefDerefTraits> CreateCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits, RefDerefTraits> AdoptCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits, RefDerefTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedRef<T, PtrTraits, RefDerefTraits> ShareCheckedRef(T &ptr) noexcept
  {
    return CheckedRef<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
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
