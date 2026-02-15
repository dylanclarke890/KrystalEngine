#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"
#include <cassert>
#include <format>
#include <utility>

// TODO(FIX): review how non nullable and nullable intrusive ptrs interact with each other.
namespace Krys
{
  /// @brief A wrapper around a pointer to an object that manages reference counting intrusively.
  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  class KRYS_TRIVIAL_ABI IntrusivePtr
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename, IsNullable>
    friend class IntrusivePtr;

  public:
    using element_type = T;
    using pointer_traits = PtrTraits;
    using pointer = typename pointer_traits::storage_type;
    using ref_policy = RefPolicy;
    constexpr static bool nullable = Nullable.Value;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD static constexpr IntrusivePtr NoRef(RawPtr<T> ptr) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

      if constexpr (!nullable)
      {
        assert(ptr);
      }

      return IntrusivePtr(ptr);
    }

    KRYS_NODISCARD static constexpr IntrusivePtr NoRef(T &ref) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

      return IntrusivePtr(&ref);
    }

    KRYS_NODISCARD static constexpr IntrusivePtr WithRef(RawPtr<T> ptr) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

      if constexpr (!nullable)
      {
        assert(ptr);
      }

      return IntrusivePtr(RefPolicy::AddRef(ptr));
    }

    KRYS_NODISCARD static constexpr IntrusivePtr WithRef(T &ref) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

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
        : _ptr(RefPolicy::AddRef(o.get()))
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    template <typename X, typename Y, typename Z, IsNullable UNullable>
    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(const IntrusivePtr<X, Y, Z, UNullable> &o) noexcept
        : _ptr(RefPolicy::AddRef(static_cast<RawPtr<T>>(o.get())))
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(IntrusivePtr &&o) noexcept : _ptr(o.release())
    {
      if constexpr (!nullable)
      {
        assert(PtrTraits::unwrap(_ptr));
      }
    }

    template <typename X, typename Y, typename Z, IsNullable UNullable>
    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(IntrusivePtr<X, Y, Z, UNullable> &&o) noexcept
        : _ptr(static_cast<RawPtr<T>>(o.release()))
    {
      if constexpr (!nullable)
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

    template <typename X, typename Y, typename Z, IsNullable UNullable>
    constexpr IntrusivePtr &operator=(const IntrusivePtr<X, Y, Z, UNullable> &o) noexcept
    {
      IntrusivePtr ref = o;
      swap(ref);
      if constexpr (!nullable)
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

    template <typename X, typename Y, typename Z, IsNullable UNullable>
    constexpr IntrusivePtr &operator=(IntrusivePtr<X, Y, Z, UNullable> &&o) noexcept
    {
      IntrusivePtr ref = Krys::Move(o);
      swap(ref);
      if constexpr (!nullable)
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
      return !RefPolicy::IsValid(PtrTraits::unwrap(_ptr));
    }

    explicit constexpr operator bool() const noexcept
    {
      return RefPolicy::IsValid(PtrTraits::unwrap(_ptr));
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

    // This X type is required to avoid intellisense losing it's fucking mind if the type is incomplete. It
    // can never be anything other than T. Either way it compiles as expected but we need the indirection to
    // avoid intellisense errors.
    template <typename X = T, typename TMember>
    constexpr TMember &operator->*(TMember X::*memptr) const noexcept
    {
      static_assert(SameType<T, X>, "T must be the same as X");
      assert(PtrTraits::unwrap(_ptr));
      return PtrTraits::unwrap(_ptr)->*memptr;
    }

    KRYS_NODISCARD constexpr RawPtr<T> get() const noexcept KRYS_LIFETIME_BOUND
    {
      return RefPolicy::ValidateGetAccess(PtrTraits::unwrap(_ptr));
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

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   std::nullptr_t) noexcept
  {
    return lhs.get() == nullptr;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator==(std::nullptr_t,
                                   const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return nullptr == rhs.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   RawPtr<U> rhs) noexcept
  {
    return lhs.get() == rhs;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable UNullable>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   const IntrusivePtr<U, UPtrTraits, URefPolicy, UNullable> &rhs) noexcept
  {
    return lhs.get() == rhs.get();
  }
  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator!=(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   std::nullptr_t) noexcept
  {
    return !(lhs == nullptr);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr inline bool operator!=(std::nullptr_t,
                                   const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return !(nullptr == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator!=(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   RawPtr<U> rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable UNullable>
  requires(ConvertibleTo<RawPtr<U>, RawPtr<T>>)
  constexpr inline bool operator!=(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                                   const IntrusivePtr<U, UPtrTraits, URefPolicy, UNullable> &rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename TExpected, typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline bool
    Is(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Upcast(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanUpcast<T, U>, "Invalid upcast");

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;

    return instrusive_ptr::WithRef(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Upcast(IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanUpcast<T, U>, "Invalid upcast");

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;

    return instrusive_ptr::NoRef(static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Downcast(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanDowncast<T, U>, "Invalid downcast");

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;
    assert(Is<T>(source));
    return instrusive_ptr::WithRef(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Downcast(IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanDowncast<T, U>, "Invalid downcast");

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;
    assert(Is<T>(source));

    return instrusive_ptr::NoRef(static_cast<RawPtr<T>>(source.release()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy,
                                               IsNullable(true)>
    DynamicDowncast(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanDowncast<T, U>, "Invalid downcast");

    // regardless of what was passed in, we have to return a nullable intrusive ptr here because the dynamic
    // cast might fail.
    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, IsNullable(true)>;
    if (!Is<T>(source))
    {
      return instrusive_ptr(nullptr);
    }

    return instrusive_ptr::WithRef(static_cast<RawPtr<T>>(source.get()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr inline IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy,
                                               IsNullable(true)>
    DynamicDowncast(IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanDowncast<T, U>, "Invalid downcast");

    // regardless of what was passed in, we have to return a nullable intrusive ptr here because the dynamic
    // cast might fail.
    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, IsNullable(true)>;
    if (!Is<T>(source))
    {
      return instrusive_ptr(nullptr);
    }

    return instrusive_ptr::NoRef(static_cast<RawPtr<T>>(source.release()));
  }
}

namespace std
{
  template <typename T, typename PtrTraits, typename RefPolicy, ::Krys::IsNullable Nullable, typename TChar>
  struct formatter<::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>, TChar>
      : public formatter<void *, TChar>
  {
    template <typename FormatContext>
    auto format(const ::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &ptr, FormatContext &ctx) const
      -> decltype(ctx.out())
    {
      return formatter<void *, TChar>::format(ptr.get(), ctx);
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, ::Krys::IsNullable Nullable>
  struct hash<::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>>
  {
    constexpr size_t
      operator()(const ::Krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &ptr) const noexcept
    {
      return std::hash<::Krys::RawPtr<T>>()(ptr.get());
    }
  };
}
