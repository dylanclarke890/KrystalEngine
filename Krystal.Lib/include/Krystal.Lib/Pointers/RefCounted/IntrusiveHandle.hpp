#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include <cassert>

namespace Krys
{
  template <typename T>
  struct StrongRefPolicy
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRef();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ptr) noexcept
    {
      ptr.AddRef();
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRef();
      }
    }
  };

  template <typename T>
  struct NoCheckAccess
  {
    KRYS_ALWAYS_INLINE constexpr static void Validate(RawPtr<T>) noexcept
    {
    }
  };

  template <typename T>
  struct CheckedAccess
  {
    KRYS_ALWAYS_INLINE constexpr static void Validate(RawPtr<T> ptr) noexcept
    {
      assert(!ptr || ptr->CheckedPtrCount());
    }
  };

  template <typename T>
  struct WeakAccess
  {
    KRYS_ALWAYS_INLINE constexpr static void Validate(RawPtr<T> ptr) noexcept
    {
      assert(!ptr || ptr->GetRefCount());
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable>
  class IntrusiveHandle
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename, typename, IsNullable>
    friend class IntrusiveHandle;

  public:
    using element_type = T;
    using pointer_traits = PtrTraits;
    using pointer = typename pointer_traits::storage_type;
    using ref_policy = RefPolicy;
    using access_policy = AccessPolicy;
    constexpr static bool nullable = Nullable.Value;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr IntrusiveHandle NoRef(RawPtr<T> ptr) noexcept
    {
      if constexpr (!nullable)
      {
        assert(ptr);
      }

      return IntrusiveHandle(ptr);
    }

    KRYS_NODISCARD static constexpr IntrusiveHandle NoRef(T &ref) noexcept
    {
      return IntrusiveHandle(&ref);
    }

    KRYS_NODISCARD static constexpr IntrusiveHandle WithRef(RawPtr<T> ptr) noexcept
    {
      if constexpr (!nullable)
      {
        assert(ptr);
      }

      return IntrusiveHandle(RefPolicy::AddRef(ptr));
    }

    KRYS_NODISCARD static constexpr IntrusiveHandle WithRef(T &ref) noexcept
    {
      return IntrusiveHandle(&RefPolicy::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveHandle() noexcept
    requires(nullable)
        : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveHandle(std::nullptr_t) noexcept
    requires(nullable)
        : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveHandle(const IntrusiveHandle &o) noexcept
        : _ptr(RefPolicy::AddRef(static_cast<RawPtr<T>>(o.get())))
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    KRYS_ALWAYS_INLINE constexpr IntrusiveHandle(const IntrusiveHandle<U, V, X, Y, UNullable> &o) noexcept
        : _ptr(RefPolicy::AddRef(static_cast<RawPtr<T>>(o.get())))
    {
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusiveHandle(IntrusiveHandle &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    KRYS_ALWAYS_INLINE constexpr IntrusiveHandle(IntrusiveHandle<U, V, X, Y, UNullable> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    KRYS_ALWAYS_INLINE constexpr ~IntrusiveHandle() noexcept
    {
      reset();
    }

    constexpr IntrusiveHandle &operator=(const IntrusiveHandle &o) noexcept
    {
      IntrusiveHandle ref = o;
      swap(ref);
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    constexpr IntrusiveHandle &operator=(const IntrusiveHandle<U, V, X, Y, UNullable> &o) noexcept
    {
      IntrusiveHandle ref = o;
      swap(ref);
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    constexpr IntrusiveHandle &operator=(IntrusiveHandle &&o) noexcept
    {
      IntrusiveHandle ref = Krys::Move(o);
      swap(ref);
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    constexpr IntrusiveHandle &operator=(IntrusiveHandle<U, V, X, Y, UNullable> &&o) noexcept
    {
      IntrusiveHandle ref = Krys::Move(o);
      swap(ref);
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    constexpr IntrusiveHandle &operator=(std::nullptr_t) noexcept
    requires(nullable)
    {
      reset();
      return *this;
    }

    constexpr bool operator!() const noexcept
    {
      return !PtrTraits::unwrap(_ptr);
    }

    explicit constexpr operator bool() const noexcept
    {
      return !!PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr T &operator*() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(PtrTraits::unwrap(_ptr));
      return *PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr RawPtr<T> operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      assert(PtrTraits::unwrap(_ptr));
      return PtrTraits::unwrap(_ptr);
    }

    template <typename TMember>
    constexpr TMember &operator->*(TMember T::*memptr) const noexcept
    {
      assert(PtrTraits::unwrap(_ptr));
      return PtrTraits::unwrap(_ptr)->*memptr;
    }

    KRYS_NODISCARD constexpr RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
    {
      AccessPolicy::Validate(PtrTraits::unwrap(_ptr));
      return PtrTraits::unwrap(_ptr);
    }

    KRYS_NODISCARD constexpr RawPtr<T> release() noexcept
    {
      return PtrTraits::exchange(_ptr, nullptr);
    }

    KRYS_ALWAYS_INLINE constexpr void reset() noexcept
    {
      RefPolicy::SubRef(PtrTraits::exchange(_ptr, nullptr));
    }

    KRYS_ALWAYS_INLINE constexpr void swap(IntrusiveHandle &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(IntrusiveHandle &a, IntrusiveHandle &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr IntrusiveHandle(RawPtr<T> ptr) noexcept : _ptr(ptr)
    {
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable>
  constexpr inline bool operator==(const IntrusiveHandle<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &a,
                                   std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable,
            typename U>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator==(const IntrusiveHandle<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &a,
                                   RawPtr<U> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable,
            typename U, typename UPtrTraits, typename URefPolicy, typename UAccessPolicy,
            IsNullable UNullable>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool
    operator==(const IntrusiveHandle<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &a,
               const IntrusiveHandle<U, UPtrTraits, URefPolicy, UAccessPolicy, UNullable> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline bool
    Is(const IntrusiveHandle<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &source) noexcept
  {
    return Is<typename T::element_type>(source.get());
  }
}
