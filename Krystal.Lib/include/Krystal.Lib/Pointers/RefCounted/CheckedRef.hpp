#pragma once

#include "Krystal.Lib/ByteUtils.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Detection/Environment.hpp"
#include "Krystal.Lib/Detection/Sanitizers.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RawPtrTraits.hpp"
#include <atomic>

namespace Krys
{
#define KRYS_USING_CAN_MAKE_CHECKEDPTR(BASE)                                                                 \
  using BASE::checkedPtrCount;                                                                               \
  using BASE::incrementCheckedPtrCount;                                                                      \
  using BASE::decrementCheckedPtrCount

  template <typename T, typename PtrTraits>
  class CheckedPtr;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  class CheckedRef
  {
  public:
    ~CheckedRef() noexcept
    {
      unpoison(*this);
      if (auto *ptr = PtrTraits::exchange(_ptr, nullptr))
      {
        PtrTraits::unwrap(ptr)->SubRefCheckedPtr();
      }
    }

    CheckedRef(T &object) noexcept : _ptr(&object)
    {
      PtrTraits::unwrap(_ptr)->AddRefCheckedPtr();
    }

    enum AdoptTag
    {
      Adopt
    };

    CheckedRef(T &object, AdoptTag) noexcept : _ptr(&object)
    {
    }

