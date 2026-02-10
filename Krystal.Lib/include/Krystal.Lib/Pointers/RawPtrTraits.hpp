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
    using storage_type = RawPtr<T>;

    template <typename U>
    KRYS_ALWAYS_INLINE KRYS_NODISCARD static RawPtr<T> exchange(storage_type &ptr, U &&newValue) noexcept
    {
      return std::exchange(ptr, newValue);
    }

    KRYS_ALWAYS_INLINE static void swap(storage_type &a, storage_type &b) noexcept
    {
      std::swap(a, b);
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD static RawPtr<T> unwrap(const storage_type &ptr) noexcept
    {
      return ptr;
    }
  };
}
