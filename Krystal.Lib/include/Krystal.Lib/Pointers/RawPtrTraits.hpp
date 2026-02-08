#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
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

    using StorageType = RawPtr<T>;

    template <typename U>
    KRYS_ALWAYS_INLINE KRYS_NODISCARD static RawPtr<T> exchange(StorageType &ptr, U &&newValue) noexcept
    {
      return std::exchange(ptr, newValue);
    }

    KRYS_ALWAYS_INLINE static void swap(StorageType &a, StorageType &b) noexcept
    {
      std::swap(a, b);
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD static RawPtr<T> unwrap(const StorageType &ptr) noexcept
    {
      return ptr;
    }

    KRYS_NODISCARD KRYS_NODISCARD static StorageType HashTableDeletedValue() noexcept
    {
      return std::bit_cast<StorageType>(static_cast<uintptr_t>(-1));
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD static bool IsHashTableDeletedValue(const StorageType &ptr) noexcept
    {
      return ptr == HashTableDeletedValue();
    }
  };
}
