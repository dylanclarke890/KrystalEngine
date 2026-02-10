#pragma once

#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/IntrusiveHandle.hpp"
#include <algorithm>
#include <utility>

namespace Krys
{
  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using RefPtr = IntrusiveHandle<T, PtrTraits, StrongRefPolicy<T>, NoCheckAccess<T>, IsNullable(true)>;

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
}