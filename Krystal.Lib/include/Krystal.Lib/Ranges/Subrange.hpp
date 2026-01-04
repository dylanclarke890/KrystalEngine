#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include "Krystal.Lib/Ranges/Range.hpp"

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  #include <ranges>
#endif
#include <iterator>
#include <utility>

namespace Krys::Ranges
{
  namespace Impl
  {
    struct SizeMark
    {
    };

    /// @brief An enumeration that helps determine whether a subrange has size information or not.
    enum class SubrangeKind : bool
    {
      /// @brief Does not have a size (or does not have a size that can be computed in O(1)).
      unsized,

      /// @brief Has a size that can be computed in O(1).
      sized
    };

    template <typename TSize, bool TStore>
    struct SubrangeSize
    {
      constexpr SubrangeSize() noexcept
      {
      }
    };

    template <typename TSize>
    struct SubrangeSize<TSize, true>
    {
      TSize _size;

      constexpr SubrangeSize() noexcept : _size(static_cast<TSize>(0))
      {
      }
      constexpr SubrangeSize(TSize &&size) noexcept(NoThrowMoveConstructible<TSize>) : _size(std::move(size))
      {
      }
      constexpr SubrangeSize(const TSize &size) noexcept(NoThrowCopyConstructible<TSize>) : _size(size)
      {
      }
    };

