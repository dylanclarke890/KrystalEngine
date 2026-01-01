#pragma once

#include "Krystal.Lib/Core/Config.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include <iterator>
#include <type_traits>
#include <utility>

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  #include <ranges>
#endif

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TRange, bool = IsRange<TRange>>
    class IsInputRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsInputRange<TRange, true> : public IntegralConstant<bool, InputIterator<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsOutputRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsOutputRange<TRange, true>
        : public IntegralConstant<bool, OutputIterator<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsForwardRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsForwardRange<TRange, true>
        : public IntegralConstant<bool, ForwardIterator<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsBidirectionalRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsBidirectionalRange<TRange, true>
        : public IntegralConstant<bool, BidirectionalIterator<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsRandomAccessRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsRandomAccessRange<TRange, true>
        : public IntegralConstant<bool, RandomAccessIterator<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsContiguousRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsContiguousRange<TRange, true>
        : public IntegralConstant<bool, ContiguousIterator<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsInputRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsInputRangeExactly<TRange, true>
        : public IntegralConstant<bool, InputIteratorExact<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsOutputRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsOutputRangeExactly<TRange, true>
        : public IntegralConstant<bool, OutputIteratorExact<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsForwardRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsForwardRangeExactly<TRange, true>
        : public IntegralConstant<bool, ForwardIteratorExact<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsBidirectionalRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsBidirectionalRangeExactly<TRange, true>
        : public IntegralConstant<bool, BidirectionalIteratorExact<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsRandomAccessRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsRandomAccessRangeExactly<TRange, true>
        : public IntegralConstant<bool, RandomAccessIteratorExact<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsContiguousRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsContiguousRangeExactly<TRange, true>
        : public IntegralConstant<bool, ContiguousIteratorExact<range_iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = IsRange<TRange>>
    class IsSizedRange : public std::false_type
    {
    };

    template <typename TRange>
    class IsSizedRange<TRange, true>
        : public IntegralConstant<bool, SizedSentinelFor<range_iterator_t<TRange>, range_sentinel_t<TRange>>>
    {
    };
  }

  template <typename TRange>
  using range_pointer_t = iterator_pointer_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_iterator_category_t = iterator_category_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_iterator_concept_t = iterator_concept_t<range_iterator_t<TRange>>;

  template <typename TTag, typename TRange>
  concept IsRangeIteratorConceptOrBetter = DerivedFrom<TTag, range_iterator_concept_t<TRange>>;

  template <typename TRange>
  concept InputRange = Impl::IsInputRange<TRange>::value;

  template <typename TRange>
  concept OutputRange = Impl::IsOutputRange<TRange>::value;

  template <typename TRange>
  concept InputOrOutputRange = InputRange<TRange> || OutputRange<TRange>;

  template <typename TRange>
  concept ForwardRange = Impl::IsForwardRange<TRange>::value;

  template <typename TRange>
  concept BidirectionalRange = Impl::IsBidirectionalRange<TRange>::value;

  template <typename TRange>
  concept RandomAccessRange = Impl::IsRandomAccessRange<TRange>::value;

  template <typename TRange>
  concept ContiguousRange = Impl::IsContiguousRange<TRange>::value;

  template <typename TRange>
  concept SizedRange = Impl::IsSizedRange<TRange>::value;

  template <typename TRange>
  concept InputRangeExact = Impl::IsInputRangeExactly<TRange>::value;

  template <typename TRange>
  concept OutputRangeExact = Impl::IsOutputRangeExactly<TRange>::value;

  template <typename TRange>
  concept InputOrOutputRangeExact = InputRangeExact<TRange> || OutputRangeExact<TRange>;

  template <typename TRange>
  concept ForwardRangeExact = Impl::IsForwardRangeExactly<TRange>::value;

  template <typename TRange>
  concept BidirectionalRangeExact = Impl::IsBidirectionalRangeExactly<TRange>::value;

  template <typename TRange>
  concept RandomAccessRangeExact = Impl::IsRandomAccessRangeExactly<TRange>::value;

  template <typename TRange>
  concept ContiguousRangeExact = Impl::IsContiguousRangeExactly<TRange>::value;

  /// @brief Checks if the given type has a .reserve member function on it that takes the provided size type.
  template <typename T, typename TSize = std::size_t>
  concept has_reserve_with_size = requires { std::declval<T>().reserve(std::declval<TSize>()); };

  template <typename TRange, typename TElement>
  concept has_push_back = requires { std::declval<TRange>().push_back(std::declval<TElement>()); };

  template <typename TRange, typename TIteratorFirst, typename TIteratorLast = TIteratorFirst>
  concept has_insert_bulk = requires {
    std::declval<TRange>().insert(Krys::Ranges::begin(std::declval<TRange>()), std::declval<TIteratorFirst>(),
                                  std::declval<TIteratorLast>());
  };
}
