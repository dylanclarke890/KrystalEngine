#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Ranges/ADL.hpp"
#include "Krystal.Lib/Ranges/Iterator.hpp"
#include <iterator>
#include <ranges>
#include <utility>

namespace Krys::Ranges
{
  namespace Impl
  {
    template <typename TIterator>
    class CountedIterator
    {
    private:
      using TIteratorDiff = iterator_difference_type_t<TIterator>;

      static constexpr bool operator_plusplus_noexcept() noexcept
      {
        return noexcept(++std::declval<TIterator &>()) && noexcept(--std::declval<TIteratorDiff &>());
      }

      static constexpr bool operator_minusminus_noexcept() noexcept
      {
        return noexcept(--std::declval<TIterator &>()) && noexcept(++std::declval<TIteratorDiff &>());
      }

    public:
      using iterator_type = TIterator;
      using difference_type = TIteratorDiff;

      constexpr CountedIterator() = default;
      constexpr CountedIterator(TIterator it,
                                TIteratorDiff count) noexcept(NoThrowMoveConstructible<iterator_type>)
          : _iterator(std::move(it)), _count(std::move(count))
      {
      }

      template <typename TIterator2, std::enable_if_t<ConvertibleTo<TIterator2, iterator_type>> * = nullptr>
      constexpr CountedIterator(const CountedIterator<TIterator2> &from) noexcept(
        NoThrowConvertibleTo<const TIterator2 &, iterator_type>)
          : _count(from._count), _iterator(from._iterator)
      {
      }

      template <typename TIterator2>
      requires(ConvertibleTo<TIterator2, iterator_type>)
      constexpr CountedIterator(CountedIterator<TIterator2> &&from) noexcept(
        NoThrowConvertibleTo<TIterator2 &&, iterator_type>)
          : _count(std::move(from._count)), _iterator(std::move(from._iterator))
      {
      }

      template <typename TIterator2>
      requires(Assignable<TIterator2, iterator_type>)
      constexpr CountedIterator &operator=(const CountedIterator<TIterator2> &right) noexcept(
        NoThrowAssignable<const TIterator2 &, iterator_type>)
      {
        this->_count = right._count;
        this->_iterator = right._iterator;
        return *this;
      }

      template <typename TIterator2>
      requires(Assignable<TIterator2, iterator_type>)
      constexpr CountedIterator &operator=(CountedIterator<TIterator2> &&right) noexcept(
        NoThrowAssignable<TIterator2 &&, iterator_type>)
      {
        this->_count = std::move(right._count);
        this->_iterator = std::move(right._iterator);
        return *this;
      }

      constexpr TIterator base() const & noexcept(NoThrowCopyConstructible<TIterator>)
      {
        return this->_iterator;
      }

      constexpr TIterator base() & noexcept(NoThrowCopyConstructible<TIterator>)
      {
        return this->_iterator;
      }

      constexpr TIterator &&base() && noexcept
      {
        return std::move(this->_iterator);
      }

      constexpr TIteratorDiff count() const noexcept
      {
        return this->_count;
      }

      constexpr decltype(auto) operator*() noexcept(noexcept(*std::declval<TIterator &>()))
      {
        return *this->_iterator;
      }

      constexpr decltype(auto) operator*() const noexcept(noexcept(*std::declval<const TIterator &>()))
      {
        return *this->_iterator;
      }

      constexpr CountedIterator &operator++() noexcept(operator_plusplus_noexcept())
      {
        ++this->_iterator;
        --this->_count;
        return *this;
      }

      constexpr CountedIterator operator++(int) noexcept(operator_plusplus_noexcept())
      {
        auto copy = (*this);
        ++(*this);
        return copy;
      }

      constexpr CountedIterator &operator--() noexcept(operator_minusminus_noexcept())
      {
        --this->_iterator;
        ++this->_count;
        return *this;
      }

      constexpr CountedIterator operator--(int) noexcept(operator_minusminus_noexcept())
      {
        auto copy = (*this);
        --(*this);
        return copy;
      }

      template <typename TIteratorType = TIterator>
      requires(RandomAccessIterator<TIteratorType>)
      constexpr CountedIterator operator+(difference_type diff) const
      {
        return CountedIterator(this->_iterator + diff, this->_count + diff);
      }

