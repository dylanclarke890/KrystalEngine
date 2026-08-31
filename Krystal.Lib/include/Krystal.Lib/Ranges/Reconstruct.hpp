#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/TypeTraits.hpp"
#include "Krystal.Lib/String/EmptyString.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Lib/Utils/Hijack.hpp"
#include "Krystal.Lib/Utils/Tag.hpp"
#include "Krystal.Lib/Utils/Unwrap.hpp"
#include <string_view>
#include <utility>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename T, typename TIterator, typename TSentinel>
    constexpr bool IsSpanReconstructible() noexcept
    {
      using TIteratorElement = remove_ref_t<std::iter_reference_t<TIterator>>;
      constexpr bool check = NonDerivedCompatiblePointer<TIteratorElement, T> // cf
                             && std::contiguous_iterator<TIterator>           // cf
                             && ((IsConst<TIteratorElement>) ? IsConst<T> : true);
      if constexpr (check)
      {
        if constexpr (std::sized_sentinel_for<TIterator, TSentinel>)
        {
          return true;
        }
        else
        {
          return ConvertibleTo<TSentinel, std::size_t>;
        }
      }
      else
      {
        return false;
      }
    }

    template <typename T, typename TIterator, typename TSentinel>
    constexpr bool IsStringViewReconstructible() noexcept
    {
      if constexpr (!std::contiguous_iterator<TIterator>)
      {
        return false;
      }
      else
      {
        if constexpr (std::sized_sentinel_for<TIterator, TSentinel>)
        {
          return true;
        }
        else
        {
          return ConvertibleTo<TSentinel, std::size_t>;
        }
      }
    }

    template <typename T, std::size_t TExtent, typename TIterator, typename TSentinel,
              enable_if_t<Impl::IsSpanReconstructible<T, TIterator, TSentinel>()> * = nullptr>
    constexpr Span<T> reconstruct(std::in_place_type_t<Span<T, TExtent>>, TIterator iterator,
                                  TSentinel sentinel) noexcept
    {
      if constexpr (std::sized_sentinel_for<TIterator, TSentinel>)
      {
        if constexpr (std::contiguous_iterator<TSentinel>)
        {
          auto iteratorAddress = std::to_address(iterator);
          auto sentinelAddress = std::to_address(sentinel);
          return Span<T>(iteratorAddress, static_cast<std::size_t>(sentinelAddress - iteratorAddress));
        }
        else
        {
          auto iteratorAddress = std::to_address(iterator);
          return Span<T>(iteratorAddress, static_cast<std::size_t>(sentinel - iterator));
        }
      }
      else
      {
        auto iteratorAddress = std::to_address(iterator);
        return Span<T>(iteratorAddress, sentinel);
      }
    }

    template <typename T, typename TTraits, typename TIterator, typename TSentinel,
              enable_if_t<Impl::IsStringViewReconstructible<T, TIterator, TSentinel>()> * = nullptr>
    constexpr std::basic_string_view<T, TTraits>
      reconstruct(std::in_place_type_t<std::basic_string_view<T, TTraits>>, TIterator iterator,
                  TSentinel sentinel) noexcept
    {
      using TSize = typename std::basic_string_view<T, TTraits>::size_type;
      if constexpr (std::sized_sentinel_for<TIterator, TSentinel>)
      {
        TSize ptrSize = static_cast<TSize>(sentinel - iterator);
        if (ptrSize == static_cast<TSize>(0))
        {
          const auto &emptyStr = EmptyString<T>();
          return std::basic_string_view<T, TTraits>(emptyStr + 0, 0);
        }
        return std::basic_string_view<T, TTraits>(std::to_address(iterator), ptrSize);
      }
      else if (static_cast<TSize>(sentinel) == static_cast<TSize>(0))
      {
        const auto &emptyStr = EmptyString<T>();
        return std::basic_string_view<T, TTraits>(emptyStr + 0, 0);
      }
      else
      {
        return std::basic_string_view<T, TTraits>(std::to_address(iterator), static_cast<TSize>(sentinel));
      }
    }

    template <bool>
    class ReconstructFn;

    template <bool, typename...>
    class CascadingReconstructFn;

    template <typename TIterator, typename TSentinel>
    concept has_tagless_iterator_sentinel_reconstruct =
      requires { reconstruct(std::declval<TIterator>(), std::declval<TSentinel>()); };

    template <typename TTag, typename TRange>
    concept has_range_reconstruct = has_tagless_iterator_sentinel_reconstruct<TTag, TRange>;

    template <typename TTag, typename TIterator, typename TSentinel>
    concept has_iterator_sentinel_reconstruct =
      requires { reconstruct(std::declval<TTag>(), std::declval<TIterator>(), std::declval<TSentinel>()); };

    template <typename TTag, typename TRange, typename TIterator, typename TSentinel>
    concept has_range_iterator_reconstruct = requires {
      reconstruct(std::declval<TTag>(), std::declval<TRange>(), std::declval<TIterator>(),
                  std::declval<TSentinel>());
    };

    template <typename TIterator, typename TSentinel>
    constexpr bool IsTaglessIteratorReconstructNoexcept() noexcept
    {
      if constexpr (((Class<TIterator> || IsEnum<TIterator>) || (Class<TSentinel> || IsEnum<TSentinel>)))
      {
        if constexpr (has_tagless_iterator_sentinel_reconstruct<TIterator, TSentinel>)
        {
          return noexcept(reconstruct(std::declval<TIterator>(), std::declval<TSentinel>()));
        }
        else
        {
          return NoThrowConstructible<
            std::ranges::subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>, TIterator,
            TSentinel>;
        }
      }
      else
      {
        return NoThrowConstructible<
          std::ranges::subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>, TIterator, TSentinel>;
      }
    }

    template <typename TInPlace, typename TIterator, typename TSentinel>
    constexpr bool IsIteratorReconstructNoexcept() noexcept
    {
      if constexpr (has_iterator_sentinel_reconstruct<TInPlace, TIterator, TSentinel>)
      {
        return noexcept(
          reconstruct(std::declval<TInPlace>(), std::declval<TIterator>(), std::declval<TSentinel>()));
      }
      else
      {
        return IsTaglessIteratorReconstructNoexcept<TIterator, TSentinel>();
      }
    }

    template <typename TInPlace, typename TRange, typename TIterator, typename TSentinel>
    constexpr bool IsRangeIteratorReconstructNoexcept() noexcept
    {
      if constexpr (has_range_iterator_reconstruct<TInPlace, TRange, TIterator, TSentinel>)
      {
        return noexcept(reconstruct(std::declval<TInPlace>(), std::declval<TRange>(),
                                    std::declval<TIterator>(), std::declval<TSentinel>()));
      }
      else
      {
        return IsIteratorReconstructNoexcept<TInPlace, TIterator, TSentinel>();
      }
    }

    template <bool IsMutable, typename TInPlace, typename TRange>
    constexpr bool IsRangeReconstructNoexcept() noexcept
    {
      if constexpr (IsMutable)
      {
        return IsRangeIteratorReconstructNoexcept<TInPlace, TRange, std::ranges::iterator_t<TRange>,
                                                  std::ranges::sentinel_t<TRange>>();
      }
      else
      {
        return IsRangeIteratorReconstructNoexcept<TInPlace, TRange, std::ranges::const_iterator_t<TRange>,
                                                  std::ranges::const_sentinel_t<TRange>>();
      }
    }

    template <bool IsMutable, typename TInPlaceOrIterator, typename TRangeOrSentinel>
    constexpr bool IsRangeReconstructOrTaglessIteratorReconstructNoexcept() noexcept
    {
      if constexpr (Krys::IsSpecializationOf<remove_cvref_t<TInPlaceOrIterator>, std::in_place_type_t>)
      {
        return IsRangeReconstructNoexcept<IsMutable, TInPlaceOrIterator, TRangeOrSentinel>();
      }
      else
      {
        return IsTaglessIteratorReconstructNoexcept<TInPlaceOrIterator, TRangeOrSentinel>();
      }
    }

    template <bool IsMutable, typename TInPlaceOrIterator, typename TRangeOrSentinel, typename... TArgs>
    constexpr static bool IsReconstructible() noexcept
    {
      if constexpr (sizeof...(TArgs) == 0)
      {
        if constexpr (Krys::IsSpecializationOf<remove_cvref_t<TInPlaceOrIterator>, std::in_place_type_t>)
        {
          if constexpr (has_range_reconstruct<TInPlaceOrIterator, TRangeOrSentinel>)
          {
            return true;
          }
          else
          {
            if constexpr (IsMutable)
            {
              return IsReconstructible<IsMutable, TInPlaceOrIterator, TRangeOrSentinel,
                                       std::ranges::iterator_t<TRangeOrSentinel>,
                                       std::ranges::sentinel_t<TRangeOrSentinel>>();
            }
            else
            {
              return IsReconstructible<IsMutable, TInPlaceOrIterator, TRangeOrSentinel,
                                       std::ranges::const_iterator_t<TRangeOrSentinel>,
                                       std::ranges::const_sentinel_t<TRangeOrSentinel>>();
            }
          }
        }
        else if constexpr (((Class<TInPlaceOrIterator> || IsEnum<TInPlaceOrIterator>)
                            || (Class<TRangeOrSentinel> || IsEnum<TRangeOrSentinel>))
                           && has_tagless_iterator_sentinel_reconstruct<TInPlaceOrIterator, TRangeOrSentinel>)
        {
          return true;
        }
        else
        {
          return false;
        }
      }
      else if constexpr (sizeof...(TArgs) == 1)
      {
        if constexpr (has_range_reconstruct<remove_cvref_t<TInPlaceOrIterator>, TRangeOrSentinel>)
        {
          return true;
        }
        else
        {
          return IsReconstructible<IsMutable, TRangeOrSentinel, TArgs...>();
        }
      }
      else if constexpr (sizeof...(TArgs) == 2)
      {
        if constexpr (has_range_iterator_reconstruct<TInPlaceOrIterator, TRangeOrSentinel, TArgs...>)
        {
          return true;
        }
        else
        {
          return IsReconstructible<IsMutable, TInPlaceOrIterator, TArgs...>();
        }
      }
      else
      {
        static_assert(DependentFalse<TInPlaceOrIterator>, "improper arguments to IsReconstructible");
      }
    }

    template <bool IsMutable, typename TInPlaceOrIterator, typename TRangeOrSentinel, typename... TArgs>
    inline constexpr bool IsReconstructibleNoexcept() noexcept
    {
      if constexpr (sizeof...(TArgs) == 0)
      {
        if constexpr (IsSpecializationOf<remove_cvref_t<TInPlaceOrIterator>, std::in_place_type_t>)
        {
          return IsRangeReconstructNoexcept<IsMutable, TInPlaceOrIterator, TRangeOrSentinel>();
        }
        else
        {
          return IsTaglessIteratorReconstructNoexcept<TInPlaceOrIterator, TRangeOrSentinel>();
        }
      }
      else if constexpr (sizeof...(TArgs) == 1)
      {
        return IsIteratorReconstructNoexcept<TInPlaceOrIterator, TRangeOrSentinel, TArgs...>();
      }
      else if constexpr (sizeof...(TArgs) == 2)
      {
        return IsRangeIteratorReconstructNoexcept<TInPlaceOrIterator, TRangeOrSentinel, TArgs...>();
      }
      else
      {
        static_assert(DependentFalse<TInPlaceOrIterator>, "improper arguments to IsReconstructibleNoexcept");
      }
    }

    template <bool IsMutable>
    class ReconstructFn : public Hijack::Token<ReconstructFn<IsMutable>>,
                          public ::KrysHijackGlobalToken<ReconstructFn<IsMutable>>
    {
    public:
      template <typename TInPlaceTag, typename TIterator, typename TSentinel>
      constexpr auto operator()(KRYS_MAYBE_UNUSED std::in_place_type_t<TInPlaceTag> inplace,
                                TIterator &&iterator, TSentinel &&sentinel) const
        noexcept(IsIteratorReconstructNoexcept<std::in_place_type_t<TInPlaceTag>, TIterator, TSentinel>())
      {
        if constexpr (has_iterator_sentinel_reconstruct<std::in_place_type_t<TInPlaceTag>,
                                                        remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>)
        {
          return reconstruct(inplace, std::forward<TIterator>(iterator), std::forward<TSentinel>(sentinel));
        }
        else
        {
          return (*this)(std::forward<TIterator>(iterator), std::forward<TSentinel>(sentinel));
        }
      }

      template <typename TInPlaceTag, typename TRange, typename TIterator, typename TSentinel>
      constexpr decltype(auto) operator()(std::in_place_type_t<TInPlaceTag> inplace, TRange &&range,
                                          TIterator &&it, TSentinel &&sen) const
        noexcept(IsRangeIteratorReconstructNoexcept<std::in_place_type_t<TInPlaceTag>, TRange, TIterator,
                                                    TSentinel>())
      {
        if constexpr (has_range_iterator_reconstruct<std::in_place_type_t<TInPlaceTag>, TRange, TIterator,
                                                     TSentinel>)
        {
          return reconstruct(inplace, std::forward<TRange>(range), std::forward<TIterator>(it),
                             std::forward<TSentinel>(sen));
        }
        else
        {
          return (*this)(inplace, std::forward<TIterator>(it), std::forward<TSentinel>(sen));
        }
      }

      template <typename TInPlaceOrIterator, typename TRangeOrSentinel>
      constexpr decltype(auto) operator()(TInPlaceOrIterator &&inplaceOrIterator,
                                          TRangeOrSentinel &&rangeOrSentinel) const
        noexcept(IsRangeReconstructOrTaglessIteratorReconstructNoexcept<IsMutable, TInPlaceOrIterator,
                                                                        TRangeOrSentinel>())
      {
        if constexpr (IsSpecializationOf<remove_cvref_t<TInPlaceOrIterator>, std::in_place_type_t>)
        {
          if constexpr (has_range_reconstruct<TInPlaceOrIterator, TRangeOrSentinel>)
          {
            return reconstruct(std::forward<TInPlaceOrIterator>(inplaceOrIterator),
                               std::forward<TRangeOrSentinel>(rangeOrSentinel));
          }
          else
          {
            if constexpr (IsMutable)
            {
              return (*this)(std::forward<TInPlaceOrIterator>(inplaceOrIterator),
                             std::forward<TRangeOrSentinel>(rangeOrSentinel),
                             std::ranges::begin(rangeOrSentinel), std::ranges::end(rangeOrSentinel));
            }
            else
            {
              return (*this)(std::forward<TInPlaceOrIterator>(inplaceOrIterator),
                             std::forward<TRangeOrSentinel>(rangeOrSentinel),
                             std::ranges::cbegin(rangeOrSentinel), std::ranges::cend(rangeOrSentinel));
            }
          }
        }
        else if constexpr (((Class<TInPlaceOrIterator> || IsEnum<TInPlaceOrIterator>)
                            || (Class<TRangeOrSentinel> || IsEnum<TRangeOrSentinel>))
                           && has_tagless_iterator_sentinel_reconstruct<TInPlaceOrIterator, TRangeOrSentinel>)
        {
          return reconstruct(std::forward<TInPlaceOrIterator>(inplaceOrIterator),
                             std::forward<TRangeOrSentinel>(rangeOrSentinel));
        }
        else
        {
          return std::ranges::subrange<remove_cvref_t<TInPlaceOrIterator>, remove_cvref_t<TRangeOrSentinel>>(
            std::forward<TInPlaceOrIterator>(inplaceOrIterator),
            std::forward<TRangeOrSentinel>(rangeOrSentinel));
        }
      }
    };
  }

  inline namespace fn
  {
    /// @brief An implementation of the P1664 (https://wg21.link/p1664 |
    /// https://thephd.dev/_vendor/future_cxx/papers/d1664.html) reconstructible ranges extension point. It
    /// is derived from Krys::Hijack::token in order for outside implementations to have a common place to
    /// put reconstruction for things outside of their control, without needing to place it in the global
    /// namespace or the immediate ztd::ranges namespace, where there are too many other types that could
    /// force asking more questions about what is in the list for ADL and drive up compile-times.
    inline constexpr Impl::ReconstructFn<true> reconstruct = {};

    /// @brief An implementation of the P1664 (https://wg21.link/p1664 |
    /// https://thephd.dev/_vendor/future_cxx/papers/d1664.html) reconstructible ranges extension point. It
    /// is derives from Krys::Hijack::token in order for outside implementations to have a common place to
    /// put reconstruction for things outside of their control, without needing to place it in the global
    /// namespace or the immediate ztd::ranges namespace, where there are too many other types that could
    /// force asking more questions about what is in the list for ADL and drive up compile-times.
    inline constexpr Impl::ReconstructFn<false> const_reconstruct = {};
  }

  namespace Impl
  {
    template <bool IsMutable, typename... TArgs, typename Tpe>
    constexpr static bool IsCascadingReconstructible(Tag<Tpe>) noexcept
    {
      return IsReconstructible<IsMutable, std::in_place_type_t<Tpe>, TArgs...>();
    }

    template <bool IsMutable, typename... TArgs, typename Tpe, typename... Tpes>
    constexpr static bool IsCascadingReconstructible(Tag<Tpe, Tpes...>) noexcept
    {
      if constexpr (IsReconstructible<IsMutable, std::in_place_type_t<Tpe>, TArgs...>())
      {
        return true;
      }
      else
      {
        return IsCascadingReconstructible<IsMutable, TArgs...>(Tag<Tpes...>());
      }
    }

    template <bool IsMutable, typename... TArgs, typename... Tpes>
    inline constexpr bool IsCascadingReconstructibleNoexcept(Tag<Tpes...>) noexcept
    {
      return ((IsReconstructibleNoexcept<IsMutable, std::in_place_type_t<Tpes>, TArgs...>()) || ...);
    }

    template <bool IsMutable, typename... TArgs, typename Tpe>
    constexpr decltype(auto) CascadingReconstruct(Tag<Tpe>, TArgs &&...args) noexcept(
      IsReconstructibleNoexcept<IsMutable, std::in_place_type_t<Tpe>, TArgs...>())
    {
      if constexpr (IsMutable)
      {
        return reconstruct(std::in_place_type<Tpe>, std::forward<TArgs>(args)...);
      }
      else
      {
        return const_reconstruct(std::in_place_type<Tpe>, std::forward<TArgs>(args)...);
      }
    }

    template <bool IsMutable, typename... TArgs, typename Tpe, typename... Tpes>
    requires(sizeof...(Tpes) > 0)
    constexpr decltype(auto) CascadingReconstruct(Tag<Tpe, Tpes...>, TArgs &&...args) noexcept(
      IsCascadingReconstructibleNoexcept<IsMutable, TArgs...>(Tag<Tpe, Tpes...>()))
    {
      if constexpr (IsReconstructible<IsMutable, std::in_place_type_t<Tpe>, TArgs...>())
      {
        if constexpr (IsMutable)
        {
          return reconstruct(std::in_place_type<Tpe>, std::forward<TArgs>(args)...);
        }
        else
        {
          return const_reconstruct(std::in_place_type<Tpe>, std::forward<TArgs>(args)...);
        }
      }
      else
      {
        return CascadingReconstruct<IsMutable>(Tag<Tpes...>(), std::forward<TArgs>(args)...);
      }
    }

    template <bool IsMutable, typename... Tpes>
    class CascadingReconstructFn
    {
    public:
      template <typename... TArgs>
      constexpr decltype(auto) operator()(TArgs &&...args) const
        noexcept(IsCascadingReconstructibleNoexcept<IsMutable, TArgs...>(Tag<Tpes...>()))
      {
        return CascadingReconstruct<IsMutable>(Tag<Tpes...>(), std::forward<TArgs>(args)...);
      }
    };
  }

  /// @brief A reconstruct that attempts multiple versions of reconstruct, and if none of them works then
  /// fallsback to the default subrange return value.
  template <typename... TArgs>
  inline constexpr Impl::CascadingReconstructFn<true, TArgs...> cascading_reconstruct = {};

  /// @brief A reconstruct that attempts multiple versions of reconstruct, and if none of them works then
  /// fallsback to the default subrange return value.
  template <typename... TArgs>
  inline constexpr Impl::CascadingReconstructFn<false, TArgs...> const_cascading_reconstruct = {};

  template <typename... TArgs>
  concept Reconstructible = Impl::IsReconstructible<true, TArgs...>();

  template <typename... TArgs>
  concept NoThrowReconstructible = Impl::IsReconstructibleNoexcept<true, TArgs...>();

  template <typename... TArgs>
  concept ConstReconstructible = Impl::IsReconstructible<false, TArgs...>();

  template <typename... TArgs>
  concept NoThrowConstReconstructible = Impl::IsReconstructibleNoexcept<false, TArgs...>();

  template <typename TIterator, typename TSentinel>
  concept TaglessIteratorReconstructible =
    ((Class<TIterator> || IsEnum<TIterator>) || (Class<TSentinel> || IsEnum<TSentinel>))
    && Reconstructible<TIterator, TSentinel>;

  template <typename TTag, typename TIterator, typename TSentinel>
  concept IteratorReconstructible = Reconstructible<TTag, TIterator, TSentinel>;

  template <typename TTag, typename TRange, typename TIterator, typename TSentinel>
  concept RangeIterableReconstructible = Reconstructible<TTag, TRange, TIterator, TSentinel>;

  template <typename TTag, typename TRange>
  concept RangeReconstructible = Reconstructible<std::in_place_type_t<unwrap_remove_cvref_t<TTag>>, TRange>;

  template <typename TIterator, typename TSentinel>
  concept NoThrowTaglessIteratorReconstructible =
    ((Class<TIterator> || IsEnum<TIterator>) || (Class<TSentinel> || IsEnum<TSentinel>))
    && TaglessIteratorReconstructible<TIterator, TSentinel> && NoThrowReconstructible<TIterator, TSentinel>;

  template <typename TTag, typename TIterator, typename TSentinel>
  concept NoThrowIteratorReconstructible =
    IteratorReconstructible<TTag, TIterator, TSentinel> && NoThrowReconstructible<TTag, TIterator, TSentinel>;

  template <typename TTag, typename TRange, typename TIterator, typename TSentinel>
  concept NoThrowRangeIteratorReconstructible =
    RangeIterableReconstructible<TTag, TRange, TIterator, TSentinel>
    && NoThrowReconstructible<TTag, TRange, TIterator, TSentinel>;

  template <typename TTag, typename TRange>
  concept NoThrowRangeReconstructible =
    RangeReconstructible<TTag, TRange>
    && NoThrowReconstructible<std::in_place_type_t<unwrap_remove_cvref_t<TTag>>, TRange>;

  template <typename TRange, typename TIterator = std::ranges::iterator_t<unwrap_remove_ref_t<TRange>>,
            typename TSentinel = std::ranges::sentinel_t<unwrap_remove_ref_t<TRange>>>
  using reconstruct_t = decltype(Krys::Ranges::reconstruct(
    std::in_place_type<unwrap_remove_cvref_t<TRange>>, std::declval<TIterator>(), std::declval<TSentinel>()));

  template <typename TRange>
  using range_reconstruct_t = decltype(Krys::Ranges::reconstruct(
    std::in_place_type<unwrap_remove_cvref_t<TRange>>, std::declval<TRange>()));

  template <typename TTag, typename TRange = TTag>
  using tag_range_reconstruct_t = decltype(Krys::Ranges::reconstruct(
    std::in_place_type<unwrap_remove_cvref_t<TTag>>, std::declval<TRange>()));

  template <typename TIterator, typename TSentinel>
  concept TaglessIteratorConstReconstructible =
    ((Class<TIterator> || IsEnum<TIterator>) || (Class<TSentinel> || IsEnum<TSentinel>))
    && ConstReconstructible<TIterator, TSentinel>;

  template <typename TTag, typename TIterator, typename TSentinel>
  concept IteratorConstReconstructible = ConstReconstructible<TTag, TIterator, TSentinel>;

  template <typename TTag, typename TRange, typename TIterator, typename TSentinel>
  concept RangeIteratorConstReconstructible = ConstReconstructible<TTag, TRange, TIterator, TSentinel>;

  template <typename TTag, typename TRange>
  concept RangeConstReconstructible =
    ConstReconstructible<std::in_place_type_t<unwrap_remove_cvref_t<TTag>>, TRange>;

  template <typename TIterator, typename TSentinel>
  concept NoThrowTaglessIteratorConstReconstructible =
    ((Class<TIterator> || IsEnum<TIterator>) || (Class<TSentinel> || IsEnum<TSentinel>))
    && TaglessIteratorConstReconstructible<TIterator, TSentinel>
    && NoThrowConstReconstructible<TIterator, TSentinel>;

  template <typename TTag, typename TIterator, typename TSentinel>
  concept NoThrowIteratorConstReconstructible = IteratorConstReconstructible<TTag, TIterator, TSentinel>
                                                && NoThrowConstReconstructible<TTag, TIterator, TSentinel>;

  template <typename TTag, typename TRange, typename TIterator, typename TSentinel>
  concept NoThrowRangeIteratorConstReconstructible =
    RangeIteratorConstReconstructible<TTag, TRange, TIterator, TSentinel>
    && NoThrowConstReconstructible<TTag, TRange, TIterator, TSentinel>;

  template <typename TTag, typename TRange>
  concept NoThrowRangeConstReconstructible =
    RangeConstReconstructible<TTag, TRange>
    && NoThrowConstReconstructible<std::in_place_type_t<unwrap_remove_cvref_t<TTag>>, TRange>;

  template <typename TRange, typename TIterator = std::ranges::iterator_t<unwrap_remove_ref_t<TRange>>,
            typename TSentinel = std::ranges::sentinel_t<unwrap_remove_ref_t<TRange>>>
  using const_reconstruct_t = decltype(Krys::Ranges::const_reconstruct(
    std::in_place_type<unwrap_remove_cvref_t<TRange>>, std::declval<TIterator>(), std::declval<TSentinel>()));

  template <typename TRange>
  using const_range_reconstruct_t = decltype(Krys::Ranges::const_reconstruct(
    std::in_place_type<unwrap_remove_cvref_t<TRange>>, std::declval<TRange>()));

  template <typename TTag, typename TRange = TTag>
  using tag_range_const_reconstruct_t = decltype(Krys::Ranges::const_reconstruct(
    std::in_place_type<unwrap_remove_cvref_t<TTag>>, std::declval<TRange>()));
}
