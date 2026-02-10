#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
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
  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  class CheckedPtr
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename>
    friend class CheckedPtr;

  public:
    using pointer = typename PtrTraits::storage_type;
    using element_type = T;
    constexpr static bool nullable = true;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr CheckedPtr NoRef(RawPtr<T> ptr) noexcept
    {
      return CheckedPtr(ptr);
    }

    KRYS_NODISCARD static constexpr CheckedPtr WithRef(RawPtr<T> ptr) noexcept
    {
      return CheckedPtr(RefDerefTraits::AddRef(ptr));
    }

    KRYS_NODISCARD static constexpr CheckedPtr NoRef(T &ref) noexcept
    {
      return CheckedPtr(ref);
    }

    KRYS_NODISCARD static constexpr CheckedPtr WithRef(T &ref) noexcept
    {
      return CheckedPtr(RefDerefTraits::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE constexpr CheckedPtr() noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr CheckedPtr(std::nullptr_t) noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE CheckedPtr(const CheckedPtr &o) noexcept : _ptr(RefDerefTraits::AddRef(o.get()))
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr CheckedPtr(const CheckedPtr<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr CheckedPtr(const CheckedRef<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
    }

    KRYS_ALWAYS_INLINE constexpr CheckedPtr(CheckedPtr &&o) noexcept : _ptr(o.release())
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr CheckedPtr(CheckedPtr<X, Y, Z> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr CheckedPtr(CheckedRef<X, Y, Z> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
    }

    KRYS_ALWAYS_INLINE constexpr ~CheckedPtr() noexcept
    {
      reset();
    }

    constexpr CheckedPtr &operator=(const CheckedPtr &o) noexcept
    {
      CheckedPtr ptr = o;
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr CheckedPtr &operator=(const CheckedPtr<X, Y, Z> &o) noexcept
    {
      CheckedPtr ptr = o;
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr CheckedPtr &operator=(CheckedRef<X, Y, Z> &o) noexcept
    {
      CheckedPtr ptr = o;
      swap(ptr);
      return *this;
    }

    constexpr CheckedPtr &operator=(CheckedPtr &&o) noexcept
    {
      CheckedPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr CheckedPtr &operator=(CheckedPtr<X, Y, Z> &&o) noexcept
    {
      CheckedPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr CheckedPtr &operator=(CheckedRef<X, Y, Z> &&o) noexcept
    {
      CheckedPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    constexpr CheckedPtr &operator=(std::nullptr_t) noexcept
    {
      reset();
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

    KRYS_NODISCARD RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
    {
      // In normal execution, a CheckedPtr always points to an object with a non-zero CheckedPtrCount().
      // When it detects a dangling pointer, KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR scribbles an object with
      // zeroes and then leaks it. When we check CheckedPtrCount() here, we're checking for a scribbled
      // object.
      assert(_ptr == nullptr || PtrTraits::unwrap(_ptr)->CheckedPtrCount());
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD constexpr inline RawPtr<T> release() noexcept
    {
      return PtrTraits::exchange(_ptr, nullptr);
    }

    KRYS_ALWAYS_INLINE constexpr void reset() noexcept
    {
      RefDerefTraits::SubRef(PtrTraits::exchange(_ptr, nullptr));
    }

    constexpr void swap(CheckedPtr &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(CheckedPtr &a, CheckedPtr &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr CheckedPtr(RawPtr<T> ptr) noexcept : _ptr(ptr)
    {
    }
  };

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  constexpr inline bool operator==(const CheckedPtr<T, U, V> &a, const CheckedPtr<X, Y, Z> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const CheckedPtr<T, U, V> &a, RawPtr<X> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const CheckedPtr<T, U, V> &a, std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits>
  struct GetPtrHelper<CheckedPtr<T, PtrTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    KRYS_NODISCARD static pointer_type GetPtr(const CheckedPtr<T, PtrTraits> &ptr) noexcept
    {
      return const_cast<pointer_type>(ptr.get());
    }
  };

  template <typename T, typename U>
  struct IsSmartPtr<CheckedPtr<T, U>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = false;
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits, RefDerefTraits> CreateCheckedPtr(T &ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits, RefDerefTraits> AdoptCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits, RefDerefTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = CheckedRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline CheckedPtr<T, PtrTraits, RefDerefTraits> ShareCheckedPtr(T *ptr) noexcept
  {
    return CheckedPtr<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
  }
}
