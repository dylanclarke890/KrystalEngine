#pragma once

#include "Krystal.Lib/Pointers/IntrusivePtr.hpp"
#include "Krystal.Lib/Pointers/RefCounted.hpp"

namespace Krys
{
  template <typename T, typename Traits = RefCountedTraits>
  using RefPtr = IntrusivePtr<T, Traits>;

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