#pragma once

#include <array>

namespace krys
{
  template <typename T, size_t size>
  using Array = std::array<T, size>;
}