    /// @brief A utility class to aid in trafficking iterator pairs (or, possibly, and iterator and sentinel
    /// pair) through the API to provide a generic, basic "range" type. Attempts to mimic
    /// `std::ranges::subrange` on platforms where it is not available.
    /// @tparam TIterator The iterator type.
    /// @tparam TSentinel The sentinel type, defaulted to `TIterator`.
    /// @tparam TKind Whether or not this is a "Sized Subrange": that is, that a calculation for the size of
    /// the subrange can be done in O(1) time and is available.
    template <typename TIterator, typename TSentinel = TIterator,
              SubrangeKind TKind =
                SizedSentinelFor<TIterator, TSentinel> ? SubrangeKind::sized : SubrangeKind::unsized>
    class Subrange
        : private SubrangeSize<iterator_size_type_t<TIterator>, !SizedSentinelFor<TIterator, TSentinel>>,
          public Krys::Ranges::ViewBase
    {
    private:
      inline static constexpr bool SizeRequired =
        TKind == SubrangeKind::sized && !SizedSentinelFor<TIterator, TSentinel>;
      using TSize = iterator_size_type_t<TIterator>;
      using TBaseSize = SubrangeSize<TSize, SizeRequired>;

    public:
      /// @brief The `iterator` type for this subrange, dictated by the template parameter `TIterator`.
      using iterator = TIterator;

      /// @brief The `const_iterator` type for this subrange, dictated by the template parameter `TIterator`.
      using const_iterator = iterator;

      /// @brief The `sentinel` type for this subrange, dictated by the template parameter `TSentinel`.
      using sentinel = TSentinel;

      /// @brief The `const_sentinel` type for this subrange, dictated by the template parameter `TSentinel`.
      using const_sentinel = sentinel;

      /// @brief The iterator category. Same as the iterator category for `TIterator`.
      using iterator_category =
        conditional_t<IsContiguousIterator<iterator>, contiguous_iterator_tag, iterator_category_t<iterator>>;

      /// @brief The iterator concept. Same as the iterator concept for `TIterator`.
      using iterator_concept =
        conditional_t<IsContiguousIterator<iterator>, contiguous_iterator_tag, iterator_concept_t<iterator>>;

      /// @brief The `pointer` type. Same as the `pointer` type for `TIterator`.
      using pointer = iterator_pointer_t<iterator>;

      /// @brief The `const_pointer` type. Same as the `const_pointer` type for `TIterator`.
      using const_pointer = pointer;

      /// @brief The `reference` type. Same as the `reference` type for `TIterator`.
      using reference = iterator_reference_t<iterator>;

      /// @brief The `const_reference` type. Same as the `const_reference` type for `TIterator`.
      using const_reference = reference;

      /// @brief The `value_type.` Same as the `value_type` for `TIterator`.
      using value_type = iterator_value_type_t<iterator>;

      /// @brief The `difference_type.` Same as the `difference_type` for `TIterator`.
      using difference_type = iterator_difference_type_t<iterator>;

      /// @brief The `size_type.` Same as the `size_type` for `TIterator`.
      using size_type = TSize;

      /// @brief Constructs a subrange containing a defaulted iterator and a defaulted
      /// sentinel.
      constexpr Subrange() = default;

      /// @brief Constructs a subrange with its begin and end constructed by `range`'s
      /// `begin()` and `end()` values.
      /// @param[in] range The Range to get the `begin()` and `end()` out of to initialize the subrange's
      /// iterators.
      template <typename TRange>
      requires(!SameType<remove_cvref_t<TRange>, Subrange>)
      constexpr Subrange(TRange &&range) noexcept(
        NoThrowConstructible<Subrange, range_iterator_t<remove_cvref_t<TRange>>,
                             range_sentinel_t<remove_cvref_t<TRange>>>)
          : Subrange(Krys::Ranges::begin(range), Krys::Ranges::end(range))
      {
      }

      /// @brief Constructs a subrange with its begin and end constructed by `range`'s
      /// `begin()` and `end()` values.
      /// @param[in] range The Range to get the `begin()` and `end()` out of to initialize the subrange's
      /// iterators.
      /// @param[in] size The size to construct with.
      template <typename TRange, SubrangeKind TStrawmanKind = TKind>
      requires(TStrawmanKind == SubrangeKind::sized)
      constexpr Subrange(TRange &&range, size_type size) noexcept(
        noexcept(Subrange(Krys::Ranges::begin(range), Krys::Ranges::end(range), std::move(size))))
          : Subrange(Krys::Ranges::begin(range), Krys::Ranges::end(range), std::move(size))
      {
      }

      /// @brief Constructs a subrange with its begin and end constructed by `range`'s
      /// `begin()` and `end()` values.
      /// @param[in] it An iterator value to `std::move` in.
      /// @param[in] sen A sentinel value to `std::move` in.
      constexpr Subrange(iterator it, sentinel sen) noexcept(NoThrowMoveConstructible<iterator>
                                                             && NoThrowMoveConstructible<sentinel>)
          : Subrange(SizeMark {}, IntegralConstant<bool, SizeRequired>(), std::move(it), std::move(sen))
      {
      }

      /// @brief Constructs a subrange with its begin and end constructed by `range`'s
      /// `begin()` and `end()` values.
      /// @param[in] it An iterator value to construct with.
      /// @param[in] sen A sentinel value to construct with.
      /// @param[in] size The size to construct with.
      template <SubrangeKind TStrawmanKind = TKind>
      requires(TStrawmanKind == SubrangeKind::sized)
      constexpr Subrange(iterator it, sentinel sen,
                         size_type size) noexcept(NoThrowMoveConstructible<iterator>
                                                  && NoThrowConstructible<sentinel>
                                                  && NoThrowMoveConstructible<size_type>)
          : TBaseSize(std::move(size)), _it(std::move(it)), _sen(std::move(sen))
      {
      }

      /// @brief The stored begin iterator.
      constexpr iterator begin() & noexcept
      {
        if constexpr (CopyConstructible<iterator>)
        {
          return this->_it;
        }
        else
        {
          return std::move(this->_it);
        }
      }

      /// @brief The stored begin iterator.
      constexpr iterator begin() const & noexcept
      {
        return this->_it;
      }

      /// @brief The stored begin iterator.
      constexpr iterator begin() && noexcept
      {
        return std::move(this->_it);
      }

      /// @brief The stored end iterator.
      constexpr const sentinel &end() const & noexcept
      {
        return this->_sen;
      }

      /// @brief The stored end iterator.
      constexpr sentinel &end() & noexcept
      {
        return this->_sen;
      }

      /// @brief The stored end iterator.
      constexpr sentinel &&end() && noexcept
      {
        return std::move(this->_sen);
      }

      /// @brief Whether or not this range is empty.
      /// @returns `begin()` == `end()`
      constexpr bool empty() const noexcept
      {
        return this->_it == this->_sen;
      }

      /// @brief The stored begin iterator, const-ified.
      /// @remarks This must be reimplemetned at some point.
      constexpr iterator cbegin() const noexcept
      {
        return this->_it;
      }

      /// @brief The stored end iterator.
      /// @remarks This must be reimplemetned at some point.
      constexpr sentinel cend() const noexcept
      {
        return this->_sen;
      }

      /// @brief The size of the range.
      /// @returns @code std::distance(begin(), end()) @endcode
      /// @remarks This function call only works if the `TKind` of this subrange is SubrangeKind::sized.
      template <SubrangeKind TStrawmanKind = TKind>
      requires(TStrawmanKind == SubrangeKind::sized)
      constexpr size_type size() const noexcept
      {
        if constexpr (SizeRequired)
        {
          return this->TBaseSize::_size;
        }
        else
        {
          return std::distance(this->_it, this->_sen);
        }
      }

      /// @brief A `pointer` to the range of elements.
      /// @returns `std::addressof(`*begin()).
      /// @remarks This function call only works if the `iterator_concept` is a `contiguous_iterator_tag` or
      /// better.
      template <typename TStrawman = TIterator>
      requires IsContiguousIterator<TStrawman>
      constexpr pointer data() const noexcept
      {
        return to_address(this->_it);
      }

      /// @brief A `pointer` to the range of elements.
      /// @returns `std::addressof(`*begin()).
      /// @remarks This function call only works if the `iterator_concept` is a `contiguous_iterator_tag` or
      /// better.
      template <typename TStrawman = TIterator>
      requires IsContiguousIterator<TStrawman>
      constexpr reference operator[](size_type index) const noexcept
      {
        return this->_it[index];
      }

      /// @brief Produces a copy of the subrange and advances the `begin()` iterator by 1.
      /// @remarks This function call only works if the underlying iterator and sentinal types are copyable.
      KRYS_NODISCARD constexpr Subrange
        next() const & noexcept((NoThrowCopyConstructible<iterator> && NoThrowCopyConstructible<sentinel>)
                                && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>())))
      {
        auto it = this->_it;
        Krys::Ranges::iter_advance(it);
        return Subrange(std::move(it), this->_sen);
      }

