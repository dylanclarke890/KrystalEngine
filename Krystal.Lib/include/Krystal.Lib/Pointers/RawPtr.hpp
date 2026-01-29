#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include <cstdint>
#include <utility>

namespace Krys
{
  template <typename T>
  using RawPtr = T *;

  template <typename T>
  struct RawPtrTraits
  {
    template <typename U>
    using RebindTraits = RawPtrTraits<U>;

    template <typename U>
    KRYS_NODISCARD static RawPtr<T> exchange(RawPtr<T> &ptr, U &&newValue)
    {
      return std::exchange(ptr, newValue);
    }

    static void swap(RawPtr<T> &a, RawPtr<T> &b)
    {
      std::swap(a, b);
    }

    static RawPtr<T> unwrap(const RawPtr<T> &ptr)
    {
      return ptr;
    }

    static RawPtr<T> hashTableDeletedValue()
    {
      return std::bit_cast<RawPtr<T>>(static_cast<uintptr_t>(-1));
    }

    static bool isHashTableDeletedValue(const RawPtr<T> &ptr)
    {
      return ptr == hashTableDeletedValue();
    }
  };

}