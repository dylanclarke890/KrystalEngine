#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <utility>

namespace krys
{
  template <class F, class T, size_t... I>
  constexpr decltype(auto) apply_impl(F &&functor, T &&tupleLike, std::index_sequence<I...>) noexcept
  {
    using std::get;
    return std::invoke(std::forward<F>(functor), get<I>(std::forward<T>(tupleLike))...);
  }

  template <class F, class T>
  constexpr decltype(auto) apply(F &&functor, T &&tupleLike) noexcept
  {
    return apply_impl(std::forward<F>(functor), std::forward<T>(tupleLike),
                      std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>> {});
  }
}