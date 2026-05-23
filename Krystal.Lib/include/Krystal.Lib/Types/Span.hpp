#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include <span>

namespace Krys
{
  template <typename T, size_t Extent = std::dynamic_extent>
  using Span = std::span<T, Extent>;

  template <typename T, size_t Extent>
  using FixedSpan = std::span<T, Extent>;

  /// @brief Determines whether or not a given type is a @c span of some kind.
  template <typename T>
  class is_span : public std::false_type
  {
  };

  template <typename T, decltype(std::dynamic_extent) N>
  class is_span<Span<T, N>> : public std::true_type
  {
  };

  template <typename T>
  concept IsSpan = is_span<T>::value;
}
