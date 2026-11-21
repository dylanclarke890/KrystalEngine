#pragma once

#include <span>

namespace krys
{
  template <typename T, size_t Extent = std::dynamic_extent>
  using Span = std::span<T, Extent>;

  template <typename T, size_t Extent>
  using FixedSpan = std::span<T, Extent>;
}
