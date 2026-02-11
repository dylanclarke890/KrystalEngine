#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys
{
  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using RefPtr = IntrusivePtr<T, PtrTraits, StrongRefPolicy<T>, NoCheckAccess<T>, IsNullable(true)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits> CreateRefPtr(Args &&...args) noexcept
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return RefPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits> AdoptRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline RefPtr<T, PtrTraits> ShareRefPtr(RawPtr<T> ptr) noexcept
  {
    return RefPtr<T, PtrTraits>::WithRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using Ref = IntrusivePtr<T, PtrTraits, StrongRefPolicy<T>, NoCheckAccess<T>, IsNullable(false)>;

  template <typename T, typename PtrTraits = RawPtrTraits<T>, typename... Args>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits> CreateRef(Args &&...args)
  {
    RawPtr<T> ptr = new T(std::forward<Args>(args)...);
    return Ref<T, PtrTraits>::NoRef(*ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits> AdoptRef(T &ptr) noexcept
  {
    return Ref<T, PtrTraits>::NoRef(ptr);
  }

  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  KRYS_NODISCARD constexpr inline Ref<T, PtrTraits> ShareRef(T &ptr) noexcept
  {
    return Ref<T, PtrTraits>::WithRef(ptr);
  }
}