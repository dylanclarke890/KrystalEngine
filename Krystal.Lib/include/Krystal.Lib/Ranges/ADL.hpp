#pragma once

#include "Krystal.Lib/Core/Config.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Utils/ContiguousIteratorTag.hpp"
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
    struct IteratorReferenceOrFallback
    {
      using type = decltype(*std::declval<TIterator &>());
    };

    template <typename TIterator>
    struct IteratorReferenceOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::reference>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::reference;
    };
  }

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  template <typename TIterator>
  using iterator_reference_t = std::iter_reference_t<TIterator>;
#else
  template <typename TIterator>
  using iterator_reference_t = typename Impl::IteratorReferenceOrFallback<remove_ref_t<TIterator>>::type;
#endif

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

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
    template <typename TIterator>
    struct IteratorValueTypeOrFallback<TIterator, void_t<std::iter_value_t<TIterator>>>
    {
      using type = std::iter_value_t<TIterator>;
    };
#else
    template <typename TIterator>
    struct IteratorValueTypeOrFallback<
      TIterator, void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::value_type>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::value_type;
    };
#endif

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
      using type = conditional_t<IsPointer<remove_cvref_t<TIterator>>, contiguous_iterator_tag,
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
      using type = conditional_t<IsPointer<remove_cvref_t<TIterator>>, contiguous_iterator_tag,
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
    struct IteratorPointerOrFallback
    {
    private:
      using TReference = iterator_reference_t<remove_ref_t<TIterator>>;

    public:
      using type = conditional_t<Reference<TReference>, add_pointer_t<remove_ref_t<TReference>>, void>;
    };

    template <typename TIterator>
    struct IteratorPointerOrFallback<TIterator,
                                     void_t<typename std::iterator_traits<remove_ref_t<TIterator>>::pointer>>
    {
      using type = typename std::iterator_traits<remove_ref_t<TIterator>>::pointer;
    };

    template <typename TIterator, typename = void>
    struct IteratorElementTypeOrFallback
    {
    private:
      using TReference = iterator_reference_t<remove_ref_t<TIterator>>;

    public:
      using type = conditional_t<Reference<TReference>, remove_ref_t<TReference>, void>;
    };

    template <typename TIterator>
    struct IteratorElementTypeOrFallback<TIterator, void_t<typename remove_ref_t<TIterator>::element_type>>
    {
      using type = typename remove_ref_t<TIterator>::element_type;
    };

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
    template <typename TIterator>
    constexpr auto FindIteratorConceptType() noexcept
    {
      if constexpr (std::contiguous_iterator<TIterator>)
      {
        return contiguous_iterator_tag {};
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
    using iterator_concept_or_fallback_t = decltype(Impl::FindIteratorConceptType<TIterator>());

    template <typename TIterator>
    using iterator_category_or_fallback_cascade_t = iterator_concept_or_fallback_t<TIterator>;
#else
    template <typename TIterator>
    using iterator_concept_or_fallback_t =
      typename IteratorConceptOrCategoryOrFallback<remove_ref_t<TIterator>>::type;

    template <typename TIterator>
    using iterator_category_or_fallback_cascade_t =
      typename Impl::IteratorCategoryOrConceptOrFallback<remove_ref_t<TIterator>>::type;
#endif
  }

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  template <typename TIterator>
  using iterator_difference_type_t = std::iter_difference_t<TIterator>;
#else
  template <typename TIterator>
  using iterator_difference_type_t =
    typename Impl::IteratorDifferenceTypeOrFallback<remove_ref_t<TIterator>>::type;
#endif

  template <typename TIterator>
  using iterator_value_type_t = typename Impl::IteratorValueTypeInterception<TIterator>::type;

  template <typename TIterator>
  using iterator_element_type_t = typename Impl::IteratorElementTypeOrFallback<TIterator>::type;

  template <typename TIterator>
  using iterator_size_type_t = make_signed_t<iterator_difference_type_t<remove_ref_t<TIterator>>>;

  namespace Impl
  {
    template <typename TRange>
    using ssize_diff_type =
      conditional_t <                                                 // cf
      std::numeric_limits<iterator_difference_type_t<TRange>>::digits // cf
      <std::numeric_limits<std::ptrdiff_t>::digits, std::ptrdiff_t, iterator_difference_type_t<TRange>>;

#if !KRYS_CONFIG(STD_LIBRARY_RANGES)
    namespace adl
    {
      using std::data;
      using std::empty;
      using std::size;

      using std::begin;
      using std::cbegin;
      using std::crbegin;
      using std::rbegin;

      using std::cend;
      using std::crend;
      using std::end;
      using std::rend;

      using std::iter_swap;
      using std::swap;

      template <typename TRange>
      concept has_begin = requires(TRange &&range) { begin(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_rbegin = requires(TRange &&range) { rbegin(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_cbegin = requires(TRange &&range) { cbegin(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_crbegin = requires(TRange &&range) { crbegin(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_end = requires(TRange &&range) { end(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_rend = requires(TRange &&range) { rend(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_cend = requires(TRange &&range) { cend(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_crend = requires(TRange &&range) { crend(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_member_begin = requires(TRange &&range) { std::forward<TRange>(range).begin(); };

      template <typename TRange>
      concept has_member_rbegin = requires(TRange &&range) { std::forward<TRange>(range).rbegin(); };

      template <typename TRange>
      concept has_member_cbegin = requires(TRange &&range) { std::forward<TRange>(range).cbegin(); };

      template <typename TRange>
      concept has_member_crbegin = requires(TRange &&range) { std::forward<TRange>(range).crbegin(); };

      template <typename TRange>
      concept has_member_end = requires(TRange &&range) { std::forward<TRange>(range).end(); };

      template <typename TRange>
      concept has_member_rend = requires(TRange &&range) { std::forward<TRange>(range).rend(); };

      template <typename TRange>
      concept has_member_cend = requires(TRange &&range) { std::forward<TRange>(range).cend(); };

      template <typename TRange>
      concept has_member_crend = requires(TRange &&range) { std::forward<TRange>(range).crend(); };

      template <typename TRange>
      constexpr bool begin_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_begin<TRange>)
        {
          return noexcept(begin(std::declval<TRange>()));
        }
        else if constexpr (has_member_begin<TRange>)
        {
          return noexcept(std::declval<TRange>().begin());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool begin_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_begin<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_begin<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TRange>
      constexpr bool cbegin_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_cbegin<TRange>)
        {
          return noexcept(cbegin(std::declval<TRange>()));
        }
        else if constexpr (has_member_cbegin<TRange>)
        {
          return noexcept(std::declval<TRange>().cbegin());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool cbegin_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_cbegin<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_cbegin<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TRange>
      constexpr bool end_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_end<TRange>)
        {
          return noexcept(end(std::declval<TRange>()));
        }
        else if constexpr (has_member_end<TRange>)
        {
          return noexcept(std::declval<TRange>().end());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool end_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_end<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_end<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TRange>
      constexpr bool cend_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_cend<TRange>)
        {
          return noexcept(cend(std::declval<TRange>()));
        }
        else if constexpr (has_member_cend<TRange>)
        {
          return noexcept(std::declval<TRange>().cend());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool cend_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_cend<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_cend<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TIterator>
      constexpr bool iter_move_noexcept() noexcept
      {
        if constexpr (LValueRef<decltype(*std::declval<TIterator>())>)
        {
          return noexcept(std::move(*std::declval<TIterator>()));
        }
        else
        {
          return noexcept(*std::declval<TIterator>());
        }
      }

      class iter_move_fn
      {
      public:
        template <typename TIterator>
        constexpr auto operator()(TIterator &&it) const noexcept(iter_move_noexcept<TIterator>())
          -> conditional_t<LValueRef<decltype(*std::forward<TIterator>(it))>,
                           decltype(std::move(*std::forward<TIterator>(it))),
                           decltype(*std::forward<TIterator>(it))>
        {
          if constexpr (LValueRef<decltype(*std::forward<TIterator>(it))>)
          {
            return std::move(*std::forward<TIterator>(it));
          }
          else
          {
            return *std::forward<TIterator>(it);
          }
        }
      };

      class begin_fn
      {
      public:
        template <typename TRange>
        requires(begin_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(begin_noexcept<TRange>())
        {
          using Range = remove_cvref_t<TRange>;
          if constexpr (IsArray<Range>)
          {
            return (range + 0);
          }
          else if constexpr (has_begin<TRange>)
          {
            return begin(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).begin();
          }
        }
      };

      class cbegin_fn
      {
      public:
        template <typename TRange>
        requires(cbegin_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(cbegin_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return (range + 0);
          }
          else if constexpr (has_cbegin<TRange>)
          {
            return cbegin(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).cbegin();
          }
        }
      };

      class end_fn
      {
      public:
        template <typename TRange>
        requires(end_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(end_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return (range + Extent<remove_cvref_t<TRange>>);
          }
          else if constexpr (has_end<TRange>)
          {
            return end(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).end();
          }
        }
      };

      class cend_fn
      {
      public:
        template <typename TRange>
        requires(cend_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(cend_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return (range + Extent<remove_cvref_t<TRange>>);
          }
          else if constexpr (has_cend<TRange>)
          {
            return cend(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).cend();
          }
        }
      };

      class data_fn
      {
      public:
        template <typename TRange>
        constexpr auto operator()(TRange &&range) const noexcept(noexcept(data(std::forward<TRange>(range))))
          -> decltype(data(std::forward<TRange>(range)))
        {
          return data(std::forward<TRange>(range));
        }
      };

      class size_fn
      {
      public:
        template <typename TRange>
        constexpr auto operator()(TRange &&range) const noexcept(noexcept(size(std::forward<TRange>(range))))
          -> decltype(size(std::forward<TRange>(range)))
        {
          return size(std::forward<TRange>(range));
        }
      };

      class empty_fn
      {
      public:
        template <typename TRange>
        constexpr auto operator()(TRange &&range) const noexcept(noexcept(empty(std::forward<TRange>(range))))
          -> decltype(empty(std::forward<TRange>(range)))
        {
          return empty(std::forward<TRange>(range));
        }
      };

      class iter_swap_fn
      {
      public:
        template <typename TLeftIterator, typename TRightIterator>
        constexpr auto operator()(TLeftIterator &&left, TRightIterator &&right) const
          noexcept(noexcept(iter_swap(std::forward<TLeftIterator>(left),
                                      std::forward<TRightIterator>(right))))
            -> decltype(iter_swap(std::forward<TLeftIterator>(left), std::forward<TRightIterator>(right)))
        {
          iter_swap(std::forward<TLeftIterator>(left), std::forward<TRightIterator>(right));
        }
      };

      class swap_fn
      {
      public:
        template <typename TLeft, typename TRight>
        constexpr auto operator()(TLeft &&left, TRight &&right) const
          noexcept(noexcept(swap(std::forward<TLeft>(left), std::forward<TRight>(right))))
            -> decltype(swap(std::forward<TLeft>(left), std::forward<TRight>(right)))
        {
          swap(std::forward<TLeft>(left), std::forward<TRight>(right));
        }
      };
    }
#endif

#if !KRYS_CONFIG(STD_LIBRARY_RANGES_REVERSE_CPOS)
    namespace adl
    {
      using std::crbegin;
      using std::crend;
      using std::rbegin;
      using std::rend;

      template <typename TRange>
      concept has_rbegin = requires(TRange &&range) { rbegin(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_crbegin = requires(TRange &&range) { crbegin(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_rend = requires(TRange &&range) { rend(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_crend = requires(TRange &&range) { crend(std::forward<TRange>(range)); };

      template <typename TRange>
      concept has_member_rbegin = requires(TRange &&range) { std::forward<TRange>(range).rbegin(); };

      template <typename TRange>
      concept has_member_crbegin = requires(TRange &&range) { std::forward<TRange>(range).crbegin(); };

      template <typename TRange>
      concept has_member_rend = requires(TRange &&range) { std::forward<TRange>(range).rend(); };

      template <typename TRange>
      concept has_member_crend = requires(TRange &&range) { std::forward<TRange>(range).crend(); };

      template <typename TRange>
      constexpr bool rbegin_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_rbegin<TRange>)
        {
          return noexcept(rbegin(std::declval<TRange>()));
        }
        else if constexpr (has_member_rbegin<TRange>)
        {
          return noexcept(std::declval<TRange>().rbegin());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool rbegin_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_rbegin<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_rbegin<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TRange>
      constexpr bool crbegin_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_crbegin<TRange>)
        {
          return noexcept(crbegin(std::declval<TRange>()));
        }
        else if constexpr (has_member_crbegin<TRange>)
        {
          return noexcept(std::declval<TRange>().crbegin());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool crbegin_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_crbegin<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_crbegin<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TRange>
      constexpr bool rend_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_rend<TRange>)
        {
          return noexcept(rend(std::declval<TRange>()));
        }
        else if constexpr (has_member_rend<TRange>)
        {
          return noexcept(std::declval<TRange>().rend());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool rend_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_rend<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_rend<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      template <typename TRange>
      constexpr bool crend_noexcept() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_crend<TRange>)
        {
          return noexcept(crend(std::declval<TRange>()));
        }
        else if constexpr (has_member_crend<TRange>)
        {
          return noexcept(std::declval<TRange>().crend());
        }
        else
        {
          return true;
        }
      }

      template <typename TRange>
      constexpr bool crend_sfinae() noexcept
      {
        if constexpr (IsArray<remove_cvref_t<TRange>>)
        {
          return true;
        }
        else if constexpr (has_crend<TRange>)
        {
          return true;
        }
        else if constexpr (has_member_crend<TRange>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      class rbegin_fn
      {
      public:
        template <typename TRange>
        requires(rbegin_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(rbegin_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return std::make_reverse_iterator(range + Extent<remove_cvref_t<TRange>>);
          }
          else if constexpr (has_rbegin<TRange>)
          {
            return rbegin(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).rbegin();
          }
        }
      };

      class crbegin_fn
      {
      public:
        template <typename TRange>
        requires(crbegin_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(crbegin_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return std::make_reverse_iterator(range + Extent<remove_cvref_t<TRange>>);
          }
          else if constexpr (has_crbegin<TRange>)
          {
            return crbegin(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).crbegin();
          }
        }
      };

      class rend_fn
      {
      public:
        template <typename TRange>
        requires(rend_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(rend_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return std::make_reverse_iterator(range + 0);
          }
          else if constexpr (has_rend<TRange>)
          {
            return rend(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).rend();
          }
        }
      };

      class crend_fn
      {
      public:
        template <typename TRange>
        requires(crend_sfinae<TRange>())
        constexpr decltype(auto) operator()(TRange &&range) const noexcept(crend_noexcept<TRange>())
        {
          if constexpr (IsArray<remove_cvref_t<TRange>>)
          {
            return std::make_reverse_iterator(range + 0);
          }
          else if constexpr (has_crend<TRange>)
          {
            return crend(std::forward<TRange>(range));
          }
          else
          {
            return std::forward<TRange>(range).crend();
          }
        }
      };
    }
#endif
  }

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  inline constexpr auto &begin = std::ranges::begin;
  inline constexpr auto &cbegin = std::ranges::cbegin;
  inline constexpr auto &end = std::ranges::end;
  inline constexpr auto &cend = std::ranges::cend;
  inline constexpr auto &size = std::ranges::size;
  inline constexpr auto &data = std::ranges::data;
  inline constexpr auto &empty = std::ranges::empty;
  inline constexpr auto &swap = std::ranges::swap;
  inline constexpr auto &iter_swap = std::ranges::iter_swap;
  inline constexpr auto &iter_move = std::ranges::iter_move;
#else
  inline constexpr Impl::adl::begin_fn begin {};
  inline constexpr Impl::adl::cbegin_fn cbegin {};
  inline constexpr Impl::adl::end_fn end {};
  inline constexpr Impl::adl::cend_fn cend {};
  inline constexpr Impl::adl::size_fn size {};
  inline constexpr Impl::adl::data_fn data {};
  inline constexpr Impl::adl::empty_fn empty {};
  inline constexpr Impl::adl::swap_fn swap {};
  inline constexpr Impl::adl::iter_swap_fn iter_swap {};
  inline constexpr Impl::adl::iter_move_fn iter_move {};
#endif

#if KRYS_CONFIG(STD_LIBRARY_RANGES_REVERSE_CPOS)
  // Not present in C++20: need to version check these
  inline constexpr auto &rbegin = std::ranges::rbegin;
  inline constexpr auto &crbegin = std::ranges::crbegin;
  inline constexpr auto &crend = std::ranges::crend;
  inline constexpr auto &rend = std::ranges::rend;
#else
  inline constexpr Impl::adl::rbegin_fn rbegin {};
  inline constexpr Impl::adl::crbegin_fn crbegin {};
  inline constexpr Impl::adl::rend_fn rend {};
  inline constexpr Impl::adl::crend_fn crend {};
#endif

  template <typename TRange>
  concept has_adl_size = requires { Krys::Ranges::size(std::declval<add_lvalue_ref_t<TRange>>()); };

  template <typename TRange>
  concept has_adl_empty = requires { Krys::Ranges::empty(std::declval<add_lvalue_ref_t<TRange>>()); };

  template <typename TRange>
  concept has_adl_begin = requires { Krys::Ranges::begin(std::declval<TRange>()); };

  template <typename TRange>
  concept has_adl_end = requires { Krys::Ranges::end(std::declval<TRange>()); };

  template <typename T>
  concept IsRange = has_adl_begin<T> && has_adl_end<T>;

  template <typename TIterator>
  using iterator_rvalue_reference_t = decltype(Krys::Ranges::iter_move(std::declval<TIterator &>()));

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  template <typename TRange>
  using range_iterator_t = std::ranges::iterator_t<TRange>;

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
  using iterator_category_t = Impl::iterator_category_or_fallback_cascade_t<TIterator>;

  template <typename TIterator>
  using iterator_concept_t = Impl::iterator_concept_or_fallback_t<TIterator>;
#else
  template <typename TRange>
  using range_iterator_t =
    remove_ref_t<decltype(Krys::Ranges::begin(std::declval<add_lvalue_ref_t<TRange>>()))>;

  template <typename TRange>
  using range_sentinel_t =
    remove_ref_t<decltype(Krys::Ranges::end(std::declval<add_lvalue_ref_t<TRange>>()))>;

  template <typename TRange>
  using range_value_type_t = iterator_value_type_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_reference_t = iterator_reference_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_rvalue_reference_t = iterator_rvalue_reference_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_difference_type_t = iterator_difference_type_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_size_type_t = iterator_size_type_t<range_iterator_t<TRange>>;

  template <typename TIterator>
  using iterator_category_t =
    typename Impl::IteratorCategoryOrConceptOrFallback<remove_ref_t<TIterator>>::type;

  template <typename TIterator>
  using iterator_concept_t = Impl::iterator_concept_or_fallback_t<TIterator>;
#endif

  template <typename TRange>
  using range_element_type_t = iterator_element_type_t<range_iterator_t<TRange>>;

  template <typename TRange>
  using range_const_iterator_t =
    remove_cvref_t<decltype(Krys::Ranges::cbegin(std::declval<add_lvalue_ref_t<TRange>>()))>;

  template <typename TRange>
  using range_const_sentinel_t =
    remove_cvref_t<decltype(Krys::Ranges::cend(std::declval<add_lvalue_ref_t<TRange>>()))>;

  template <typename TTag, typename TActualTag>
  concept IsConceptOrBetter = DerivedFrom<TTag, TActualTag>;

  template <typename TTag, typename TIterator>
  concept IsIteratorCategoryOrBetter = IsConceptOrBetter<TTag, iterator_category_t<TIterator>>;

  template <typename TTag, typename TIterator>
  concept IsIteratorConceptOrBetter = IsConceptOrBetter<TTag, iterator_concept_t<TIterator>>;

  namespace Impl
  {
    template <typename T, typename... TArgs>
    concept has_iter_advance = requires(T &&t, TArgs &&...args) {
      { t.advance(std::forward<TArgs>(args)...) };
    };

    template <typename T, typename... TArgs>
    concept has_iter_recede = requires(T &&t, TArgs &&...args) {
      { t.recede(std::forward<TArgs>(args)...) };
    };

    template <typename TIterator, typename... TArgs>
    constexpr bool advance_noexcept() noexcept
    {
      if constexpr (has_iter_advance<TIterator, TArgs...>)
      {
        return noexcept(std::declval<TIterator>().advance(std::declval<TArgs>()...));
      }
      else
      {
        return noexcept(++std::declval<add_lvalue_ref_t<remove_ref_t<TIterator>>>());
      }
    }

    template <typename TIterator, typename... TArgs>
    constexpr bool recede_noexcept() noexcept
    {
      if constexpr (has_iter_recede<TIterator, TArgs...>)
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
#if !KRYS_CONFIG(STD_LIBRARY_RANGES)
      class ssize_fn
      {
      public:
        template <typename TRange>
        constexpr auto operator()(TRange &&range) const noexcept(noexcept(size(std::forward<TRange>(range))))
          -> decltype(static_cast<Impl::ssize_diff_type<TRange>>(size(std::forward<TRange>(range))))
        {
          return static_cast<Impl::ssize_diff_type<TRange>>(size(std::forward<TRange>(range)));
        }
      };

      class cdata_fn
      {
      private:
        template <typename TRange>
        using ConstRange = conditional_t<LValueRef<TRange>, const remove_ref_t<TRange> &, const TRange>;

      public:
        template <typename TRange>
        constexpr auto operator()(TRange &&range) const
          noexcept(noexcept(Krys::Ranges::data(static_cast<ConstRange<TRange> &&>(range))))
            -> remove_ref_t<range_reference_t<ConstRange<TRange>>> *
        {
          return Krys::Ranges::data(static_cast<ConstRange<TRange> &&>(range));
        }
      };
#endif
      class iter_advance_fn
      {
      public:
        template <typename TIterator>
        constexpr decltype(auto) operator()(TIterator &&it) const noexcept(advance_noexcept<TIterator>())
        {
          if constexpr (Impl::has_iter_advance<TIterator>)
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
          noexcept(advance_noexcept<TIterator, TDiff>())
        {
          if constexpr (Impl::has_iter_advance<TIterator, TDiff>)
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
        constexpr decltype(auto) operator()(TIterator &&it) const noexcept(recede_noexcept<TIterator>())
        {
          if constexpr (has_iter_recede<TIterator>)
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
          noexcept(recede_noexcept<TIterator, TDiff>())
        {
          if constexpr (has_iter_recede<TIterator, TDiff>)
          {
            std::forward<TIterator>(it).recede(diff);
          }
          else
          {
            if constexpr (IsIteratorConceptOrBetter<std::random_access_iterator_tag,
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

    class ViewBase
    {
    };
  }

  using ViewBase
#if KRYS_CONFIG(STD_LIBRARY_RANGES)
    = std::ranges::view_base;
#else
    = Impl::ViewBase;
#endif

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  inline constexpr auto &ssize = std::ranges::ssize;
  inline constexpr auto &cdata = std::ranges::cdata;
#else
  inline constexpr Impl::adl::cdata_fn cdata {};
  inline constexpr Impl::adl::ssize_fn ssize {};
#endif
  inline constexpr Impl::adl::iter_advance_fn iter_advance {};
  inline constexpr Impl::adl::iter_recede_fn iter_recede {};

  /// @brief A trait specialized by downstream classes to determine whether or not the type is a view.
  /// @tparam TRange The range type that may or may not be a view.
  template <typename TRange>
  inline constexpr bool EnableView
#if KRYS_CONFIG(STD_LIBRARY_RANGES)
    = std::ranges::enable_view<TRange>;
#else
    = DerivedFrom<ViewBase, TRange>;
#endif

  /// @brief A trait specialized by downstream classes to determine whether or not the type is a borrowed
  /// range.
  /// @tparam TRange The range type that may or may not be a borrowed range.
  template <typename TRange>
  inline constexpr bool EnableBorrowedRange
#if KRYS_CONFIG(STD_LIBRARY_RANGES)
    = std::ranges::enable_borrowed_range<TRange>;
#else
    = false;
#endif

  /// @brief Checks whether or not the provided type is a view. This means that `EnableView` has been
  /// turned on, and it meets a few other criteria.
  /// @tparam T The type to check if it is a view or not.
  template <typename T>
  concept IsView =
    EnableView<T> && IsRange<T>
    && ((MoveConstructible<T> && MoveAssignable<T>) || (IsRange<T> && !Const<T> && LValueRef<T>));

  /// @brief Whether or not a given type is a borrowed range or not. Used as a proxy over the standard's
  /// borrowed_range, if it exists.
  /// @tparam TRange The range type that may or may not be a borrowed range.
  /// @remarks This is placed in the low-level library because it has to be used in multiple places,
  /// including the std::span shim if necessary.
  template <typename TRange>
  concept BorrowedRange =
#if KRYS_CONFIG(STD_LIBRARY_RANGES)
    std::ranges::borrowed_range<TRange>;
#else
    IsRange<TRange> // must have begin/end, at least!
    && (LValueRef<TRange> || EnableBorrowedRange<remove_cvref_t<TRange>>);
#endif
}