#pragma once

#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RawPtrTraits.hpp"
#include <atomic>
#include <wtf/SingleThreadIntegralWrapper.h>

#if ASSERT_ENABLED
  #include <wtf/Threading.h>
#endif

namespace Krys
{

#define USING_CAN_MAKE_CHECKEDPTR(BASE)                                                                      \
  using BASE::checkedPtrCount;                                                                               \
  using BASE::checkedPtrCountWithoutThreadCheck;                                                             \
  using BASE::incrementCheckedPtrCount;                                                                      \
  using BASE::decrementCheckedPtrCount

  template <typename T, typename PtrTraits>
  class CheckedRef
  {
    WTF_DEPRECATED_MAKE_FAST_ALLOCATED(CheckedRef);

  public:
    ~CheckedRef()
    {
      unpoison(*this);
      if (auto *ptr = PtrTraits::exchange(_ptr, nullptr))
        PtrTraits::unwrap(ptr)->decrementCheckedPtrCount();
    }

    CheckedRef(T &object) : _ptr(&object)
    {
      PtrTraits::unwrap(_ptr)->incrementCheckedPtrCount();
    }

    enum AdoptTag
    {
      Adopt
    };
    CheckedRef(T &object, AdoptTag) : _ptr(&object)
    {
    }

    KRYS_ALWAYS_INLINE CheckedRef(const CheckedRef &other) : _ptr {PtrTraits::unwrap(other._ptr)}
    {
      auto *ptr = PtrTraits::unwrap(_ptr);
      ptr->incrementCheckedPtrCount();
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef(const CheckedRef<OtherType, OtherPtrTraits> &other) : _ptr {PtrTraits::unwrap(other._ptr)}
    {
      auto *ptr = PtrTraits::unwrap(_ptr);
      ptr->incrementCheckedPtrCount();
    }

    KRYS_ALWAYS_INLINE CheckedRef(CheckedRef &&other) : _ptr {other.releasePtr()}
    {
      assert(_ptr);
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef(CheckedRef<OtherType, OtherPtrTraits> &&other) : _ptr {other.releasePtr()}
    {
      assert(_ptr);
    }

    CheckedRef(HashTableDeletedValueType) : _ptr(PtrTraits::hashTableDeletedValue())
    {
    }
    bool isHashTableDeletedValue() const
    {
      return PtrTraits::isHashTableDeletedValue(_ptr);
    }

    CheckedRef(HashTableEmptyValueType) : _ptr(hashTableEmptyValue())
    {
    }
    bool isHashTableEmptyValue() const
    {
      return _ptr == hashTableEmptyValue();
    }
    static T *hashTableEmptyValue()
    {
      return nullptr;
    }

    const T *ptrAllowingHashTableEmptyValue() const
    {
      assert(_ptr || isHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }
    T *ptrAllowingHashTableEmptyValue()
    {
      assert(_ptr || isHashTableEmptyValue());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T *ptr() const
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero checkedPtrCount().
      // When it detects a dangling pointer, WTF_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check checkedPtrCountWithoutThreadCheck() here, we're checking for
      // a scribbled object.
      assert(PtrTraits::unwrap(_ptr)->checkedPtrCountWithoutThreadCheck());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T &get() const
    {
      assert(_ptr);
      return *ptr();
    }

    KRYS_ALWAYS_INLINE T *operator->() const
    {
      assert(_ptr);
      return ptr();
    }

    KRYS_ALWAYS_INLINE operator T &() const
    {
      return get();
    }
    KRYS_ALWAYS_INLINE explicit operator bool() const
    {
      return ptr();
    }

    CheckedRef &operator=(T &reference)
    {
      unpoison(*this);
      CheckedRef copy {reference};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedRef &operator=(const CheckedRef &other)
    {
      unpoison(*this);
      CheckedRef copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef &operator=(const CheckedRef<OtherType, OtherPtrTraits> &other)
    {
      unpoison(*this);
      CheckedRef copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedRef &operator=(CheckedRef &&other)
    {
      unpoison(*this);
      CheckedRef moved {WTF::move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedRef &operator=(CheckedRef<OtherType, OtherPtrTraits> &&other)
    {
      unpoison(*this);
      CheckedRef moved {WTF::move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

  private:
    template <typename OtherType, typename OtherPtrTraits>
    friend class CheckedRef;
    template <typename OtherType, typename OtherPtrTraits>
    friend class CheckedPtr;

    T *releasePtr()
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
    void poison(ObjectType &)
    {
    }
    template <typename ObjectType>
    void unpoison(ObjectType &)
    {
    }
#endif

    typename PtrTraits::StorageType _ptr;
  };

  template <typename T, typename PtrTraits>
  struct GetPtrHelper<CheckedRef<T, PtrTraits>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const CheckedRef<T, PtrTraits> &p)
    {
      return const_cast<T *>(p.ptr());
    }
  };

  template <typename T, typename U>
  struct IsSmartPtr<CheckedRef<T, U>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = true;
  };

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline bool is(CheckedRef<ArgType, ArgPtrTraits> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline bool is(const CheckedRef<ArgType, ArgPtrTraits> &source)
  {
    return is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline ExpectedType &downcast(CheckedRef<ArgType, ArgPtrTraits> &source)
  {
    return downcast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline ExpectedType &downcast(const CheckedRef<ArgType, ArgPtrTraits> &source)
  {
    return downcast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline const ExpectedType &downcast(CheckedRef<const ArgType, ArgPtrTraits> &source)
  {
    return downcast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline CheckedPtr<match_constness_t<ArgType, ExpectedType>>
    dynamicDowncast(CheckedRef<ArgType, ArgPtrTraits> &source)
  {
    return dynamicDowncast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline CheckedPtr<match_constness_t<ArgType, ExpectedType>>
    dynamicDowncast(const CheckedRef<ArgType, ArgPtrTraits> &source)
  {
    return dynamicDowncast<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline const CheckedPtr<match_constness_t<ArgType, ExpectedType>>
    dynamicDowncast(CheckedRef<const ArgType, ArgPtrTraits> &source)
  {
    return dynamicDowncast<ExpectedType>(source.get());
  }

  template <typename P>
  struct CheckedRefHashTraits : SimpleClassHashTraits<CheckedRef<P>>
  {
    static constexpr bool emptyValueIsZero = true;
    static CheckedRef<P> emptyValue()
    {
      return HashTableEmptyValue;
    }

    template <typename>
    static void constructEmptyValue(CheckedRef<P> &slot)
    {
      new (NotNull, std::addressof(slot)) CheckedRef<P>(HashTableEmptyValue);
    }

    static constexpr bool hasIsEmptyValueFunction = true;
    static bool isEmptyValue(const CheckedRef<P> &value)
    {
      return value.isHashTableEmptyValue();
    }

    using PeekType = P *;
    static PeekType peek(const CheckedRef<P> &value)
    {
      return const_cast<PeekType>(value.ptrAllowingHashTableEmptyValue());
    }
    static PeekType peek(P *value)
    {
      return value;
    }

    using TakeType = CheckedPtr<P>;
    static TakeType take(CheckedRef<P> &&value)
    {
      return isEmptyValue(value) ? nullptr : CheckedPtr<P>(WTF::move(value));
    }
  };

  template <typename P>
  struct HashTraits<CheckedRef<P>> : CheckedRefHashTraits<P>
  {
  };

  template <typename P>
  struct PtrHash<CheckedRef<P>> : PtrHashBase<CheckedRef<P>, IsSmartPtr<CheckedRef<P>>::value>
  {
    static constexpr bool safeToCompareToEmptyOrDeleted = false;
  };

  template <typename P>
  struct DefaultHash<CheckedRef<P>> : PtrHash<CheckedRef<P>>
  {
  };

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
    CanMakeCheckedPtrBase() = default;
    CanMakeCheckedPtrBase(CanMakeCheckedPtrBase &&)
    {
    }
    CanMakeCheckedPtrBase &operator=(CanMakeCheckedPtrBase &&)
    {
      return *this;
    }
    CanMakeCheckedPtrBase(const CanMakeCheckedPtrBase &)
    {
    }
    CanMakeCheckedPtrBase &operator=(const CanMakeCheckedPtrBase &)
    {
      return *this;
    }

    ~CanMakeCheckedPtrBase()
    {
      assert(m_didBeginDeletion
                                       || deleteException == CheckedPtrDeleteCheckException::Yes);
    }

    PtrCounterType checkedPtrCount() const
    {
      return m_checkedPtrCount;
    }
    void incrementCheckedPtrCount() const
    {
      ++m_checkedPtrCount;
    }
    KRYS_ALWAYS_INLINE void decrementCheckedPtrCount() const
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero checkedPtrCount().
      // When it detects a dangling pointer, WTF_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check checkedPtrCountWithoutThreadCheck() here, we're checking for
      // a scribbled object.
      if (!checkedPtrCountWithoutThreadCheck()) [[unlikely]]
        crashDueToCheckedPtrToDeadObject();
      --m_checkedPtrCount;
    }

    KRYS_ALWAYS_INLINE PtrCounterType checkedPtrCountWithoutThreadCheck() const
    {
      if constexpr (std::is_same_v<StorageType, std::atomic<uint32_t>>)
        return m_checkedPtrCount;
      else
        return m_checkedPtrCount.valueWithoutThreadCheck();
    }

    void setDidBeginCheckedPtrDeletion()
    {
#if ASSERT_ENABLED || ENABLE(SECURITY_ASSERTIONS)
      m_didBeginDeletion = true;
#endif
    }

  private:
    static NO_RETURN_DUE_TO_CRASH NEVER_INLINE void crashDueToCheckedPtrToDeadObject()
    {
      CRASH();
    }

    mutable StorageType m_checkedPtrCount {0};
#if ASSERT_ENABLED || ENABLE(SECURITY_ASSERTIONS)
    DeletionFlagType m_didBeginDeletion {false};
#endif
  };

  template <typename T, DefaultedOperatorEqual defaultedOperatorEqual = DefaultedOperatorEqual::No,
            CheckedPtrDeleteCheckException deleteException = CheckedPtrDeleteCheckException::No>
  class CanMakeCheckedPtr
      : public CanMakeCheckedPtrBase<SingleThreadIntegralWrapper<uint32_t>, uint32_t, bool, deleteException>
  {
  public:
    ~CanMakeCheckedPtr()
    {
      static_assert(
        std::is_same<typename T::WTFIsFastMallocAllocated, int>::value,
        "Objects that use CanMakeCheckedPtr must use FastMalloc (WTF_DEPRECATED_MAKE_FAST_ALLOCATED)");
      static_assert(std::is_same<typename T::WTFDidOverrideDeleteForCheckedPtr, int>::value,
                    "Objects that use CanMakeCheckedPtr must use WTF_OVERRIDE_DELETE_FOR_CHECKED_PTR");
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
      static_assert(
        std::is_same<typename T::WTFIsFastMallocAllocated, int>::value,
        "Objects that use CanMakeCheckedPtr must use FastMalloc (WTF_DEPRECATED_MAKE_FAST_ALLOCATED)");
      static_assert(std::is_same<typename T::WTFDidOverrideDeleteForCheckedPtr, int>::value,
                    "Objects that use CanMakeCheckedPtr must use WTF_OVERRIDE_DELETE_FOR_CHECKED_PTR");
    }

    friend bool operator==(const CanMakeThreadSafeCheckedPtr &, const CanMakeThreadSafeCheckedPtr &)
    {
      static_assert(defaultedOperatorEqual == DefaultedOperatorEqual::Yes,
                    "Derived class should opt-in when defaulting operator==, or invalid/undefined comparison "
                    "should be reworked/defined");
      return true;
    }
  };

} 

using WTF::CanMakeCheckedPtr;
using WTF::CanMakeThreadSafeCheckedPtr;
using WTF::CheckedRef;
