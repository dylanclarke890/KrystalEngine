#pragma once

#include <ranges>

namespace Krys::Ranges
{
  template <typename TRange>
  using subrange_for_t =
    std::ranges::subrange<std::ranges::iterator_t<TRange>, std::ranges::sentinel_t<TRange>>;

  template <typename TRange>
  using csubrange_for_t =
    std::ranges::subrange<std::ranges::const_iterator_t<TRange>, std::ranges::const_sentinel_t<TRange>>;
}