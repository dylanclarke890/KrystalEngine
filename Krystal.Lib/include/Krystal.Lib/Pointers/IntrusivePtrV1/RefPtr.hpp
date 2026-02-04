#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtrV1/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/IntrusivePtrV1/RefCounted.hpp"

namespace Krys
{
  template <typename T, typename Traits = RefCountedTraits>
  using RefPtr = IntrusivePtr<T, Traits>;

  template <typename T, typename Traits = RefCountedTraits, typename... Args>
  requires(Constructible<T, Args...>)
  KRYS_NODISCARD constexpr inline RefPtr<T, Traits> CreateRefPtr(Args &&...args)
  {
    return RefPtr<T, Traits>::NoRef(new T(std::forward<Args>(args)...));
  }

  template <typename T, typename Traits = RefCountedTraits>
  constexpr RefPtr<T, Traits> RefPtrRetain(T *ptr) noexcept
  {
    return RefPtr<T, Traits>::WithRef(ptr);
  }

  template <typename T, typename Traits = RefCountedTraits>
  constexpr RefPtr<T, Traits> RefPtrAttach(T *ptr) noexcept
  {
    return RefPtr<T, Traits>::NoRef(ptr);
  }
}