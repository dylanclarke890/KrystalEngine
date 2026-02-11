#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
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
  struct CheckedPolicy
  {
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->AddRefChecked();
      }
      return ptr;
    }

    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ref) noexcept
    {
      ref.AddRefChecked();
      return ref;
    }

    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      if (ptr) KRYS_LIKELY
      {
        ptr->SubRefChecked();
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
  class IntrusivePtr
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");

    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename, typename, IsNullable>
    friend class IntrusivePtr;

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
    KRYS_NODISCARD static constexpr IntrusivePtr NoRef(RawPtr<T> ptr) noexcept
    {
      if constexpr (!nullable)
      {
        assert(ptr);
      }

      return IntrusivePtr(ptr);
    }

    KRYS_NODISCARD static constexpr IntrusivePtr NoRef(T &ref) noexcept
    {
      return IntrusivePtr(&ref);
    }

    KRYS_NODISCARD static constexpr IntrusivePtr WithRef(RawPtr<T> ptr) noexcept
    {
      if constexpr (!nullable)
      {
        assert(ptr);
      }

      return IntrusivePtr(RefPolicy::AddRef(ptr));
    }

    KRYS_NODISCARD static constexpr IntrusivePtr WithRef(T &ref) noexcept
    {
      return IntrusivePtr(&RefPolicy::AddRef(ref));
    }

    KRYS_ALWAYS_INLINE constexpr IntrusivePtr() noexcept
    requires(nullable)
        : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(std::nullptr_t) noexcept
    requires(nullable)
        : _ptr(nullptr)
    {
    }

    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(const IntrusivePtr &o) noexcept
        : _ptr(RefPolicy::AddRef(static_cast<RawPtr<T>>(o.get())))
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(const IntrusivePtr<U, V, X, Y, UNullable> &o) noexcept
        : _ptr(RefPolicy::AddRef(static_cast<RawPtr<T>>(o.get())))
    {
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(IntrusivePtr &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(IntrusivePtr<U, V, X, Y, UNullable> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    KRYS_ALWAYS_INLINE constexpr ~IntrusivePtr() noexcept
    {
      reset();
    }

    constexpr IntrusivePtr &operator=(const IntrusivePtr &o) noexcept
    {
      IntrusivePtr ref = o;
      swap(ref);
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    constexpr IntrusivePtr &operator=(const IntrusivePtr<U, V, X, Y, UNullable> &o) noexcept
    {
      IntrusivePtr ref = o;
      swap(ref);
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    constexpr IntrusivePtr &operator=(IntrusivePtr &&o) noexcept
    {
      IntrusivePtr ref = Krys::Move(o);
      swap(ref);
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    template <typename U, typename V, typename X, typename Y, IsNullable UNullable>
    constexpr IntrusivePtr &operator=(IntrusivePtr<U, V, X, Y, UNullable> &&o) noexcept
    {
      IntrusivePtr ref = Krys::Move(o);
      swap(ref);
      if constexpr (UNullable && !nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }

      return *this;
    }

    constexpr IntrusivePtr &operator=(std::nullptr_t) noexcept
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

    KRYS_ALWAYS_INLINE constexpr void swap(IntrusivePtr &o) noexcept
    {
      PtrTraits::swap(_ptr, o._ptr);
    }

    friend constexpr void swap(IntrusivePtr &a, IntrusivePtr &b) noexcept
    {
      a.swap(b);
    }

  private:
    KRYS_ALWAYS_INLINE explicit constexpr IntrusivePtr(RawPtr<T> ptr) noexcept : _ptr(ptr)
    {
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable>
  constexpr inline bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &a,
                                   std::nullptr_t) noexcept
  {
    return a.get() == nullptr;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable,
            typename U>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &a,
                                   RawPtr<U> b) noexcept
  {
    return a.get() == b;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable,
            typename U, typename UPtrTraits, typename URefPolicy, typename UAccessPolicy,
            IsNullable UNullable>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool
    operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &a,
               const IntrusivePtr<U, UPtrTraits, URefPolicy, UAccessPolicy, UNullable> &b) noexcept
  {
    return a.get() == b.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, typename AccessPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline bool
    Is(const IntrusivePtr<T, PtrTraits, RefPolicy, AccessPolicy, Nullable> &source) noexcept
  {
    return Is<typename T::element_type>(source.get());
  }
}
