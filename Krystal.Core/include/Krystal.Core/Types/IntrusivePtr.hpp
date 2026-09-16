#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/StronglyTypedValue.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include "Krystal.Core/Utils/ForbidHeapAllocation.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <xutility>

namespace krys
{
  /// @brief A wrapper around a pointer to an object that manages reference counting intrusively.
  /// @typeparam T The type of the object being pointed to. Must not be a pointer type.
  /// @typeparam PtrTraits The pointer traits to use for this intrusive pointer. This defines how the pointer
  /// is stored and accessed.
  /// @typeparam RefPolicy The reference counting policy to use for this intrusive pointer. This defines how
  /// reference counting is performed on the object being pointed to.
  /// @typeparam Nullable Whether this intrusive pointer can be null or not. If false, reference-like
  /// semantics are used (asserts for null ptrs are enabled), otherwise pointer-like semantics are used.
  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  class KRYS_TRIVIAL_ABI IntrusivePtr
  {
    KRYS_FORBID_HEAP_ALLOCATION_ALLOWING_PLACEMENT_NEW;

    template <typename, typename, typename, IsNullable>
    friend class IntrusivePtr;

  public:
    using element_type = T;
    using pointer_traits = PtrTraits;
    using ref_policy = RefPolicy;
    constexpr static bool nullable = Nullable.Value;
    using pointer = typename pointer_traits::storage_type;

  private:
    pointer _ptr;

  public:
    KRYS_NODISCARD constexpr static IntrusivePtr NoRef(T *ptr) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

      if constexpr (!nullable)
      {
        krys_assert(ptr != nullptr);
      }

      return IntrusivePtr(ptr);
    }

    KRYS_NODISCARD constexpr static IntrusivePtr NoRef(T &ref) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

