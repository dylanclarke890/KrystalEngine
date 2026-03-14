#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Impl/ContiguousIterator.hpp"
#include "Krystal.Lib/Utils/ToAddress.hpp"
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename T, typename... TArgs>
    concept has_next = requires() { std::declval<T>().next(std::declval<TArgs>()...); };

    template <typename T, typename... TArgs>
    concept has_prev = requires() { std::declval<T>().prev(std::declval<TArgs>()...); };

    template <typename T>
    using has_lvalue_increment = decltype(++std::declval<T &>());

    template <typename T>
    using has_lvalue_decrement = decltype(--std::declval<T &>());

    template <typename TIterator, typename TSentinel, typename = void>
    struct is_distance_operable : std::false_type
    {
    };

    template <typename TIterator, typename TSentinel>
    struct is_distance_operable<TIterator, TSentinel,
                                void_t<decltype(std::declval<TSentinel>() - std::declval<TIterator>())>>
        : IntegralConstant<bool,
                           ConvertibleTo<decltype(std::declval<TSentinel>() - std::declval<TIterator>()),
                                         iterator_difference_type_t<TIterator>>>
    {
    };

    template <typename TIterator, typename TSentinel>
    concept DistanceOperable = is_distance_operable<TIterator, TSentinel>::value;
  }

  template <typename TIterator>
  concept InputIterator = IsIteratorConceptOrBetter<std::input_iterator_tag, TIterator>;

  template <typename TIterator>
  concept OutputIterator = IsIteratorConceptOrBetter<std::output_iterator_tag, TIterator>;

  template <typename TIterator>
  concept InputOrOutputIterator = InputIterator<TIterator> || OutputIterator<TIterator>;

  template <typename TIterator>
  concept ForwardIterator = IsIteratorConceptOrBetter<std::forward_iterator_tag, TIterator>;

  template <typename TIterator>
  concept BidirectionalIterator = IsIteratorConceptOrBetter<std::bidirectional_iterator_tag, TIterator>;

  template <typename TIterator>
  concept RandomAccessIterator = IsIteratorConceptOrBetter<std::random_access_iterator_tag, TIterator>;

  template <typename TIterator>
  concept ContiguousIterator =
    Impl::IsContiguousIterator<TIterator>
    || (
      IsIteratorConceptOrBetter<std::contiguous_iterator_tag, TIterator> &&
      ToAddressable<TIterator> && LValueRef<std::iter_reference_t<remove_ref_t<TIterator>>>);

  template <typename TIterator>
  concept InputIteratorExact = SameType<std::input_iterator_tag, iterator_concept_t<TIterator>>;

  template <typename TIterator>
  concept OutputIteratorExact = SameType<std::output_iterator_tag, iterator_concept_t<TIterator>>;

  template <typename TIterator>
  concept InputOrOutputIteratorExact = InputIteratorExact<TIterator> || OutputIteratorExact<TIterator>;

  template <typename TIterator>
  concept ForwardIteratorExact = SameType<std::forward_iterator_tag, iterator_concept_t<TIterator>>;

  template <typename TIterator>
  concept BidirectionalIteratorExact =
    SameType<std::bidirectional_iterator_tag, iterator_concept_t<TIterator>>;

  template <typename TIterator>
  concept RandomAccessIteratorExact =
    SameType<std::random_access_iterator_tag, iterator_concept_t<TIterator>>;

  template <typename TIterator>
  concept ContiguousIteratorExact = SameType<std::contiguous_iterator_tag, iterator_concept_t<TIterator>>;

  template <typename TIterator, typename TSentinel>
  concept SizedSentinelFor = Impl::DistanceOperable<TIterator, TSentinel>;
}
