#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Config.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#if KRYS_CONFIG(STD_LIBRARY_BORROWED_RANGE)
  #include <ranges>
#endif
#include <span>

namespace Krys
{
  template <typename T, size_t Extent = std::dynamic_extent>
  using Span = std::span<T, Extent>;

  template <typename T, size_t Extent>
  using FixedSpan = std::span<T, Extent>;

  /// @brief Determines whether or not a given type is a @c span of some kind.
  template <typename T>
  class is_span : public ::std::false_type
  {
  };

  template <typename T, decltype(std::dynamic_extent) N>
  class is_span<Span<T, N>> : public ::std::true_type
  {
  };

  template <typename T>
  concept IsSpan = is_span<T>::value;

  namespace Ranges
  {
    template <typename T, decltype(std::dynamic_extent) Extent>
    inline constexpr bool EnableView<Krys::Span<T, Extent>> = true;

    template <typename T, decltype(std::dynamic_extent) Extent>
    inline constexpr bool EnableBorrowedRange<Krys::Span<T, Extent>> = true;
  }
}
