#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Ranges/SaveIterator.hpp"

namespace Krys::Ranges
{
  /// @brief Clones a range if it is not aan input or output range.
  /// @param range The range to save.
  /// @returns Either an implementation-defined object that indicates the save could not be done, or a
  /// move/copy of the given range.
  template <typename TRange>
  constexpr auto SaveRange(TRange &&range) noexcept
  {
    if constexpr (SameType<remove_cvref_t<TRange>, Impl::range_iterator_unsaveable_t>)
    {
      return Impl::range_iterator_unsaveable;
    }
    else if constexpr (Krys::Ranges::InputOrOutputRangeExact<remove_cvref_t<TRange>>)
    {
      return Impl::range_iterator_unsaveable;
    }
    else
    {
      return std::forward<TRange>(range);
    }
  }

  /// @brief Clones a range if it is not aan input or output range.
  /// @param range An object returned by a previous call to ztd::ranges::SaveRange.
  /// @param fallbackRange The range to return if the given range is either an input/output range or is
  /// the implementation-defined "unsaveable" object.
  /// @returns Either a clone of the `range` if possible, or if not returns the `fallbackRange`.
  template <typename TRange, typename TFallbackRange>
  constexpr auto RestoreRange(TRange &&range, TFallbackRange &&fallbackRange) noexcept
  {
    if constexpr (SameType<remove_cvref_t<TRange>, Impl::range_iterator_unsaveable_t>)
    {
      return std::forward<TFallbackRange>(fallbackRange);
    }
    else if constexpr (Krys::Ranges::InputOrOutputRangeExact<remove_cvref_t<TRange>>)
    {
      return std::forward<TFallbackRange>(fallbackRange);
    }
    else
    {
      return std::forward<TRange>(range);
    }
  }

  template <typename TIt>
  inline constexpr bool UnsaveableRange = UnsaveableIterator<remove_cvref_t<TIt>>;
}
