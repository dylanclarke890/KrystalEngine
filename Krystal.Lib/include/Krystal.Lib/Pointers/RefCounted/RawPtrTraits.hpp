#pragma once

#include <bit>
#include <cstdint>
#include <utility>

namespace Krys
{
  template <typename T>
  struct RawPtrTraits
  {
    template <typename U>
    using RebindTraits = RawPtrTraits<U>;

    using StorageType = T *;

    template <typename U>
    static KRYS_ALWAYS_INLINE T *exchange(StorageType &ptr, U &&newValue)
    {
      return std::exchange(ptr, newValue);
    }

    static KRYS_ALWAYS_INLINE void swap(StorageType &a, StorageType &b)
    {
      std::swap(a, b);
    }
    static KRYS_ALWAYS_INLINE T *unwrap(const StorageType &ptr)
    {
      return ptr;
    }

    static StorageType hashTableDeletedValue()
    {
      return std::bit_cast<StorageType>(static_cast<uintptr_t>(-1));
    }

    static KRYS_ALWAYS_INLINE bool isHashTableDeletedValue(const StorageType &ptr)
    {
      return ptr == hashTableDeletedValue();
    }
  };
}
