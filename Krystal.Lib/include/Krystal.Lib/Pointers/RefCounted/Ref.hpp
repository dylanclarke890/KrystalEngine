#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/ForbidHeapAllocation.hpp"
#include "Krystal.Lib/Pointers/GetPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RawPtrTraits.hpp"
#include "Krystal.Lib/Pointers/RefCounted/IntrusiveHandle.hpp"
#include <cassert>

namespace Krys
{
  template <typename T, typename PtrTraits = RawPtrTraits<T>>
  using Ref = IntrusiveHandle<T, PtrTraits, StrongRefPolicy<T>, NoCheckAccess<T>, IsNullable(false)>;

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
