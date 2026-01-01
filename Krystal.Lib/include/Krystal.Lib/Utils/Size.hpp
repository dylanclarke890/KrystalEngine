#pragma once

namespace Krys
{
  template <typename T, typename TSentinel>
  constexpr size_t cstringPtrSize(T *ptr, const TSentinel &sentinel) noexcept
  {
    if (ptr == nullptr)
    {
      return 0;
    }
    size_t len = 0;
    while (ptr[len] != sentinel)
    {
      ++len;
    }
    return len;
  }

  template <typename T>
  constexpr size_t cstringPtrSize(T *ptr) noexcept
  {
    constexpr T sentinel {};
    return cstringPtrSize(ptr, sentinel);
  }
}