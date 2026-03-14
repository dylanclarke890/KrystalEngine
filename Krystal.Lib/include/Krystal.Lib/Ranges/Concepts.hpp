#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include <iterator>
#include <ranges>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TIterator, typename = void>
    struct IteratorPointerOrFallback
    {
    private:
      using TReference = std::iter_reference_t<remove_ref_t<TIterator>>;

    public:
      using type = conditional_t<Reference<TReference>, add_pointer_t<remove_ref_t<TReference>>, void>;
    };

    template <typename TIterator>
    struct IteratorPointerOrFallback<TIterator,
                                     void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::pointer>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::pointer;
    };

    template <typename TRange, bool = std::ranges::range<TRange>>
    class IsInputRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsInputRangeExactly<TRange, true>
        : public IntegralConstant<bool, InputIteratorExact<std::ranges::iterator_t<TRange>>>
    {
    };

    template <typename TRange, bool = std::ranges::range<TRange>>
    class IsOutputRangeExactly : public std::false_type
    {
    };

    template <typename TRange>
    class IsOutputRangeExactly<TRange, true>
        : public IntegralConstant<bool, OutputIteratorExact<std::ranges::iterator_t<TRange>>>
    {
    };
  }

  template <typename TIterator>
  using iterator_pointer_t = typename Impl::IteratorPointerOrFallback<remove_ref_t<TIterator>>::type;

  template <typename TRange>
  using range_pointer_t = Krys::Ranges::iterator_pointer_t<std::ranges::iterator_t<TRange>>;

  template <typename TRange>
  concept InputRangeExact = Impl::IsInputRangeExactly<TRange>::value;

  template <typename TRange>
  concept OutputRangeExact = Impl::IsOutputRangeExactly<TRange>::value;

  template <typename TRange>
  concept InputOrOutputRangeExact = InputRangeExact<TRange> || OutputRangeExact<TRange>;

  template <typename TRange>
  using range_iterator_concept_t = iterator_concept_t<std::ranges::iterator_t<TRange>>;

  template <typename TTag, typename TRange>
  concept IsRangeIteratorConceptOrBetter = DerivedFrom<TTag, range_iterator_concept_t<TRange>>;

  template <typename TRange, typename TIteratorFirst, typename TIteratorLast = TIteratorFirst>
  concept has_insert_bulk = requires {
    std::declval<TRange>().insert(begin(std::declval<TRange>()), std::declval<TIteratorFirst>(),
                                  std::declval<TIteratorLast>());
  };

  template <typename TRange, typename TElement>
  concept has_push_back = requires { std::declval<TRange>().push_back(std::declval<TElement>()); };

  template <typename T, typename TSize = std::size_t>
  concept has_reserve_with_size = requires { std::declval<T>().reserve(std::declval<TSize>()); };
}