      /// @brief Produces a copy of the subrange and advances the `begin()` iterator by 1.
      /// @remarks This function call can be more efficient and allows working with move-only iterators.
      /// This function call will move the iterators underlying this object.
      KRYS_NODISCARD constexpr Subrange
        next() && noexcept((NoThrowMoveConstructible<iterator> && NoThrowMoveConstructible<sentinel>)
                           && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>())))
      {
        iterator it = std::move(this->_it);
        Krys::Ranges::iter_advance(it);
        return Subrange(std::move(it), std::move(this->_sen));
      }

      /// @brief Produces a copy of the subrange and advances the `begin()` iterator by `diff`.
      /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
      /// @remarks This function call only works if the underlying iterator and sentinal types are copyable.
      KRYS_NODISCARD constexpr Subrange next(difference_type diff) const & noexcept(
        (NoThrowCopyConstructible<iterator> && NoThrowCopyConstructible<sentinel>)
        && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>(), std::declval<difference_type>())))
      {
        auto it = this->_it;
        Krys::Ranges::iter_advance(it, diff);
        return Subrange(std::move(it), this->_sen);
      }

      /// @brief Produces a copy of the subrange and advances the `begin()` iterator by `diff`.
      /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
      /// @remarks This function call can be more efficient and allows working with move-only iterators.
      /// This function call will move the iterators underlying this object.
      KRYS_NODISCARD constexpr Subrange next(difference_type diff) && noexcept(
        (NoThrowMoveConstructible<iterator> && NoThrowMoveConstructible<sentinel>)
        && noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>(), std::declval<difference_type>())))
      {
        iterator it = std::move(this->_it);
        Krys::Ranges::iter_advance(it, diff);
        return Subrange(std::move(it), std::move(this->_sen));
      }

      /// @brief Produces a copy of the subrange and recedes the `begin()` iterator by `diff`.
      /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
      /// @remarks This function call requires that the underlying iterator are bidirectional.
      KRYS_NODISCARD constexpr Subrange prev(difference_type diff = 1) const
        noexcept((NoThrowCopyConstructible<iterator> && NoThrowCopyConstructible<sentinel>)
                 && noexcept(Krys::Ranges::iter_recede(std::declval<iterator &>(),
                                                       std::declval<difference_type>())))
      {
        auto it = this->_it;
        Krys::Ranges::iter_recede(it, diff);
        return Subrange(std::move(it), this->_sen);
      }

      /// @brief Advances the `begin()` iterator of this subrange by `diff` or just `1` if
      /// the argument is not specified.
      /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
      constexpr Subrange &advance(difference_type diff = 1) noexcept(
        noexcept(Krys::Ranges::iter_advance(std::declval<iterator &>(), std::declval<difference_type>())))
      {
        Krys::Ranges::iter_advance(this->_it, diff);
        return *this;
      }

      /// @brief Recedes the `begin()` iterator of this subrange by `diff` or just `1` if the
      /// argument is not specified.
      /// @param[in] diff The amount to move this iterator by. Can be positive or negative.
      /// @remarks This function call requires that the underlying iterator are bidirectional.
      constexpr Subrange &recede(difference_type diff = 1) noexcept(
        noexcept(Krys::Ranges::iter_recede(std::declval<iterator &>(), std::declval<difference_type>())))
      {
        Krys::Ranges::iter_recede(this->_it, diff);
        return *this;
      }

    private:
      template <typename TArgIterator, typename TArgSentinel>
      constexpr Subrange(SizeMark, std::true_type, TArgIterator &&it,
                         TArgSentinel &&sen) noexcept(NoThrowConstructible<iterator, TArgIterator>
                                                      && NoThrowConstructible<sentinel, TArgSentinel>
                                                      && NoThrowConstructible<TBaseSize, TSize>)
          : TBaseSize(static_cast<TSize>(sen - it)), _it(std::forward<TArgIterator>(it)),
            _sen(std::forward<TArgSentinel>(sen))
      {
      }

      template <typename TArgIterator, typename TArgSentinel>
      constexpr Subrange(SizeMark, std::false_type, TArgIterator &&it,
                         TArgSentinel &&sen) noexcept(NoThrowConstructible<iterator, TArgIterator>
                                                      && NoThrowConstructible<sentinel, TArgSentinel>)
          : _it(std::forward<TArgIterator>(it)), _sen(std::forward<TArgSentinel>(sen))
      {
      }

      iterator _it;
      sentinel _sen;
    };
  }

