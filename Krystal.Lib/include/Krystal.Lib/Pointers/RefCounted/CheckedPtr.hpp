#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/CheckedRef.hpp"
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
  class CheckedPtr
  {
  public:
    constexpr CheckedPtr() noexcept : _ptr(nullptr)
    {
    }

    constexpr CheckedPtr(std::nullptr_t) noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE CheckedPtr(RawPtr<T> ptr) noexcept : _ptr {ptr}
    {
      AddRefIfNotNull();
    }

    KRYS_ALWAYS_INLINE CheckedPtr(const CheckedPtr &other) noexcept : _ptr {other._ptr}
    {
      AddRefIfNotNull();
    }

    KRYS_ALWAYS_INLINE CheckedPtr(CheckedPtr &&other) noexcept
        : _ptr {PtrTraits::exchange(other._ptr, nullptr)}
    {
    }

    KRYS_ALWAYS_INLINE ~CheckedPtr() noexcept
    {
      SubRefIfNotNull();
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(const CheckedPtr<OtherType, OtherPtrTraits> &other) noexcept
        : CheckedPtr(OtherPtrTraits::unwrap(other._ptr))
    {
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(CheckedPtr<OtherType, OtherPtrTraits> &&other) noexcept
        : _ptr {OtherPtrTraits::exchange(other._ptr, nullptr)}
    {
    }

    CheckedPtr(CheckedRef<T, PtrTraits> &other) noexcept : CheckedPtr(PtrTraits::unwrap(other._ptr))
    {
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(const CheckedRef<OtherType, OtherPtrTraits> &other) noexcept
        : CheckedPtr(OtherPtrTraits::unwrap(other._ptr))
    {
    }

    CheckedPtr(CheckedRef<T, PtrTraits> &&other) noexcept : _ptr {other.releasePtr()}
    {
      assert(get());
    }

    template <typename OtherType, typename OtherPtrTraits>
    CheckedPtr(CheckedRef<OtherType, OtherPtrTraits> &&other) noexcept : _ptr {other.releasePtr()}
    {
      assert(get());
    }

    CheckedPtr(HashTableDeletedValueType) noexcept : _ptr(PtrTraits::HashTableDeletedValue())
    {
    }

    KRYS_NODISCARD bool IsHashTableDeletedValue() const noexcept
    {
      return PtrTraits::IsHashTableDeletedValue(_ptr);
    }

    KRYS_ALWAYS_INLINE explicit operator bool() const noexcept
    {
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
    {
      return PtrTraits::unwrap(_ptr);
    }

    CheckedRef<T, PtrTraits> ReleaseNonNull() noexcept
    {
      assert(_ptr);
      auto &ptr = *PtrTraits::unwrap(std::exchange(_ptr, nullptr));
      return CheckedRef {ptr, CheckedRef<T>::Adopt};
    }

    KRYS_ALWAYS_INLINE T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return *get();
    }

    KRYS_ALWAYS_INLINE RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(_ptr);
      return PtrTraits::unwrap(_ptr);
    }

    bool operator==(const RawPtr<T> other) const noexcept
    {
      return _ptr == other;
    }

    template <typename U>
    bool operator==(RawPtr<U> other) const noexcept
    {
      return _ptr == other;
    }

    bool operator==(const CheckedPtr &other) const noexcept
    {
      return _ptr == other._ptr;
    }

    template <typename TOther, typename TOtherPtrTraits>
    bool operator==(const CheckedPtr<TOther, TOtherPtrTraits> &other) const noexcept
    {
      return _ptr == other._ptr;
    }

    CheckedPtr &operator=(std::nullptr_t) noexcept
    {
      SubRefIfNotNull();
      _ptr = nullptr;
      return *this;
    }

    CheckedPtr &operator=(RawPtr<T> ptr) noexcept
    {
      CheckedPtr copy {ptr};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedPtr &operator=(const CheckedPtr &other) noexcept
    {
      CheckedPtr copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    template <typename TOther, typename TOtherPtrTraits>
    CheckedPtr &operator=(const CheckedPtr<TOther, TOtherPtrTraits> &other) noexcept
    {
      CheckedPtr copy {other};
      PtrTraits::swap(_ptr, copy._ptr);
      return *this;
    }

    CheckedPtr &operator=(CheckedPtr &&other) noexcept
    {
      CheckedPtr moved {Krys::Move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

    template <typename TOther, typename TOtherPtrTraits>
    CheckedPtr &operator=(CheckedPtr<TOther, TOtherPtrTraits> &&other) noexcept
    {
      CheckedPtr moved {Krys::Move(other)};
      PtrTraits::swap(_ptr, moved._ptr);
      return *this;
    }

  private:
    template <typename TOther, typename TOtherPtrTraits>
    friend class CheckedPtr;

    KRYS_ALWAYS_INLINE void AddRefIfNotNull() noexcept
    {
      if (RawPtr<T> ptr = PtrTraits::unwrap(_ptr); ptr) KRYS_LIKELY
      {
        ptr->AddRefCheckedPtr();
      }
    }

    KRYS_ALWAYS_INLINE void SubRefIfNotNull() noexcept
    {
      if (RawPtr<T> ptr = PtrTraits::unwrap(_ptr); ptr) KRYS_LIKELY
      {
        ptr->SubRefCheckedPtr();
      }
    }

    typename PtrTraits::storage_type _ptr;
  };

  template <typename T, typename PtrTraits>
  struct GetPtrHelper<CheckedPtr<T, PtrTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const CheckedPtr<T, PtrTraits> &p) noexcept
    {
      return const_cast<pointer_type>(p.get());
    }
  };

  template <typename T, typename U>
  struct IsSmartPtr<CheckedPtr<T, U>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = false;
  };

  template <typename TExpected, typename TArg, typename ArgPtrTraits>
  inline bool Is(CheckedPtr<TArg, ArgPtrTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename TExpected, typename TArg, typename TArgPtrTraits>
  inline bool Is(const CheckedPtr<TArg, TArgPtrTraits> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }
}