      return IntrusivePtr(&ref);
    }

    KRYS_NODISCARD constexpr static IntrusivePtr WithRef(T *ptr) noexcept
    {
      static_assert(!IsPointer<T>, "T must not be a pointer type.");

      if constexpr (!nullable)
      {
        krys_assert(ptr != nullptr);
      }

      return IntrusivePtr(RefPolicy::AddRef(ptr));
    }

    KRYS_NODISCARD constexpr static IntrusivePtr WithRef(T &ref) noexcept
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
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      }
    }

    template <typename X, typename Y, typename Z, IsNullable ONullable>
    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(const IntrusivePtr<X, Y, Z, ONullable> &o) noexcept
        : _ptr(RefPolicy::AddRef(static_cast<T *>(o.get())))
    {
      if constexpr (!nullable)
      {
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      }
    }

    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(IntrusivePtr &&o) noexcept : _ptr(o.release())
    {
      if constexpr (!nullable)
      {
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      }
    }

    template <typename X, typename Y, typename Z, IsNullable ONullable>
    KRYS_ALWAYS_INLINE constexpr IntrusivePtr(IntrusivePtr<X, Y, Z, ONullable> &&o) noexcept
        : _ptr(static_cast<T *>(o.release()))
    {
      if constexpr (!nullable)
      {
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
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
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      }

      return *this;
    }

    template <typename X, typename Y, typename Z, IsNullable ONullable>
    constexpr IntrusivePtr &operator=(const IntrusivePtr<X, Y, Z, ONullable> &o) noexcept
    {
      IntrusivePtr ref = o;
      swap(ref);

      if constexpr (!nullable)
      {
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      }

      return *this;
    }

    constexpr IntrusivePtr &operator=(IntrusivePtr &&o) noexcept
    {
      IntrusivePtr ref = krys::move(o);
      swap(ref);

      if constexpr (!nullable)
      {
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      }

      return *this;
    }

    template <typename X, typename Y, typename Z, IsNullable ONullable>
    constexpr IntrusivePtr &operator=(IntrusivePtr<X, Y, Z, ONullable> &&o) noexcept
    {
      IntrusivePtr ref = krys::move(o);
      swap(ref);

      if constexpr (!nullable)
      {
        krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
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
      krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      return *PtrTraits::unwrap(_ptr);
    }

    KRYS_ALWAYS_INLINE constexpr T *operator->() const noexcept KRYS_LIFETIME_BOUND
    {
      krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      return PtrTraits::unwrap(_ptr);
    }

    // This X type is required to avoid intellisense losing it's fucking mind if the type is incomplete. It
    // can never be anything other than T. Either way it compiles as expected but we need the indirection to
    // avoid intellisense errors.
    template <typename X = T, typename TMember>
    constexpr TMember &operator->*(TMember X::*memptr) const noexcept
    {
#ifndef __INTELLISENSE__
      static_assert(SameType<T, X>, "T must be the same as X");
      krys_assert(PtrTraits::unwrap(_ptr) != nullptr);
      return PtrTraits::unwrap(_ptr)->*memptr;
#endif
    }

    KRYS_NODISCARD constexpr T *get() const noexcept KRYS_LIFETIME_BOUND
    {
      return RefPolicy::ValidateGetAccess(PtrTraits::unwrap(_ptr));
    }

    KRYS_NODISCARD constexpr T *release() noexcept
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
    KRYS_ALWAYS_INLINE explicit constexpr IntrusivePtr(T *ptr) noexcept : _ptr(ptr)
    {
    }
  };

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            std::nullptr_t) noexcept
  {
    return lhs.get() == nullptr;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator==(std::nullptr_t,
                            const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return nullptr == rhs.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs, U *rhs) noexcept
  {
    return lhs.get() == rhs;
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable ONullable>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator==(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            const IntrusivePtr<U, UPtrTraits, URefPolicy, ONullable> &rhs) noexcept
  {
    return lhs.get() == rhs.get();
  }
  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator!=(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            std::nullptr_t) noexcept
  {
    return !(lhs == nullptr);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool operator!=(std::nullptr_t,
                            const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return !(nullptr == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator!=(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs, U *rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U,
            typename UPtrTraits, typename URefPolicy, IsNullable ONullable>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator!=(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &lhs,
                            const IntrusivePtr<U, UPtrTraits, URefPolicy, ONullable> &rhs) noexcept
  {
    return !(lhs == rhs);
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator==(T *lhs, const IntrusivePtr<U, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return lhs == rhs.get();
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable, typename U>
  requires(ConvertibleTo<U *, T *>)
  constexpr bool operator!=(T *lhs, const IntrusivePtr<U, PtrTraits, RefPolicy, Nullable> &rhs) noexcept
  {
    return lhs != rhs.get();
  }

  template <typename TExpected, typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr bool Is(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    return Is<TExpected>(source.get());
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Upcast(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanUpcast<T, U>, "Invalid upcast");

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;

    return instrusive_ptr::WithRef(static_cast<T *>(source.get()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Upcast(IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanUpcast<T, U>, "Invalid upcast");

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;

    return instrusive_ptr::NoRef(static_cast<T *>(source.release()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Downcast(const IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanDowncast<T, U>, "Invalid downcast");
    krys_assert(Is<T>(source));

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;
    return instrusive_ptr::WithRef(static_cast<T *>(source.get()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>
    Downcast(IntrusivePtr<T, PtrTraits, RefPolicy, Nullable> &&source) noexcept
  {
    static_assert(!UnnecessaryTypeCast<T, U>, "Unnecessary cast to same type");
    static_assert(CanDowncast<T, U>, "Invalid downcast");
    krys_assert(Is<T>(source));

    using instrusive_ptr = IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, Nullable>;
    return instrusive_ptr::NoRef(static_cast<T *>(source.release()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, IsNullable(true)>
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

    return instrusive_ptr::WithRef(static_cast<T *>(source.get()));
  }

  template <typename T, typename U, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  KRYS_NODISCARD constexpr IntrusivePtr<match_constness_t<T, U>, PtrTraits, RefPolicy, IsNullable(true)>
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

    return instrusive_ptr::NoRef(static_cast<T *>(source.release()));
  }

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  struct DefaultHash<IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>>
      : public PtrHash<IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>>
  {
  };

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool IsSmartPtr<IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>> = true;

  template <typename T, typename PtrTraits, typename RefPolicy, IsNullable Nullable>
  constexpr bool IsNullableSmartPtr<IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>> =
    IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>::nullable;
}

namespace std
{
  template <typename T, typename PtrTraits, typename RefPolicy, krys::IsNullable Nullable>
  struct hash<krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>>
  {
    using argument_type = krys::IntrusivePtr<T, PtrTraits, RefPolicy, Nullable>;
    using result_type = size_t;
    constexpr result_type operator()(const argument_type &ptr) const noexcept
    {
      return krys::Hash::Combine(ptr.get());
    }
  };
}