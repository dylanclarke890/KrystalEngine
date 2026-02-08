#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Detection/Sanitizers.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include <atomic>

namespace Krys
{
#define KRYS_USING_CAN_MAKE_CHECKEDPTR(Base)                                                                 \
  using Base::CheckedPtrCount;                                                                               \
  using Base::AddRefCheckedPtr;                                                                              \
  using Base::SubRefCheckedPtr

  template <typename T, typename PtrTraits>
  class CheckedPtr;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  class CheckedRef
  {
  public:
    enum AdoptTag
    {
      Adopt
    };

    CheckedRef(T &object) noexcept : _ptr(&object)
    {
      PtrTraits::unwrap(_ptr)->AddRefCheckedPtr();
    }

    CheckedRef(T &object, AdoptTag) noexcept : _ptr(&object)
    {
    }

    CheckedRef(HashTableDeletedValueType) noexcept : _ptr(PtrTraits::HashTableDeletedValue())
    {
    }

    CheckedRef(HashTableEmptyValueType) noexcept : _ptr(GetHashTableEmptyValue())
    {
    }

    KRYS_ALWAYS_INLINE CheckedRef(const CheckedRef &other) noexcept : _ptr {PtrTraits::unwrap(other._ptr)}
    {
      auto *ptr = PtrTraits::unwrap(_ptr);
      ptr->AddRefCheckedPtr();
    }

    template <typename TOther, typename TOtherPtrTraits>
    CheckedRef(const CheckedRef<TOther, TOtherPtrTraits> &other) noexcept
        : _ptr {PtrTraits::unwrap(other._ptr)}
    {
      auto *ptr = PtrTraits::unwrap(_ptr);
      ptr->AddRefCheckedPtr();
    }

    KRYS_ALWAYS_INLINE CheckedRef(CheckedRef &&other) noexcept : _ptr {other.ReleasePtr()}
    {
      assert(_ptr);
    }

    template <typename TOther, typename TOtherPtrTraits>
    CheckedRef(CheckedRef<TOther, TOtherPtrTraits> &&other) noexcept : _ptr {other.ReleasePtr()}
    {
      assert(_ptr);
    }

    ~CheckedRef() noexcept
    {
      unpoison(*this);
      if (auto *ptr = PtrTraits::exchange(_ptr, nullptr))
      {
        PtrTraits::unwrap(ptr)->SubRefCheckedPtr();
      }
    }

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return PtrTraits::IsHashTableDeletedValue(_ptr);
    }

    KRYS_NODISCARD bool IsHashTableEmptyValue() const noexcept
    {
      return _ptr == GetHashTableEmptyValue();
    }

    KRYS_NODISCARD static RawPtr<T> GetHashTableEmptyValue() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD const RawPtr<T> ptrAllowingHashTableEmptyValue() const noexcept
    {
      assert(_ptr || IsHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD RawPtr<T> ptrAllowingHashTableEmptyValue() noexcept
    {
      assert(_ptr || IsHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE RawPtr<T> ptr() const noexcept
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero CheckedPtrCount().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check CheckedPtrCount() here, we're checking for a scribbled
      // object.
      assert(PtrTraits::unwrap(_ptr)->CheckedPtrCount());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE T &get() const noexcept
    {
      assert(_ptr);
      return *ptr();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE RawPtr<T> operator->() const noexcept
    {
      assert(_ptr);
      return ptr();
    }

    KRYS_NODISCARD KRYS_ALWAYS_INLINE operator T &() const noexcept
    {
      return get();
    }
    KRYS_ALWAYS_INLINE explicit operator bool() const noexcept
    {
      return ptr();
    }

    CheckedRef &operator=(T &reference) noexcept
    {
      unpoison(*this);
      CheckedRef copy {reference};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedRef &operator=(const CheckedRef &other) noexcept
    {
      unpoison(*this);
      CheckedRef copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef &operator=(const CheckedRef<OtherType, OtherPtrTraits> &other) noexcept
    {
      unpoison(*this);
      CheckedRef copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedRef &operator=(CheckedRef &&other) noexcept
    {
      unpoison(*this);
      CheckedRef moved {Krys::Move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef &operator=(CheckedRef<OtherType, OtherPtrTraits> &&other) noexcept
    {
      unpoison(*this);
      CheckedRef moved {Krys::Move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

  private:
    template <typename OtherType, typename OtherPtrTraits>
    friend class CheckedRef;
    template <typename OtherType, typename OtherPtrTraits>
    friend class CheckedPtr;

    KRYS_NODISCARD RawPtr<T> ReleasePtr() noexcept
    {
      RawPtr<T> ptr = PtrTraits::exchange(_ptr, nullptr);
      poison(*this);
      return ptr;
    }

#if KRYS_ASAN_ENABLED
    template <typename ObjectType>
    void poison(ObjectType &object)
    {
      __asan_poison_memory_region(&object, sizeof(ObjectType));
    }

    template <typename ObjectType>
    void unpoison(ObjectType &object)
    {
      if (__asan_address_is_poisoned(&object))
        __asan_unpoison_memory_region(&object, sizeof(ObjectType));
    }
#else
    template <typename ObjectType>
    void poison(ObjectType &) noexcept
    {
    }
    template <typename ObjectType>
    void unpoison(ObjectType &) noexcept
    {
    }
#endif

    typename PtrTraits::storage_type _ptr;
  };

  template <typename T, typename PtrTraits>
  struct GetPtrHelper<CheckedRef<T, PtrTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const CheckedRef<T, PtrTraits> &p) noexcept
    {
      return const_cast<pointer_type>(p.ptr());
    }
  };

  template <typename T, typename U>
  struct IsSmartPtr<CheckedRef<T, U>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline bool Is(CheckedRef<TArg, TArgPtrTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline bool Is(const CheckedRef<TArg, TArgPtrTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline TExpected &Downcast(CheckedRef<TArg, TArgPtrTraits> &source) noexcept
  {
    return Downcast<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline TExpected &Downcast(const CheckedRef<TArg, TArgPtrTraits> &source) noexcept
  {
    return Downcast<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline const TExpected &Downcast(CheckedRef<const TArg, TArgPtrTraits> &source) noexcept
  {
    return Downcast<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline CheckedPtr<match_constness_t<TArg, TExpected>, TArgPtrTraits>
    DynamicDowncast(CheckedRef<TArg, TArgPtrTraits> &source) noexcept
  {
    return DynamicDowncast<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline CheckedPtr<match_constness_t<TArg, TExpected>, TArgPtrTraits>
    DynamicDowncast(const CheckedRef<TArg, TArgPtrTraits> &source) noexcept
  {
    return DynamicDowncast<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  KRYS_NODISCARD inline const CheckedPtr<match_constness_t<TArg, TExpected>, TArgPtrTraits>
    DynamicDowncast(CheckedRef<const TArg, TArgPtrTraits> &source) noexcept
  {
    return DynamicDowncast<TExpected>(source.get());
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

    KRYS_ALWAYS_INLINE void AddRefCheckedPtr() const noexcept
    {
      ++_checkedPtrCount;
    }

    KRYS_ALWAYS_INLINE void SubRefCheckedPtr() const noexcept
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
