#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include <cassert>

namespace Krys
{
  template <typename T>
  struct DefaultRefDerefTraits
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRef();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      ref.AddRef();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRef();
      }
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  class IntrusiveRef
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename X, typename Y, typename Z>
    friend class IntrusiveRef;

  public:
    using pointer = typename PtrTraits::storage_type;
    using element_type = T;
    constexpr static bool nullable = false;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr IntrusiveRef NoRef(T &ref) noexcept
    {
      return IntrusiveRef(ref);
    }

    KRYS_NODISCARD static constexpr IntrusiveRef WithRef(T &ref) noexcept
    {
      return IntrusiveRef(RefDerefTraits::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveRef(const IntrusiveRef &o) noexcept : _ptr(RefDerefTraits::AddRef(o.get()))
    {
      assert(_ptr);
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr IntrusiveRef(const IntrusiveRef<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
      assert(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveRef(IntrusiveRef &&o) noexcept : _ptr(o.release())
    {
      assert(_ptr);
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr IntrusiveRef(IntrusiveRef<X, Y, Z> &&o) noexcept : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      assert(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr ~IntrusiveRef() noexcept
    {
      reset();
    }

    constexpr IntrusiveRef &operator=(const IntrusiveRef &o) noexcept
    {
      IntrusiveRef ref = o;
      swap(ref);
      assert(_ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr IntrusiveRef &operator=(const IntrusiveRef<X, Y, Z> &o) noexcept
    {
      IntrusiveRef ref = o;
      swap(ref);
      assert(_ptr);
      return *this;
    }

    constexpr IntrusiveRef &operator=(IntrusiveRef &&o) noexcept
    {
      IntrusiveRef ref = Krys::Move(o);
      swap(ref);
      assert(_ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr IntrusiveRef &operator=(IntrusiveRef<X, Y, Z> &&o) noexcept
    {
      IntrusiveRef ref = Krys::Move(o);
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

    KRYS_ALWAYS_INLINE constexpr void swap(IntrusiveRef &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(IntrusiveRef &a, IntrusiveRef &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr IntrusiveRef(T &object) noexcept : _ptr(&object)
    {
    }
  };

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  constexpr inline bool operator==(const IntrusiveRef<T, U, V> &a, const IntrusiveRef<X, Y, Z> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const IntrusiveRef<T, U, V> &a, RawPtr<X> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const IntrusiveRef<T, U, V> &a, std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline IntrusiveRef<T, PtrTraits, RefDerefTraits> CreateRef(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return IntrusiveRef<T, PtrTraits, RefDerefTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline IntrusiveRef<T, PtrTraits, RefDerefTraits> AdoptRef(T &ptr) noexcept
  {
    return IntrusiveRef<T, PtrTraits, RefDerefTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline IntrusiveRef<T, PtrTraits, RefDerefTraits> ShareRef(T &ptr) noexcept
  {
    return IntrusiveRef<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct IsSmartPtr<IntrusiveRef<T, PtrTraits, RefDerefTraits>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = false;
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct GetPtrHelper<IntrusiveRef<T, PtrTraits, RefDerefTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    static pointer_type GetPtr(const IntrusiveRef<T, PtrTraits, RefDerefTraits> &ref) noexcept
    {
      return const_cast<pointer_type>(ref.get());
    }
  };
}
