#pragma once

#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted/InlineWeakRef.hpp"
#include <bit>

namespace Krys
{
  template <typename T>
  class InlineWeakPtr
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename X>
    friend class InlineWeakPtr;

  private:
    RawPtr<T> _ptr;

  public:
    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr() noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr(std::nullptr_t) noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr(RawPtr<T> ptr) noexcept : _ptr(AddRefWeak(ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr(T &ptr) noexcept : _ptr(&AddRefWeak(ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr(const InlineWeakPtr &o) noexcept : _ptr(AddRefWeak(o._ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr ~InlineWeakPtr() noexcept
    {
      SubRefWeak(_ptr);
    }

    template <typename X>
    constexpr InlineWeakPtr(const InlineWeakPtr<X> &o) noexcept : _ptr(SubRefWeak(o._ptr))
    {
    }

    KRYS_ALWAYS_INLINE constexpr InlineWeakPtr(InlineWeakPtr &&o) noexcept : _ptr(o.LeakWeak())
    {
    }

    template <typename X>
    constexpr InlineWeakPtr(InlineWeakPtr<X> &&o) noexcept : _ptr(o.LeakWeak())
    {
    }

    constexpr static RawPtr<T> GetHashTableDeletedValue() noexcept
    {
      return std::bit_cast<RawPtr<T>>(static_cast<uintptr_t>(-1));
    }

    constexpr InlineWeakPtr(HashTableDeletedValueType) noexcept : _ptr(GetHashTableDeletedValue())
    {
    }

    constexpr InlineWeakPtr(HashTableEmptyValueType) noexcept : _ptr(nullptr)
    {
    }

    constexpr bool IsHashTableDeletedValue() const noexcept
    {
      return _ptr == GetHashTableDeletedValue();
    }

    constexpr bool IsHashTableEmptyValue() const noexcept
    {
      return !_ptr;
    }

    constexpr bool IsWeakNullValue() const noexcept
    {
      return !_ptr->GetRefCount();
    }

    constexpr RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
    {
      if (!_ptr || !_ptr->GetRefCount())
      {
        return nullptr;
      }

      return _ptr;
    }

    constexpr KRYS_NODISCARD RawPtr<T> LeakWeak() noexcept
    {
      return std::exchange(_ptr, nullptr);
    }

    constexpr T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(this->get());
      return *this->get();
    }

    constexpr KRYS_ALWAYS_INLINE RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      return get();
    }

    constexpr bool operator!() const noexcept
    {
      return !_ptr || !_ptr->GetRefCount();
    }

    explicit constexpr operator bool() const noexcept
    {
      return _ptr && _ptr->GetRefCount();
    }

    constexpr InlineWeakPtr &operator=(RawPtr<T> optr) noexcept
    {
      InlineWeakPtr ptr = optr;
      swap(ptr);
      return *this;
    }

    constexpr InlineWeakPtr &operator=(std::nullptr_t) noexcept
    {
      SubRefWeak(std::exchange(_ptr, nullptr));
      return *this;
    }

    constexpr InlineWeakPtr &operator=(const InlineWeakPtr &o) noexcept
    {
      InlineWeakPtr ptr = o;
      swap(ptr);
      return *this;
    }

    constexpr InlineWeakPtr &operator=(InlineWeakPtr &&o) noexcept
    {
      InlineWeakPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X>
    constexpr void swap(InlineWeakPtr<X> &) noexcept
    {
      std::swap(_ptr, o._ptr);
    }
  };

  template <typename T, typename U>
  constexpr inline bool operator==(const InlineWeakPtr<T> &a, const InlineWeakPtr<U> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U>
  constexpr inline bool operator==(const InlineWeakPtr<T> &a, RawPtr<U> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T>
  struct GetPtrHelper<InlineWeakPtr<T>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    constexpr static pointer_type GetPtr(const InlineWeakPtr<T> &p) noexcept
    {
      return const_cast<pointer_type>(p.get());
    }
  };

  template <typename T>
  struct IsSmartPtr<InlineWeakPtr<T>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };
}
