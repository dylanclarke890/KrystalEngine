#pragma once

#include "Krystal.Lib/Pointers/Concepts.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"

namespace Krys
{
  struct StrongRefPolicy
  {
    template <typename T>
    KRYS_ALWAYS_INLINE constexpr static RawPtr<T> AddRef(RawPtr<T> ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr) KRYS_LIKELY
      {
        ptr->AddRef();
      }
      return ptr;
    }

    template <typename T>
    KRYS_ALWAYS_INLINE constexpr static T &AddRef(T &ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      ptr.AddRef();
      return ptr;
    }

    template <typename T>
    KRYS_ALWAYS_INLINE constexpr static void SubRef(RawPtr<T> ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      if (ptr) KRYS_LIKELY
      {
        ptr->SubRef();
      }
    }

    template <typename T>
    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static RawPtr<T> ValidateGetAccess(RawPtr<T> ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      return ptr;
    }

    template <typename T>
    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(RawPtr<T> ptr) noexcept
    {
      static_assert(IsTypeComplete<T>, "T is an incomplete type.");

      return ptr != nullptr;
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using Ref = IntrusivePtr<T, PtrTraits, StrongRefPolicy, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using RefPtr = IntrusivePtr<T, PtrTraits, StrongRefPolicy, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  requires(Constructible<T, Args...>)
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits>
    CreateRef(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return Ref<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  requires(Constructible<T, Args...>)
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits>
    CreateRefPtr(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return RefPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits> AdoptRef(T &ptr) noexcept
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");
    return Ref<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits> AdoptRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits> ShareRef(T &ptr) noexcept
  {
    static_assert(!IsPointer<T>, "T must not be a pointer type.");
    return Ref<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits> ShareRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits>::WithRef(ptr);
  }

  /// @brief Similar to 'ShareRef' but is to be used for indicating that the underlying object needs to
  /// persist for the duration of it's current scope. This is for cases where operations on the object may
  /// decrement it's last reference count, but the caller needs to ensure that the object isn't destroyed
  /// until the end of the current scope.
  template <typename T>
  KRYS_NODISCARD constexpr inline Ref<T> Protect(T &object) noexcept
  {
    return ShareRef(object);
  }
}