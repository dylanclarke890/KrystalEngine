#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

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

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static RawPtr<T> ValidateGetAccess(RawPtr<T> ptr) noexcept
    {
      return ptr;
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD constexpr static bool IsValid(RawPtr<T> ptr) noexcept
    {
      return ptr != nullptr;
    }
  };

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using Ref = IntrusivePtr<T, PtrTraits, StrongRefPolicy<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using RefPtr = IntrusivePtr<T, PtrTraits, StrongRefPolicy<T>, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits>
    CreateRef(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return Ref<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits>
    CreateRefPtr(Args &&...args) noexcept(NoThrowConstructible<T, Args...>)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return RefPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits> AdoptRef(T &ptr) noexcept
  {
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
    return Ref<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits> ShareRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits>::WithRef(ptr);
  }
}