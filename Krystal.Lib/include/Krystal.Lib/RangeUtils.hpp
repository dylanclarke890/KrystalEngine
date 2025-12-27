#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <type_traits>

namespace Krys
{
  struct RangeUtils
  {
    STATIC_CLASS(RangeUtils)

    template <typename T>
    static void Init(T *first, size_t count)
    {
      if constexpr (std::is_trivial_v<T>)
      {
        memset(first, 0, count * sizeof(T));
      }
      else
      {
        for (size_t i = 0; i < count; i++)
          new (first + i) T();
      }
    }

    template <typename T>
    static void Destroy(T *first, size_t count)
    {
      if constexpr (std::is_trivially_destructible_v<T>)
      {
        return;
      }
      else
      {
        for (size_t i = 0; i < count; i++)
          first[i].~T();
      }
    }

    template <typename T>
    static void Copy(const T *src, T *dest, size_t count)
    {
      if constexpr (std::is_trivially_copyable_v<T>)
      {
        memcpy(dest, src, count * sizeof(T));
      }
      else
      {
        for (size_t i = 0; i < count; i++)
          new (dest + i) T(src[i]);
      }
    }

    template <typename T>
    static void Move(T *src, T *dest, size_t count)
    {
      if constexpr (std::is_trivially_copyable_v<T>)
      {
        memcpy(dest, src, count * sizeof(T));
      }
      else
      {
        for (size_t i = 0; i < count; i++)
          new (dest + i) T(std::move(src[i]));
      }
    }

    template <typename T>
    static int Compare(const T *a, const T *b, size_t count)
    {
      if constexpr (std::is_trivial_v<T>)
      {
        return memcmp(a, b, count * sizeof(T));
      }
      else
      {
        for (size_t i = 0; i < count; i++)
          if (!(a[i] == b[i]))
            return a[i] < b[i] ? -1 : 1;
        return 0;
      }
    }
  };
}