#pragma once

#include <span>

namespace Krys
{
  template <typename T>
  using Span = std::span<T>;

  template <typename T, size_t Extent>
  using FixedSpan = std::span<T, Extent>;
}