#if KRYS_CONFIG(STD_LIBRARY_RANGES)
  // std::subrange is busted and I'm not interested in digging into why it's busted
  using std::ranges::subrange;
  using std::ranges::subrange_kind;
#else
  /// @brief The type of subrange, sized or unsized.
  using subrange_kind = Impl::SubrangeKind;

  /// @brief A general-purpose iterator-sentinel (or iterator-sentinel-size) container.
  template <typename TIterator, typename TSentinel = TIterator,
            subrange_kind TKind =
              SizedSentinelFor<TIterator, TSentinel> ? subrange_kind::sized : subrange_kind::unsized>
  using subrange = Impl::Subrange<TIterator, TSentinel, TKind>;
#endif

  template <typename TRange>
  using subrange_for_t = Impl::Subrange<range_iterator_t<TRange>, range_sentinel_t<TRange>>;

  template <typename TRange>
  using csubrange_for_t = Impl::Subrange<range_const_iterator_t<TRange>, range_const_sentinel_t<TRange>>;

  /// @brief Decomposes a range into its two iterators and returns it as a subrange.
  template <typename TRange>
  constexpr subrange_for_t<TRange>
    CreateSubrange(TRange &&range) noexcept(NoThrowConstructible<TRange, subrange_for_t<TRange>>)
  {
    return subrange_for_t<TRange>(Krys::Ranges::begin(range), Krys::Ranges::end(range));
  }

  /// @brief Takes two iterators and returns them as a subrange.
  template <typename TIterator, typename TSentinel>
  constexpr subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>
    CreateSubrange(TIterator &&it, TSentinel &&sen) noexcept(
      NoThrowConstructible<subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>, TIterator,
                           TSentinel>)
  {
    return subrange<remove_cvref_t<TIterator>, remove_cvref_t<TSentinel>>(std::forward<TIterator>(it),
                                                                          std::forward<TSentinel>(sen));
  }
}

#if KRYS_CONFIG(STD_LIBRARY_BORROWED_RANGE)
namespace std::ranges
{

  template <typename TIterator, typename TSentinel, Krys::Ranges::Impl::SubrangeKind TKind>
  inline constexpr bool enable_borrowed_range<Krys::Ranges::Impl::Subrange<TIterator, TSentinel, TKind>> =
    true;
}
#endif

namespace Krys::Ranges
{
  /// @brief Mark subranges as appropriately borrowed ranges.
  template <typename TIterator, typename TSentinel, Krys::Ranges::Impl::SubrangeKind TKind>
  inline constexpr bool enable_borrowed_range<Impl::Subrange<TIterator, TSentinel, TKind>> = true;
}
