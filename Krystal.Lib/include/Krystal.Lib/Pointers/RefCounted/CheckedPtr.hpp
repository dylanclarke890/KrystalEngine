#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedRef.hpp"
#include "Krystal.Lib/Pointers/RefCounted/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/RawPtrTraits.hpp"
#include <cassert>

namespace Krys
{
  /// @brief CheckedPtr is used to verify that the object being pointed to outlives the CheckedPtr.
  /// It does not affect the lifetime of the object being pointed to; it simply adds a runtime
  /// check (via assert) that when the object being pointed to is destroyed, there are
  /// no outstanding CheckedPtrs that reference it.
  /// @note Use is similar to WeakPtr, but CheckedPtr has less overhead and is used in cases where the target is
  /// never expected to become null.
  template <typename T, typename PtrTraits>
  class CheckedPtr
  {
  public:
    constexpr CheckedPtr() : _ptr(nullptr)
    {
    }

    constexpr CheckedPtr(std::nullptr_t) : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE CheckedPtr(T *ptr) : _ptr {ptr}
    {
      refIfNotNull();
    }

    KRYS_ALWAYS_INLINE CheckedPtr(const CheckedPtr &other) : _ptr {other._ptr}
    {
      refIfNotNull();
    }

    KRYS_ALWAYS_INLINE CheckedPtr(CheckedPtr &&other) : _ptr {PtrTraits::exchange(other._ptr, nullptr)}
    {
    }

    KRYS_ALWAYS_INLINE ~CheckedPtr()
    {
      derefIfNotNull();
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(const CheckedPtr<OtherType, OtherPtrTraits> &other)
        : CheckedPtr(OtherPtrTraits::unwrap(other._ptr))
    {
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(CheckedPtr<OtherType, OtherPtrTraits> &&other)
        : _ptr {OtherPtrTraits::exchange(other._ptr, nullptr)}
    {
    }

    CheckedPtr(CheckedRef<T, PtrTraits> &other) : CheckedPtr(PtrTraits::unwrap(other._ptr))
    {
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(const CheckedRef<OtherType, OtherPtrTraits> &other)
        : CheckedPtr(OtherPtrTraits::unwrap(other._ptr))
    {
    }

    CheckedPtr(CheckedRef<T, PtrTraits> &&other) : _ptr {other.releasePtr()}
    {
      assert(get());
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(CheckedRef<OtherType, OtherPtrTraits> &&other) : _ptr {other.releasePtr()}
    {
      assert(get());
    }

    CheckedPtr(HashTableDeletedValueType) : _ptr(PtrTraits::hashTableDeletedValue())
    {
    }

    bool isHashTableDeletedValue() const
    {
      return PtrTraits::isHashTableDeletedValue(_ptr);
    }

    KRYS_ALWAYS_INLINE explicit operator bool() const
    {
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T *get() const KRYS_LIFETIME_BOUND
    {
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T *unsafeGet() const
    {
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE T &operator*() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *get();
    }

    KRYS_ALWAYS_INLINE T *operator->() const KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }

    CheckedRef<T, PtrTraits> releaseNonNull()
    {
      assert(_ptr);
      auto &ptr = *PtrTraits::unwrap(std::exchange(_ptr, nullptr));
      return CheckedRef {ptr, CheckedRef<T>::Adopt};
    }

    bool operator==(const T *other) const
    {
      return _ptr == other;
    }

    template <typename U>
    bool operator==(U *other) const
    {
      return _ptr == other;
    }

    bool operator==(const CheckedPtr &other) const
    {
      return _ptr == other._ptr;
    }

    template <typename OtherType, typename OtherPtrTraits>
    bool operator==(const CheckedPtr<OtherType, OtherPtrTraits> &other) const
    {
      return _ptr == other._ptr;
    }

    CheckedPtr &operator=(std::nullptr_t)
    {
      derefIfNotNull();
      _ptr = nullptr;
      return *this;
    }

    CheckedPtr &operator=(T *ptr)
    {
      CheckedPtr copy {ptr};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedPtr &operator=(const CheckedPtr &other)
    {
      CheckedPtr copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr &operator=(const CheckedPtr<OtherType, OtherPtrTraits> &other)
    {
      CheckedPtr copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedPtr &operator=(CheckedPtr &&other)
    {
      CheckedPtr moved {Krys::Move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr &operator=(CheckedPtr<OtherType, OtherPtrTraits> &&other)
    {
      CheckedPtr moved {Krys::Move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

  private:
    template <typename OtherType, typename OtherPtrTraits>
    friend class CheckedPtr;

    KRYS_ALWAYS_INLINE void refIfNotNull()
    {
      if (T *ptr = PtrTraits::unwrap(_ptr); ptr) [[likely]]
        ptr->incrementCheckedPtrCount();
    }

    KRYS_ALWAYS_INLINE void derefIfNotNull()
    {
      if (T *ptr = PtrTraits::unwrap(_ptr); ptr) [[likely]]
        ptr->decrementCheckedPtrCount();
    }

    typename PtrTraits::StorageType _ptr;
  };

  template <typename T, typename PtrTraits>
  struct GetPtrHelper<CheckedPtr<T, PtrTraits>>
  {
    using PtrType = T *;
    using UnderlyingType = T;
    static T *getPtr(const CheckedPtr<T, PtrTraits> &p)
    {
      return const_cast<T *>(p.get());
    }
  };

  template <typename T, typename U>
  struct IsSmartPtr<CheckedPtr<T, U>>
  {
    static constexpr bool value = true;
    static constexpr bool isNullable = false;
  };

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline bool Is(CheckedPtr<ArgType, ArgPtrTraits> &source)
  {
    return Is<ExpectedType>(source.get());
  }

  template <typename ExpectedType, typename ArgType, typename ArgPtrTraits>
  inline bool Is(const CheckedPtr<ArgType, ArgPtrTraits> &source)
  {
    return Is<ExpectedType>(source.get());
  }
}
