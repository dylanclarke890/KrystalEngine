#pragma once

#include <span>

namespace Krys
{
  template <typename T, size_t Extent = std::dynamic_extent>
  using Span = std::span<T, Extent>;
}
