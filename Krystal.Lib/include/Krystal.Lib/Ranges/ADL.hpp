#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <limits>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TIterator, typename = void>
    struct IteratorValueTypeFromRefOrVoid
    {
      using type = void;
    };

    template <typename TIterator>
    struct IteratorValueTypeFromRefOrVoid<TIterator, void_t<decltype(*std::declval<TIterator &>())>>
    {
      using type = remove_ref_t<decltype(*std::declval<TIterator &>())>;
    };

    template <typename TIterator, typename = void>
    struct IteratorValueTypeOrFallback
    {
      using type = typename IteratorValueTypeFromRefOrVoid<TIterator>::type;
    };

    template <typename TIterator>
    struct IteratorValueTypeOrFallback<TIterator, void_t<std::iter_value_t<TIterator>>>
    {
      using type = std::iter_value_t<TIterator>;
    };

    template <typename TIterator, typename = void>
    struct IteratorDifferenceTypeOrFallback
    {
      using type = std::ptrdiff_t;
    };

    template <typename TIterator>
    struct IteratorDifferenceTypeOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::difference_type>>
    {
    private:
      using maybe_void_type = typename std::iterator_traits<remove_ref_t<TIterator>>::difference_type;

    public:
      using type = conditional_t<Void<maybe_void_type>, std::ptrdiff_t, maybe_void_type>;
    };

    template <typename TIterator, typename = void>
    struct IteratorValueTypeInterception
    {
      using type = typename IteratorValueTypeOrFallback<remove_ref_t<TIterator>>::type;
    };

    template <typename TContainer>
    struct IteratorValueTypeInterception<std::back_insert_iterator<TContainer>>
    {
      using type = typename remove_cvref_t<unwrap_t<TContainer>>::value_type;
    };

    template <typename TIterator, typename = void>
    struct IteratorCategoryFailure
    {
      using type = conditional_t<IsPointer<remove_cvref_t<TIterator>>, std::contiguous_iterator_tag,
                                 std::output_iterator_tag>;
    };

    template <typename TIterator>
    struct IteratorCategoryFailure<TIterator, void_t<typename remove_ref_t<TIterator>::iterator_category>>
    {
      using type = typename remove_ref_t<TIterator>::iterator_category;
    };

    template <typename TIterator, typename = void>
    struct IteratorCategoryOrFallback
    {
      using type = typename IteratorCategoryFailure<TIterator>::type;
    };

    template <typename TIterator>
    struct IteratorCategoryOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_category>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_category;
    };

    template <typename TIterator, typename = void>
    struct IteratorConceptFailure
    {
      using type = conditional_t<IsPointer<remove_cvref_t<TIterator>>, std::contiguous_iterator_tag,
                                 std::output_iterator_tag>;
    };

    template <typename TIterator>
    struct IteratorConceptFailure<TIterator, void_t<typename remove_ref_t<TIterator>::iterator_concept>>
    {
      using type = typename remove_ref_t<TIterator>::iterator_concept;
    };

    template <typename TIterator, typename = void>
    struct IteratorConceptOrFallback
    {
      using type = typename IteratorConceptFailure<TIterator>::type;
    };

    template <typename TIterator>
    struct IteratorConceptOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_concept>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_concept;
    };

    template <typename TIterator, typename = void>
    struct IteratorCategoryOrConceptOrFallback
    {
    private:
      using MaybeType = typename IteratorCategoryOrFallback<TIterator>::type;

    public:
      using type =
        conditional_t<SameType<MaybeType, std::output_iterator_tag>,
                      typename IteratorConceptOrFallback<remove_cvref_t<TIterator>>::type, MaybeType>;
    };

    template <typename TIterator>
    struct IteratorCategoryOrConceptOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_category>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_category;
    };

    template <typename TIterator, typename = void>
    struct IteratorConceptOrCategoryOrFallback
    {
    private:
      using MaybeType = typename IteratorConceptOrFallback<TIterator>::type;

    public:
      using type =
        conditional_t<SameType<MaybeType, std::output_iterator_tag>,
                      typename IteratorCategoryOrFallback<remove_cvref_t<TIterator>>::type, MaybeType>;
    };

    template <typename TIterator>
    struct IteratorConceptOrCategoryOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_concept>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::iterator_concept;
    };

    template <typename TIterator, typename = void>
    struct IteratorElementTypeOrFallback
    {
    private:
      using TReference = std::iter_reference_t<remove_ref_t<TIterator>>;

    public:
      using type = conditional_t<Reference<TReference>, remove_ref_t<TReference>, void>;
    };

    template <typename TIterator>
    struct IteratorElementTypeOrFallback<TIterator, void_t<typename remove_ref_t<TIterator>::element_type>>
    {
      using type = typename remove_ref_t<TIterator>::element_type;
    };

    template <typename TIterator>
    constexpr auto FindIteratorConceptType() noexcept
    {
      if constexpr (std::contiguous_iterator<TIterator>)
      {
        return std::contiguous_iterator_tag {};
      }
      else if constexpr (std::random_access_iterator<TIterator>)
      {
        return std::random_access_iterator_tag {};
      }
      else if constexpr (std::bidirectional_iterator<TIterator>)
      {
        return std::bidirectional_iterator_tag {};
      }
      else if constexpr (std::forward_iterator<TIterator>)
      {
        return std::forward_iterator_tag {};
      }
      else if constexpr (std::contiguous_iterator<TIterator>)
      {
        return std::input_iterator_tag {};
      }
      else
      {
        using fallback_t = typename IteratorConceptOrCategoryOrFallback<remove_ref_t<TIterator>>::type;
        return fallback_t {};
      }
    }

    template <typename TIterator>
    using iterator_concept_or_fallback_t =
      decltype(::Krys::Ranges::Impl::FindIteratorConceptType<TIterator>());

    template <typename TIterator>
    using iterator_category_or_fallback_cascade_t = iterator_concept_or_fallback_t<TIterator>;
  }

  template <typename TIterator>
  using iterator_difference_type_t = std::iter_difference_t<TIterator>;

  template <typename TIterator>
  using iterator_value_type_t = typename Impl::IteratorValueTypeInterception<TIterator>::type;

  template <typename TIterator>
  using iterator_element_type_t = typename Impl::IteratorElementTypeOrFallback<TIterator>::type;

  template <typename TIterator>
  using iterator_size_type_t = make_signed_t<iterator_difference_type_t<remove_ref_t<TIterator>>>;

  template <typename TRange>
  concept HasSizeADL = requires { ::std::ranges::size(std::declval<add_lvalue_ref_t<TRange>>()); };

  template <typename TRange>
  concept HasEmptyADL = requires { ::std::ranges::empty(std::declval<add_lvalue_ref_t<TRange>>()); };

  template <typename TRange>
  concept HasBeginADL = requires { ::std::ranges::begin(std::declval<TRange>()); };

  template <typename TRange>
  concept HasEndADL = requires { ::std::ranges::end(std::declval<TRange>()); };

  template <typename TIterator>
  using iterator_rvalue_reference_t = decltype(::std::ranges::iter_move(std::declval<TIterator &>()));

  template <typename TRange>
  using range_sentinel_t = std::ranges::sentinel_t<TRange>;

  template <typename TRange>
  using range_value_type_t = std::ranges::range_value_t<TRange>;

  template <typename TRange>
  using range_reference_t = std::ranges::range_reference_t<TRange>;

  template <typename TRange>
  using range_rvalue_reference_t = std::ranges::range_rvalue_reference_t<TRange>;

  template <typename TRange>
  using range_difference_type_t = std::ranges::range_difference_t<TRange>;

  template <typename TRange>
  using range_size_type_t = std::ranges::range_size_t<TRange>;

  template <typename TIterator>
  using iterator_category_t = Krys::Ranges::Impl::iterator_category_or_fallback_cascade_t<TIterator>;

  template <typename TIterator>
  using iterator_concept_t = Krys::Ranges::Impl::iterator_concept_or_fallback_t<TIterator>;

  template <typename TRange>
  using range_element_type_t = iterator_element_type_t<::std::ranges::iterator_t<TRange>>;

  template <typename TRange>
  using range_const_iterator_t =
    remove_cvref_t<decltype(::std::ranges::cbegin(std::declval<add_lvalue_ref_t<TRange>>()))>;

  template <typename TRange>
  using range_const_sentinel_t =
    remove_cvref_t<decltype(::std::ranges::cend(std::declval<add_lvalue_ref_t<TRange>>()))>;

  template <typename TTag, typename TActualTag>
  concept IsConceptOrBetter = DerivedFrom<TTag, TActualTag>;

  template <typename TTag, typename TIterator>
  concept IsIteratorCategoryOrBetter = IsConceptOrBetter<TTag, iterator_category_t<TIterator>>;

  template <typename TTag, typename TIterator>
  concept IsIteratorConceptOrBetter = IsConceptOrBetter<TTag, iterator_concept_t<TIterator>>;

  template <typename T, typename... TArgs>
  concept HasIterAdvance = requires(T &&t, TArgs &&...args) {
    { t.advance(std::forward<TArgs>(args)...) };
  };

  template <typename T, typename... TArgs>
  concept HasIterRecede = requires(T &&t, TArgs &&...args) {
    { t.recede(std::forward<TArgs>(args)...) };
  };

  namespace detail
  {
    template <typename TIterator, typename... TArgs>
    consteval bool NoThrowIterAdvance() noexcept
    {
      if constexpr (HasIterAdvance<TIterator, TArgs...>)
      {
        return noexcept(std::declval<TIterator>().advance(std::declval<TArgs>()...));
      }
      else
      {
        return noexcept(++std::declval<add_lvalue_ref_t<remove_ref_t<TIterator>>>());
      }
    }

    template <typename TIterator, typename... TArgs>
    consteval bool NoThrowIterRecede() noexcept
    {
      if constexpr (HasIterRecede<TIterator, TArgs...>)
      {
        return noexcept(std::declval<TIterator>().recede(std::declval<TArgs>()...));
      }
      else
      {
        return noexcept(--std::declval<add_lvalue_ref_t<remove_ref_t<TIterator>>>());
      }
    }

    namespace adl
    {
      class iter_advance_fn
      {
      public:
        template <typename TIterator>
        constexpr decltype(auto) operator()(TIterator &&it) const noexcept(NoThrowIterAdvance<TIterator>())
        {
          if constexpr (::Krys::Ranges::HasIterAdvance<TIterator>)
          {
            std::forward<TIterator>(it).advance();
          }
          else
          {
            ++it;
          }
          return std::forward<TIterator>(it);
        }

        template <typename TIterator, typename TDiff>
        constexpr decltype(auto) operator()(TIterator &&it, TDiff diff) const
          noexcept(NoThrowIterAdvance<TIterator, TDiff>())
        {
          if constexpr (::Krys::Ranges::HasIterAdvance<TIterator, TDiff>)
          {
            std::forward<TIterator>(it).advance(diff);
          }
          else
          {
            if constexpr (IsIteratorConceptOrBetter<std::random_access_iterator_tag,
                                                    remove_cvref_t<TIterator>>)
            {
              it += diff;
            }
            else
            {
              for (; diff > 0; --diff)
              {
                this->operator()(std::forward<TIterator>(it));
              }
            }
          }
          return std::forward<TIterator>(it);
        }
      };

      class iter_recede_fn
      {
      public:
        template <typename TIterator>
        constexpr decltype(auto) operator()(TIterator &&it) const noexcept(NoThrowIterRecede<TIterator>())
        {
          if constexpr (::Krys::Ranges::HasIterRecede<TIterator>)
          {
            std::forward<TIterator>(it).recede();
          }
          else
          {
            --it;
          }
          return std::forward<TIterator>(it);
        }

        template <typename TIterator, typename TDiff>
        constexpr decltype(auto) operator()(TIterator &&it, TDiff diff) const
          noexcept(NoThrowIterRecede<TIterator, TDiff>())
        {
          if constexpr (::Krys::Ranges::HasIterRecede<TIterator, TDiff>)
          {
            std::forward<TIterator>(it).recede(diff);
          }
          else
          {
            if constexpr (::Krys::Ranges::IsIteratorConceptOrBetter<std::random_access_iterator_tag,
                                                                    remove_cvref_t<TIterator>>)
            {
              it -= diff;
            }
            else
            {
              for (; diff > 0; --diff)
              {
                --it;
              }
            }
            return std::forward<TIterator>(it);
          }
        }
      };
    }
  }

  using ViewBase = std::ranges::view_base;

  inline constexpr ::Krys::Ranges::detail::adl::iter_advance_fn iter_advance {};
  inline constexpr ::Krys::Ranges::detail::adl::iter_recede_fn iter_recede {};
}