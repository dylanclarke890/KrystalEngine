#pragma once

#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/Ref.hpp"
#include <algorithm>
#include <utility>

namespace Krys
{
  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  class RefPtr
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename>
    friend class RefPtr;

  public:
    using pointer = typename PtrTraits::storage_type;
    using element_type = T;
    constexpr static bool nullable = true;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr RefPtr NoRef(RawPtr<T> ptr) noexcept
    {
      return RefPtr(ptr);
    }

    KRYS_NODISCARD static constexpr RefPtr WithRef(RawPtr<T> ptr) noexcept
    {
      return RefPtr(RefDerefTraits::AddRef(ptr));
    }

    KRYS_NODISCARD static constexpr RefPtr NoRef(T &ref) noexcept
    {
      return RefPtr(ref);
    }

    KRYS_NODISCARD static constexpr RefPtr WithRef(T &ref) noexcept
    {
      return RefPtr(RefDerefTraits::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr() noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(std::nullptr_t) noexcept : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(const RefPtr &o) noexcept : _ptr(RefDerefTraits::AddRef(o.get()))
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr RefPtr(const RefPtr<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr RefPtr(Ref<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
    }

    KRYS_ALWAYS_INLINE constexpr RefPtr(RefPtr &&o) noexcept : _ptr(o.release())
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr RefPtr(RefPtr<X, Y, Z> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr RefPtr(Ref<X, Y, Z> &&o) noexcept : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
    }

    KRYS_ALWAYS_INLINE constexpr ~RefPtr() noexcept
    {
      reset();
    }

    constexpr RefPtr &operator=(const RefPtr &o) noexcept
    {
      RefPtr ptr = o;
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr RefPtr &operator=(const RefPtr<X, Y, Z> &o) noexcept
    {
      RefPtr ptr = o;
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr RefPtr &operator=(Ref<X, Y, Z> &o) noexcept
    {
      RefPtr ptr = o;
      swap(ptr);
      return *this;
    }

    constexpr RefPtr &operator=(RefPtr &&o) noexcept
    {
      RefPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr RefPtr &operator=(RefPtr<X, Y, Z> &&o) noexcept
    {
      RefPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr RefPtr &operator=(Ref<X, Y, Z> &&o) noexcept
    {
      RefPtr ptr = Krys::Move(o);
      swap(ptr);
      return *this;
    }

    constexpr RefPtr &operator=(std::nullptr_t) noexcept
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

    KRYS_NODISCARD constexpr RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
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

    KRYS_ALWAYS_INLINE constexpr void swap(RefPtr &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(RefPtr &a, RefPtr &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr RefPtr(RawPtr<T> ptr) noexcept : _ptr(ptr)
    {
    }
  };

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  constexpr inline bool operator==(const RefPtr<T, U, V> &a, const RefPtr<X, Y, Z> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const RefPtr<T, U, V> &a, RawPtr<X> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename U, typename V>
  constexpr inline bool operator==(const RefPtr<T, U, V> &a, std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits, RefDerefTraits> CreateRefPtr(Args &&...args) noexcept
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return RefPtr<T, PtrTraits, RefDerefTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits, RefDerefTraits> AdoptRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits, RefDerefTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits, RefDerefTraits> ShareRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
  }

  template <typename T, typename U, typename V>
  struct IsSmartPtr<RefPtr<T, U, V>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = true;
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct GetPtrHelper<RefPtr<T, PtrTraits, RefDerefTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    static pointer_type GetPtr(const RefPtr<T, PtrTraits, RefDerefTraits> &ptr) noexcept
    {
      return const_cast<pointer_type>(ptr.get());
    }
  };
}