#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Ranges/TypeTraits.hpp"
#include <ranges>

namespace Krys::Ranges
{
  template <typename TFirst0, typename TLast0, typename TFirst1, typename TLast1>
  constexpr int LexicographicalCompareThreeWayBasic(TFirst0 first0, TLast0 last0, TFirst1 first1,
                                                    TLast1 last1)
  {
    for (; (first0 != last0) && (first1 != last1); ++first0, (void)++first1)
    {
      if (*first0 < *first1)
      {
        return -1;
      }
      if (*first1 < *first0)
      {
        return 1;
      }
    }

    if (first0 == last0 && first1 == last1)
    {
      return 0;
    }
    else if (first0 == last0)
    {
      return -1;
    }
    else
    {
      return 1;
    }
  }

  template <typename TOutputContainer, typename TInsertion>
  constexpr void ContainerInsertBulk(TOutputContainer &output, TInsertion &&insertion) noexcept
  {
    using iterator = std::ranges::iterator_t<remove_cvref_t<TInsertion>>;

    if constexpr (HasInsertBulk<TOutputContainer, iterator, iterator>)
    {
      output.insert(std::ranges::cend(output), std::ranges::cbegin(insertion), std::ranges::cend(insertion));
    }
    else
    {
      for (auto &&value : insertion)
      {
        if constexpr (HasPushBack<TOutputContainer, std::iter_reference_t<iterator>>)
        {
          output.push_back(std::forward<decltype(value)>(value));
        }
        else
        {
          output.insert(std::ranges::cend(output), std::forward<decltype(value)>(value));
        }
      }
    }
  }

  struct range_iterator_unsaveable_t
  {
  };

  constexpr range_iterator_unsaveable_t range_iterator_unsaveable {};

  template <typename TRange>
  concept InputOrOutputRange =
    std::ranges::input_range<TRange> || std::ranges::output_range<TRange, std::ranges::range_value_t<TRange>>;

  /// @brief Clones a range if it is not an input or output range.
  /// @param range The range to save.
  /// @returns Either an implementation-defined object that indicates the save could not be done, or a
  /// move/copy of the given range.
  template <typename TRange>
  constexpr auto SaveRange(TRange &&range) noexcept
  {
    if constexpr (SameType<remove_cvref_t<TRange>, range_iterator_unsaveable_t>)
    {
      return range_iterator_unsaveable;
    }
    else if constexpr (InputOrOutputRange<remove_cvref_t<TRange>>)
    {
      return range_iterator_unsaveable;
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
    if constexpr (SameType<remove_cvref_t<TRange>, range_iterator_unsaveable_t>)
    {
      return std::forward<TFallbackRange>(fallbackRange);
    }
    else if constexpr (InputOrOutputRange<remove_cvref_t<TRange>>)
    {
      return std::forward<TFallbackRange>(fallbackRange);
    }
    else
    {
      return std::forward<TRange>(range);
    }
  }
}