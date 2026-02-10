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

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  class RefPtr;

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  class Ref
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename X, typename Y, typename Z>
    friend class Ref;

  public:
    using pointer = typename PtrTraits::storage_type;
    using element_type = T;
    constexpr static bool nullable = false;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr Ref NoRef(T &ref) noexcept
    {
      return Ref(ref);
    }

    KRYS_NODISCARD static constexpr Ref WithRef(T &ref) noexcept
    {
      return Ref(RefDerefTraits::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE constexpr Ref(const Ref &o) noexcept : _ptr(RefDerefTraits::AddRef(o.get()))
    {
      assert(_ptr);
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr Ref(const Ref<X, Y, Z> &o) noexcept
        : _ptr(static_cast<RawPtr<T>>(RefDerefTraits::AddRef(o.get())))
    {
      assert(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr Ref(Ref &&o) noexcept : _ptr(o.release())
    {
      assert(_ptr);
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    KRYS_ALWAYS_INLINE constexpr Ref(Ref<X, Y, Z> &&o) noexcept : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      assert(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr ~Ref() noexcept
    {
      reset();
    }

    constexpr Ref &operator=(const Ref &o) noexcept
    {
      Ref ref = o;
      swap(ref);
      assert(_ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr Ref &operator=(const Ref<X, Y, Z> &o) noexcept
    {
      Ref ref = o;
      swap(ref);
      assert(_ptr);
      return *this;
    }

    constexpr Ref &operator=(Ref &&o) noexcept
    {
      Ref ref = Krys::Move(o);
      swap(ref);
      assert(_ptr);
      return *this;
    }

    template <typename X, typename Y, typename Z>
    requires(ConvertibleTo<RawPtr<X>, RawPtr<T>>)
    constexpr Ref &operator=(Ref<X, Y, Z> &&o) noexcept
    {
      Ref ref = Krys::Move(o);
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

    KRYS_ALWAYS_INLINE constexpr void swap(Ref &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(Ref &a, Ref &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr Ref(T &object) noexcept : _ptr(&object)
    {
    }
  };

  template <typename T, typename U, typename V, typename X, typename Y, typename Z>
  constexpr inline bool operator==(const Ref<T, U, V> &a, const Ref<X, Y, Z> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const Ref<T, U, V> &a, RawPtr<X> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename U, typename V, typename X>
  constexpr inline bool operator==(const Ref<T, U, V> &a, std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits, RefDerefTraits> CreateRef(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return Ref<T, PtrTraits, RefDerefTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits, RefDerefTraits> AdoptRef(T &ptr) noexcept
  {
    return Ref<T, PtrTraits, RefDerefTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>,
            typename RefDerefTraits = DefaultRefDerefTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits, RefDerefTraits> ShareRef(T &ptr) noexcept
  {
    return Ref<T, PtrTraits, RefDerefTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct IsSmartPtr<Ref<T, PtrTraits, RefDerefTraits>>
  {
    static constexpr bool value = true;
    static constexpr bool nullable = false;
  };

  template <typename T, typename PtrTraits, typename RefDerefTraits>
  struct GetPtrHelper<Ref<T, PtrTraits, RefDerefTraits>>
  {
    using pointer_type = RawPtr<T>;
    using underlying_type = T;

    static pointer_type GetPtr(const Ref<T, PtrTraits, RefDerefTraits> &ref) noexcept
    {
      return const_cast<pointer_type>(ref.get());
    }
  };
}
