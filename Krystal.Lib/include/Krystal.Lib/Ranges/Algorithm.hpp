#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/CountedIterator.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"
#include "Krystal.Lib/Ranges/Subrange.hpp"
#include "Krystal.Lib/Ranges/Unbounded.hpp"
#include "Krystal.Lib/Ranges/UnreachableSentinel.hpp"
#include "Krystal.Lib/Utils/ToAddress.hpp"
#include <algorithm>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TInIteratorOrRange, typename TOutIteratorOrRange>
    struct InOutResult
    {
      TInIteratorOrRange In;
      TOutIteratorOrRange Out;

      template <typename TArgInIterator, typename TArgOutIterator>
      requires(ConvertibleTo<const TInIteratorOrRange &, TArgInIterator>
               && ConvertibleTo<const TOutIteratorOrRange &, TArgOutIterator>)
      constexpr operator InOutResult<TArgInIterator, TArgOutIterator>() const &
      {
        return {In, Out};
      }

      template <typename TArgInIterator, typename TArgOutIterator>
      requires(ConvertibleTo<const TInIteratorOrRange &, TArgInIterator>
               && ConvertibleTo<const TOutIteratorOrRange &, TArgOutIterator>)
      constexpr operator InOutResult<TArgInIterator, TArgOutIterator>() &
      {
        return {In, Out};
      }

      template <typename TArgInIterator, typename TArgOutIterator>
      requires(ConvertibleTo<TInIteratorOrRange, TArgInIterator>
               && ConvertibleTo<TOutIteratorOrRange, TArgOutIterator>)
      constexpr operator InOutResult<TArgInIterator, TArgOutIterator>() &&
      {
        return {std::move(In), std::move(Out)};
      }
    };

    template <typename TInIteratorOrRange, typename TOutIteratorOrRange>
    struct CurrentLastResult
    {
      TInIteratorOrRange Current;
      TOutIteratorOrRange Last;

      template <typename TArgInIterator, typename TArgOutIterator>
      requires(ConvertibleTo<const TInIteratorOrRange &, TArgInIterator>
               && ConvertibleTo<const TOutIteratorOrRange &, TArgOutIterator>)
      constexpr operator CurrentLastResult<TArgInIterator, TArgOutIterator>() const &
      {
        return {Current, Last};
      }

      template <typename TArgInIterator, typename TArgOutIterator>
      requires(ConvertibleTo<const TInIteratorOrRange &, TArgInIterator>
               && ConvertibleTo<const TOutIteratorOrRange &, TArgOutIterator>)
      constexpr operator CurrentLastResult<TArgInIterator, TArgOutIterator>() &
      {
        return {Current, Last};
      }

      template <typename TArgInIterator, typename TArgOutIterator>
      requires(ConvertibleTo<TInIteratorOrRange, TArgInIterator>
               && ConvertibleTo<TOutIteratorOrRange, TArgOutIterator>)
      constexpr operator CurrentLastResult<TArgInIterator, TArgOutIterator>() &&
      {
        return {std::move(Current), std::move(Last)};
      }
    };
  }

  template <typename TInIteratorOrRange, typename TOutIteratorOrRange>
  using InOutResult = std::ranges::in_out_result<TInIteratorOrRange, TOutIteratorOrRange>;

  template <typename TInIteratorOrRange, typename TOutIteratorOrRange>
  using CurrentLastResult = Impl::CurrentLastResult<TInIteratorOrRange, TOutIteratorOrRange>;

  namespace Impl
  {
    template <typename TIterator0, typename TIterator1>
    constexpr TIterator0 ReverseRange(TIterator0 first, TIterator1 last) noexcept
    {
      return std::ranges::reverse(std::move(first), std::move(last));
    }

    template <typename TFirst, typename, typename TOutFirst>
    constexpr bool IsCopyUnsafeNoexcept() noexcept
    {
      return NoThrowAssignable<iterator_reference_t<TOutFirst>, iterator_reference_t<TFirst>>;
    }

    template <typename TFirst, typename TLast, typename TOutFirst, typename TOutLast>
    constexpr bool IsCopyNoexcept() noexcept
    {
      if constexpr (SizedSentinelFor<TOutFirst, TOutLast>)
      {
        return IsCopyUnsafeNoexcept<TFirst, TLast, TOutFirst>()
               && NoThrowAssignable<iterator_reference_t<TOutFirst>, iterator_reference_t<TFirst>>;
      }
      else
      {
        return NoThrowAssignable<iterator_reference_t<TOutFirst>, iterator_reference_t<TFirst>>;
      }
    }

    template <typename TFirst, typename TFirstCount, typename TOutFirst>
    constexpr auto
      CopyNUnsafe(TFirst first, TFirstCount size,
                  TOutFirst outFirst) noexcept(IsCopyUnsafeNoexcept<TFirst, TFirstCount, TOutFirst>())
    {
      using TResultInIterator = CountedIterator<TFirst>;
      using TInRange = subrange<TResultInIterator, DefaultSentinel>;
      using TOutRange = UnboundedView<TOutFirst>;
      using TResult = InOutResult<TInRange, TOutRange>;

      if (!std::is_constant_evaluated())
      {
        using TValue = iterator_value_type_t<TFirst>;
        using TOutValue = iterator_value_type_t<TOutFirst>;

        if constexpr (IsContiguousIterator<TFirst> && HasUniqueObjectRepresentations<TValue>
                      && IsContiguousIterator<TOutFirst> && HasUniqueObjectRepresentations<TOutValue>)
        {
          auto first_ptr = to_address(first);
          auto distance = size;
          std::size_t byteDistance = sizeof(TValue) * distance;
          std::size_t outDistance = byteDistance / sizeof(TOutValue);
          std::memcpy(to_address(outFirst), first_ptr, byteDistance);
          return TResult {TInRange(TResultInIterator(std::move(first) + distance, 0), default_sentinel),
                          TOutRange(std::move(outFirst) + outDistance)};
        }
      }
      TFirstCount currentSize = 0;
      for (; currentSize < size; ++first, (void)++outFirst, (void)++currentSize)
      {
        *outFirst = *first;
      }
      return TResult {TInRange(TResultInIterator(std::move(first), 0), default_sentinel),
                      TOutRange(std::move(outFirst))};
    }

    template <typename TFirst, typename TLast, typename TOutFirst>
    constexpr auto CopyUnsafe(TFirst first, TLast last,
                              TOutFirst outFirst) noexcept(IsCopyUnsafeNoexcept<TFirst, TLast, TOutFirst>())
    {
      using TInRange = subrange<TFirst, TLast>;
      using TOutRange = UnboundedView<TOutFirst>;
      using TResult = InOutResult<TInRange, TOutRange>;

      if (!std::is_constant_evaluated())
      {
        using TValue = iterator_value_type_t<TFirst>;
        using TOutValue = iterator_value_type_t<TOutFirst>;
        if constexpr (IsContiguousIterator<TFirst> && HasUniqueObjectRepresentations<TValue>
                      && IsContiguousIterator<TOutFirst> && HasUniqueObjectRepresentations<TOutValue>)
        {
          auto first_ptr = to_address(first);
          auto distance = last - first;
          std::size_t byteDistance = sizeof(TValue) * distance;
          std::size_t outDistance = byteDistance / sizeof(TOutValue);
          std::memcpy(to_address(outFirst), first_ptr, byteDistance);
          return TResult {TInRange(std::move(first) + distance, std::move(last)),
                          TOutRange(std::move(outFirst) + outDistance)};
        }
      }

      for (; first != last; ++first, (void)++outFirst)
      {
        *outFirst = *first;
      }
      return TResult {TInRange(std::move(first), std::move(last)), TOutRange(std::move(outFirst))};
    }

    template <typename TFirst, typename TFirstCount, typename TOutFirst, typename TOutFirstCount>
    constexpr auto
      CopyN(TFirst first, TFirstCount size, TOutFirst outFirst,
            TOutFirstCount outSize) noexcept(IsCopyNoexcept<TFirst, TFirstCount, TOutFirst, TOutFirstCount>())
    {
      using TInRange = subrange<CountedIterator<TFirst>, DefaultSentinel>;
      using TOutRange = subrange<CountedIterator<TOutFirst>, DefaultSentinel>;
      using TResult = InOutResult<TInRange, TOutRange>;
      if (!std::is_constant_evaluated())
      {
        if constexpr (IsIteratorConceptOrBetter<std::random_access_iterator_tag, TOutFirst>)
        {
          if (size <= outSize)
          {
            auto result = CopyNUnsafe(std::move(first), size, std::move(outFirst));
            return TResult {std::move(result.In), TOutRange(result.Out.begin(), result.Out.begin() + size)};
          }
          else
          {
            auto result = CopyNUnsafe(std::move(first), outSize, std::move(outFirst));
            iterator_difference_type_t<TOutFirst> outSize_left =
              static_cast<iterator_difference_type_t<TOutFirst>>(size - outSize);
            return TResult {std::move(result.In),
                            TOutRange({std::move(result.Out).begin(), outSize_left}, default_sentinel)};
          }
        }
      }

      decltype(size) currentCount = 0;
      decltype(outSize) currentOutCount = 0;
      for (; currentCount < size && currentOutCount != outSize;
           ++first, (void)++outFirst, (void)++currentCount, (void)++currentOutCount)
      {
        *outFirst = *first;
      }

      iterator_difference_type_t<TFirst> size_left =
        static_cast<iterator_difference_type_t<TFirst>>(size - currentCount);

      iterator_difference_type_t<TOutFirst> outSize_left =
        static_cast<iterator_difference_type_t<TOutFirst>>(outSize - currentOutCount);

      return TResult {TInRange({std::move(first), size_left}, default_sentinel),
                      TOutRange({std::move(outFirst), outSize_left}, default_sentinel)};
    }

    template <typename TFirst, typename TLast, typename TOutFirst, typename TOutLast>
    constexpr auto Copy(TFirst first, TLast last, TOutFirst outFirst,
                        TOutLast outLast) noexcept(IsCopyNoexcept<TFirst, TLast, TOutFirst, TOutLast>())
    {
      using TInRange = subrange<TFirst, TLast>;
      using TOutRange = subrange<TOutFirst, TOutLast>;
      using TResult = InOutResult<TInRange, TOutRange>;

      if (!std::is_constant_evaluated())
      {
        if constexpr (SizedSentinelFor<TOutFirst, TOutLast>)
        {
          auto outSize = outLast - outFirst;
          auto size = last - first;
          if (size <= outSize)
          {
            auto result = CopyUnsafe(std::move(first), std::move(last), std::move(outFirst));
            return TResult {std::move(result.In),
                            TOutRange(std::move(result.Out).begin(), std::move(outLast))};
          }
          else
          {
            auto shortLast = first + outSize;
            auto result = CopyUnsafe(std::move(first), std::move(shortLast), std::move(outFirst));
            return TResult {std::move(result.In),
                            TOutRange(std::move(result.Out).begin(), std::move(outLast))};
          }
        }
      }

      for (; first != last && outFirst != outLast; ++first, (void)++outFirst)
      {
        *outFirst = *first;
      }
      return TResult {TInRange(std::move(first), std::move(last)),
                      TOutRange(std::move(outFirst), std::move(outLast))};
    }

    template <typename TInput, typename TOutput>
    constexpr auto Copy(TInput &&input, TOutput &&output) noexcept(
      IsCopyNoexcept<range_const_iterator_t<TInput>, range_const_sentinel_t<TInput>,
                     range_iterator_t<TOutput>, range_sentinel_t<TOutput>>())
    {
      return Impl::Copy(Krys::Ranges::cbegin(std::forward<TInput>(input)), Krys::Ranges::cend(input),
                        Krys::Ranges::begin(std::forward<TOutput>(output)), Krys::Ranges::end(output));
    }

    template <typename TFirst0, typename TLast0, typename TFirst1, typename TLast1>
    constexpr int LexicographicalCompareThreeWayBasic(TFirst0 first0, TLast0 last0, TFirst1 first1,
                                                      TLast1 last1)
    {
      for (; (first0 != last0) && (first1 != last1); ++first0, (void)++first1)
      {
        if (*first0 < *first1)
          return -1;
        if (*first1 < *first0)
          return 1;
      }
      bool firstlast0_exhausted = (first0 == last0);
      bool firstlast1_exhausted = (first1 == last1);
      if (firstlast0_exhausted && firstlast1_exhausted)
      {
        return 0;
      }
      else if (firstlast0_exhausted)
      {
        return -1;
      }
      else
      {
        return 1;
      }
    }

    template <typename TFirst0, typename TLast0, typename TFirst1, typename TLast1>
    constexpr bool LexicographicalCompare(TFirst0 first0, TLast0 last0, TFirst1 first1, TLast1 last1) noexcept
    {
      return std::ranges::lexicographical_compare(std::move(first0), std::move(last0), std::move(first1),
                                                  std::move(last1));
    }

  }

  template <typename TIterator, typename TLast>
  constexpr auto distance(TIterator &&it, TLast &&last) noexcept
  {
    if constexpr (IsIteratorConceptOrBetter<std::random_access_iterator_tag, TIterator>)
    {
      return last - it;
    }
    else
    {
      iterator_difference_type_t<TIterator> diff = 0;
      for (; it != last; ++it)
      {
        ++diff;
      }
      return diff;
    }
  }

  template <typename TIterator, typename TLast, typename TPredicate>
  constexpr CurrentLastResult<TIterator, TLast> find_if(TIterator first, TLast last,
                                                        TPredicate predicate) noexcept
  {
    for (; first != last; ++first)
    {
      if (predicate(*first))
      {
        return {std::move(first), std::move(last)};
      }
    }
    return {std::move(first), std::move(last)};
  }

  template <typename TIterator, typename TLast, typename T, typename TCompare>
  constexpr CurrentLastResult<TIterator, TLast> lower_bound(TIterator first, TLast last, T &&targetValue,
                                                            TCompare compare) noexcept
  {
    using TDiff = iterator_difference_type_t<TIterator>;
    if (!std::is_constant_evaluated())
    {
      if constexpr (SameType<TIterator, TLast>)
      {
        auto it = std::lower_bound(first, last, targetValue, compare);
        return {std::move(it), std::move(last)};
      }
    }
    TIterator it = {};
    TDiff count = Krys::Ranges::distance(first, last);
    TDiff step = 0;
    while (count > 0)
    {
      it = first;
      step = count / 2;
      Krys::Ranges::iter_advance(it, step);
      if (compare(*it, targetValue))
      {
        first = ++it;
        count -= step + 1;
      }
      else
      {
        count = step;
      }
    }
    return {std::move(first), std::move(last)};
  }

  template <typename TIteratorLast, typename T, typename TCompare>
  constexpr auto lower_bound(TIteratorLast &&it_last, T &&targetValue, TCompare compare) noexcept
  {
    return std::ranges::lower_bound(cbegin(std::forward<TIteratorLast>(it_last)), cend(it_last),
                                    std::forward<T>(targetValue), std::forward<TCompare>(compare));
  }

  template <typename TIterator0, typename TSentinel0, typename TIterator1, typename TSentinel1>
  constexpr bool equal(TIterator0 first0, TSentinel0 last0, TIterator1 first1, TSentinel1 last1)
  {
    return std::ranges::equal(std::move(first0), std::move(last0), std::move(first1), std::move(last1));
  }

  template <typename TIterator0, typename TSentinel0, typename TIterator1, typename TSentinel1,
            typename TPredicate>
  constexpr bool equal(TIterator0 first0, TSentinel0 last0, TIterator1 first1, TSentinel1 last1,
                       TPredicate &&predicate)
  {
    return std::ranges::equal(std::move(first0), std::move(last0), std::move(first1), std::move(last1),
                              std::forward<TPredicate>(predicate));
  }

  template <typename TFirstLast0, typename TFirstLast1>
  constexpr bool equal(TFirstLast0 &&first_last0, TFirstLast1 &&first_last1)
  {
    return std::ranges::equal(std::forward<TFirstLast0>(first_last0), std::forward<TFirstLast1>(first_last1));
  }

  template <typename TFirstLast0, typename TFirstLast1, typename TPredicate>
  constexpr bool equal(TFirstLast0 &&first_last0, TFirstLast1 &&first_last1, TPredicate &&predicate)
  {
    return std::ranges::equal(std::forward<TFirstLast0>(first_last0), std::forward<TFirstLast1>(first_last1),
                              std::forward<TPredicate>(predicate));
  }
}