    KRYS_ALWAYS_INLINE CheckedRef(const CheckedRef &other) noexcept : _ptr {PtrTraits::unwrap(other._ptr)}
    {
      auto *ptr = PtrTraits::unwrap(_ptr);
      ptr->AddRefCheckedPtr();
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef(const CheckedRef<OtherType, OtherPtrTraits> &other) noexcept
        : _ptr {PtrTraits::unwrap(other._ptr)}
    {
      auto *ptr = PtrTraits::unwrap(_ptr);
      ptr->incrementCheckedPtrCount();
    }

    KRYS_ALWAYS_INLINE CheckedRef(CheckedRef &&other) noexcept : _ptr {other.releasePtr()}
    {
      assert(_ptr);
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef(CheckedRef<OtherType, OtherPtrTraits> &&other) noexcept : _ptr {other.releasePtr()}
    {
      assert(_ptr);
    }

    CheckedRef(HashTableDeletedValueType) noexcept : _ptr(PtrTraits::hashTableDeletedValue())
    {
    }

    bool isHashTableDeletedValue() const noexcept
    {
      return PtrTraits::isHashTableDeletedValue(_ptr);
    }

    CheckedRef(HashTableEmptyValueType) noexcept : _ptr(hashTableEmptyValue())
    {
    }

    bool isHashTableEmptyValue() const noexcept
    {
      return _ptr == hashTableEmptyValue();
    }

    static T *hashTableEmptyValue() noexcept
    {
      return nullptr;
    }

    const T *ptrAllowingHashTableEmptyValue() const noexcept
    {
      assert(_ptr || isHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    T *ptrAllowingHashTableEmptyValue() noexcept
    {
      assert(_ptr || isHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T *ptr() const noexcept
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero checkedPtrCount().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check checkedPtrCount() here, we're checking for a scribbled
      // object.
      assert(PtrTraits::unwrap(_ptr)->checkedPtrCount());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T &get() const noexcept
    {
      assert(_ptr);
      return *ptr();
    }

    KRYS_ALWAYS_INLINE T *operator->() const noexcept
    {
      assert(_ptr);
      return ptr();
    }

    KRYS_ALWAYS_INLINE operator T &() const noexcept
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

    T *releasePtr() noexcept
    {
      T *ptr = PtrTraits::exchange(_ptr, nullptr);
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

    typename PtrTraits::StorageType _ptr;
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

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline bool Is(CheckedRef<ArgType, ArgPtrTraits> &source) noexcept
  {
    return Is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline bool Is(const CheckedRef<ArgType, ArgPtrTraits> &source) noexcept
  {
    return Is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline ExpectedType &Downcast(CheckedRef<ArgType, ArgPtrTraits> &source) noexcept
  {
    return Downcast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline ExpectedType &Downcast(const CheckedRef<ArgType, ArgPtrTraits> &source) noexcept
  {
    return Downcast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline const ExpectedType &Downcast(CheckedRef<const ArgType, ArgPtrTraits> &source) noexcept
  {
    return Downcast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline CheckedPtr<match_constness_t<ArgType, ExpectedType>, ArgPtrTraits>
    DynamicDowncast(CheckedRef<ArgType, ArgPtrTraits> &source) noexcept
  {
    return DynamicDowncast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline CheckedPtr<match_constness_t<ArgType, ExpectedType>, ArgPtrTraits>
    DynamicDowncast(const CheckedRef<ArgType, ArgPtrTraits> &source) noexcept
  {
    return DynamicDowncast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline const CheckedPtr<match_constness_t<ArgType, ExpectedType>, ArgPtrTraits>
    DynamicDowncast(CheckedRef<const ArgType, ArgPtrTraits> &source) noexcept
  {
    return DynamicDowncast<ExpectedType>(source.get());
  }

  enum class DefaultedOperatorEqual : bool
  {
    No,
    Yes
  };

  // DO NOT make use of this enum in new code. An object which supports CanMakeCheckedPtr must be heap
  // allocated on its own.
  enum class CheckedPtrDeleteCheckException : bool
  {
    No,
    Yes
  };

  template <typename StorageType, typename PtrCounterType, typename DeletionFlagType,
            CheckedPtrDeleteCheckException deleteException>
  class CanMakeCheckedPtrBase
  {
  public:
    CanMakeCheckedPtrBase() noexcept = default;
    CanMakeCheckedPtrBase(CanMakeCheckedPtrBase &&) noexcept
    {
    }
    CanMakeCheckedPtrBase &operator=(CanMakeCheckedPtrBase &&) noexcept
    {
      return *this;
    }
    CanMakeCheckedPtrBase(const CanMakeCheckedPtrBase &) noexcept
    {
    }
    CanMakeCheckedPtrBase &operator=(const CanMakeCheckedPtrBase &) noexcept
    {
      return *this;
    }

    ~CanMakeCheckedPtrBase() noexcept
    {
      assert(m_didBeginDeletion || deleteException == CheckedPtrDeleteCheckException::Yes);
    }

    PtrCounterType checkedPtrCount() const noexcept
    {
      return m_checkedPtrCount;
    }

    void AddRefCheckedPtr() const noexcept
    {
      ++m_checkedPtrCount;
    }

    KRYS_ALWAYS_INLINE void SubRefCheckedPtr() const noexcept
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero checkedPtrCount().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check checkedPtrCount() here, we're checking for a scribbled
      // object.
      if (!checkedPtrCount()) KRYS_UNLIKELY
      {
        crashDueToCheckedPtrToDeadObject();
      }
      --m_checkedPtrCount;
    }

    void setDidBeginCheckedPtrDeletion() noexcept
    {
#if KRYS_ENV(DEV)
      m_didBeginDeletion = true;
#endif
    }

  private:
    KRYS_NORETURN KRYS_NEVER_INLINE static void crashDueToCheckedPtrToDeadObject() noexcept
    {
      assert(false);
    }

    mutable StorageType m_checkedPtrCount {0};
#if KRYS_ENV(DEV)
    DeletionFlagType m_didBeginDeletion {false};
#endif
  };

  template <typename T, DefaultedOperatorEqual defaultedOperatorEqual = DefaultedOperatorEqual::No,
            CheckedPtrDeleteCheckException deleteException = CheckedPtrDeleteCheckException::No>
  class CanMakeCheckedPtr : public CanMakeCheckedPtrBase<uint32_t, uint32_t, bool, deleteException>
  {
  public:
    ~CanMakeCheckedPtr() noexcept
    {
      static_assert(SameType<typename T::KrysDidOverrideDeleteForCheckedPtr, int>,
                    "Objects that use CanMakeCheckedPtr must use KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR");
    }

    friend bool operator==(const CanMakeCheckedPtr &, const CanMakeCheckedPtr &)
    {
      static_assert(defaultedOperatorEqual == DefaultedOperatorEqual::Yes,
                    "Derived class should opt-in when defaulting operator==, or invalid/undefined comparison "
                    "should be reworked/defined");
      return true;
    }
  };

  template <typename T, DefaultedOperatorEqual defaultedOperatorEqual = DefaultedOperatorEqual::No,
            CheckedPtrDeleteCheckException deleteException = CheckedPtrDeleteCheckException::No>
  class CanMakeThreadSafeCheckedPtr
      : public CanMakeCheckedPtrBase<std::atomic<uint32_t>, uint32_t, std::atomic<bool>, deleteException>
  {
  public:
    ~CanMakeThreadSafeCheckedPtr()
    {
      static_assert(SameType<typename T::WTFDidOverrideDeleteForCheckedPtr, int>,
                    "Objects that use CanMakeCheckedPtr must use KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR");
    }

    friend bool operator==(const CanMakeThreadSafeCheckedPtr &, const CanMakeThreadSafeCheckedPtr &)
    {
      static_assert(defaultedOperatorEqual == DefaultedOperatorEqual::Yes,
                    "Derived class should opt-in when defaulting operator==, or invalid/undefined comparison "
                    "should be reworked/defined");

      return true;
    }
  };

// delete(T*, std::destroying_delete_t, size_t) is preferred over delete(void*)
// in overload resolution, so we can use it to interpose before calling delete(void*).
// Note: KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR must be declared in every subclass.
#define KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(T)                                                         \
  void operator delete(T *object, std::destroying_delete_t, size_t size)                                     \
  {                                                                                                          \
    assert(sizeof(T) == size);                                                                               \
                                                                                                             \
    object->setDidBeginCheckedPtrDeletion();                                                                 \
                                                                                                             \
    /* Run destructor manually */                                                                            \
    object->T::~T();                                                                                         \
                                                                                                             \
    /* If CheckedPtrs still exist, poison and keep memory */                                                 \
    if (object->checkedPtrCount()) KRYS_UNLIKELY                                                             \
    {                                                                                                        \
      ByteUtils::ZeroObject(*object);                                                                        \
      return;                                                                                                \
    }                                                                                                        \
                                                                                                             \
    /* Free memory WITHOUT re-entering delete */                                                             \
    ::operator delete(static_cast<void *>(object));                                                          \
  }                                                                                                          \
  using KrysDidOverrideDeleteForCheckedPtr = int;

  // Note: KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR must be declared in the most derived subclass.
#define KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ClassName)                                                      \
public:                                                                                                      \
  KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(ClassName)                                                       \
private:                                                                                                     \
  using _forceSemicolonAfterKrysOverrideDelete = int

#define KRYS_STRUCT_OVERRIDE_DELETE_FOR_CHECKED_PTR(ClassName)                                               \
  KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR_IMPL(ClassName)                                                       \
  using _forceSemicolonAfterKrysOverrideDelete = int
}
