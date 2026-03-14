#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include <iterator>
#include <ranges>
#include <utility>

namespace Krys::Ranges
{
  using ::std::ranges::subrange;
  using ::std::ranges::subrange_kind;

  template <typename TRange>
  using subrange_for_t = subrange<::std::ranges::iterator_t<TRange>, range_sentinel_t<TRange>>;

  template <typename TRange>
  using csubrange_for_t = subrange<range_const_iterator_t<TRange>, range_const_sentinel_t<TRange>>;

  /// @brief Decomposes a range into its two iterators and returns it as a subrange.
  template <typename TRange>
  constexpr subrange_for_t<TRange>
    CreateSubrange(TRange &&range) noexcept(NoThrowConstructible<TRange, subrange_for_t<TRange>>)
  {
    return subrange_for_t<TRange>(::std::ranges::begin(range), ::std::ranges::end(range));
  }

  /// @brief Takes two iterators and returns them as a subrange.
  template <typename TIterator, typename TSentinel>
  constexpr subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>
    CreateSubrange(TIterator &&it, TSentinel &&sen) noexcept(
      NoThrowConstructible<subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>, TIterator,
                           TSentinel>)
  {
    return subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>(::std::forward<TIterator>(it),
                                                                          ::std::forward<TSentinel>(sen));
  }
}