      template <typename TIteratorType = TIterator>
      requires(RandomAccessIterator<TIteratorType>)
      friend constexpr CountedIterator operator+(difference_type diff, const CountedIterator &right)
      {
        return CountedIterator(right._iterator + diff, right._count + diff);
      }

      template <typename TIteratorType = TIterator>
      requires(RandomAccessIterator<TIteratorType>)
      constexpr CountedIterator &operator+=(difference_type diff)
      {
        this->_iterator += diff;
        this->_count += diff;
        return *this;
      }

      template <typename TIteratorType = TIterator>
      requires(RandomAccessIterator<TIteratorType>)
      constexpr CountedIterator operator-(difference_type diff) const
      {
        return CountedIterator(this->_iterator - diff, this->_count - diff);
      }

      template <typename TRightIterator>
      requires(RandomAccessIterator<TRightIterator> && RandomAccessIterator<TIterator>)
      friend constexpr iterator_difference_type_t<TRightIterator>
        operator-(const CountedIterator &left, const CountedIterator<TRightIterator> &right)
      {
        return left._iterator - right._iterator;
      }

      template <typename TRightIterator, typename TIteratorType = TIterator>
      requires(RandomAccessIterator<TIteratorType>)
      constexpr CountedIterator &operator-=(difference_type diff)
      {
        this->_iterator -= diff;
        this->_count -= diff;
        return *this;
      }

      template <typename TIteratorType = TIterator>
      requires(RandomAccessIterator<TIteratorType>)
      constexpr decltype(auto) operator[](difference_type index) const
      {
        return this->_iterator[index];
      }

      friend constexpr difference_type operator-(const CountedIterator &left,
                                                 std::default_sentinel_t) noexcept
      {
        return left._count;
      }

      friend constexpr difference_type operator-(std::default_sentinel_t, const CountedIterator &right) noexcept
      {
        return -right._count;
      }

      template <typename TRightIterator>
      friend constexpr bool operator==(const CountedIterator &left,
                                       const CountedIterator<TRightIterator> &right) noexcept
      {
        return left._iterator == right._iterator && left._count == right._count;
      }

      friend constexpr bool operator==(const CountedIterator &left, std::default_sentinel_t) noexcept
      {
        return left._count == static_cast<TIteratorDiff>(0);
      }

      template <typename TRightIterator>
      friend constexpr bool operator!=(const CountedIterator &left,
                                       const CountedIterator<TRightIterator> &right) noexcept
      {
        return left._iterator != right._iterator || left._count != right._count;
      }

      friend constexpr bool operator!=(const CountedIterator &left, std::default_sentinel_t) noexcept
      {
        return left._count != static_cast<TIteratorDiff>(0);
      }

      friend constexpr iterator_rvalue_reference_t<TIterator>
        iter_move(const CountedIterator &it) noexcept(noexcept(::std::ranges::iter_move(it.base())))
      {
        return ::std::ranges::iter_move(it.base());
      }

      template <typename TRightIterator>
      friend constexpr void
        iter_swap(const CountedIterator &left, const CountedIterator<TRightIterator> &right) noexcept(
          noexcept(::std::ranges::iter_swap(left.base(), right.base())))
      {
        ::std::ranges::iter_swap(left.base(), right.base());
      }

    private:
      TIterator _iterator = TIterator();
      difference_type _count = difference_type {};
    };
  }

  /// @brief A counted iterator that stores an iterator plus a count, which is used to iterator over the
  /// specified count of elements. Useful for algorithms wherein the iterator is not random access but still
  /// works on a given iterator and a size (e.g., the `std::ranges::copy` algorithm).
  /// @tparam TIterator The Iterator to wrap. The count is a `difference_type` that is associated with the
  /// Iterator. (The `difference_type` is usually a signed type such as the `ptrdiff_t` type.)
  template <typename TIterator>
  using CountedIterator = std::counted_iterator<TIterator>;
}

namespace std
{
  template <typename TIterator>
  struct iterator_traits<Krys::Ranges::CountedIterator<TIterator>> : iterator_traits<TIterator>
  {
    using pointer = void;
  };
}
