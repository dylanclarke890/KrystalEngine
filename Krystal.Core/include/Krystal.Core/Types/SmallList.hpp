#pragma once

#include "Krystal.Core/Attributes.hpp"
#include <algorithm>
#include <compare>
#include <initializer_list>
#include <llvm/ADT/SmallVector.h>
#include <ranges>

namespace krys
{
  /// @brief A `List` that keeps up to @p N elements inline, so it allocates only once it grows past them.
  ///
  /// It has `std::vector`'s interface (less `at`, `shrink_to_fit` and the `c`-prefixed iterators), over
  /// LLVM's `SmallVector`, whose other members it hides.
  /// @note Without an @p N, it picks one that keeps it to about 64 bytes, which suits most uses.
  /// @warning It doesn't attempt to be exception safe.
  template <typename T, unsigned N = llvm::CalculateSmallVectorDefaultInlinedElements<T>::value>
  class SmallList : private llvm::SmallVector<T, N>
  {
  private:
    template <typename U, unsigned M>
    friend class SmallList;

    using Base = llvm::SmallVector<T, N>;

  public:
    using typename Base::const_iterator;
    using typename Base::const_pointer;
    using typename Base::const_reference;
    using typename Base::const_reverse_iterator;
    using typename Base::difference_type;
    using typename Base::iterator;
    using typename Base::pointer;
    using typename Base::reference;
    using typename Base::reverse_iterator;
    using typename Base::size_type;
    using typename Base::value_type;

    using Base::Base;

    SmallList() noexcept = default;

    /// @brief A list of the elements of @p range.
    template <std::ranges::input_range TRange>
    SmallList(std::from_range_t, TRange &&range) noexcept
    {
      append_range(std::forward<TRange>(range));
    }

    /// @brief Takes the elements of a list with a different inline capacity.
    template <unsigned M>
    requires(M != N)
    SmallList(SmallList<T, M> &&other) noexcept : Base(static_cast<llvm::SmallVectorImpl<T> &&>(other))
    {
    }

    template <unsigned M>
    requires(M != N)
    SmallList(const SmallList<T, M> &other) noexcept : Base(other.begin(), other.end())
    {
    }

    template <unsigned M>
    requires(M != N)
    SmallList &operator=(SmallList<T, M> &&other) noexcept
    {
      Base::operator=(static_cast<llvm::SmallVectorImpl<T> &&>(other));
      return *this;
    }

    SmallList &operator=(std::initializer_list<T> values) noexcept
    {
      Base::assign(values);
      return *this;
    }

    using Base::begin;
    using Base::end;
    using Base::rbegin;
    using Base::rend;

    using Base::capacity;
    using Base::empty;
    using Base::max_size;
    using Base::size;

    using Base::back;
    using Base::data;
    using Base::front;
    using Base::operator[];

    using Base::assign;
    using Base::clear;
    using Base::emplace_back;
    using Base::erase;
    using Base::insert;
    using Base::pop_back;
    using Base::push_back;
    using Base::reserve;
    using Base::resize;

    /// @brief Appends the elements of @p range.
    template <std::ranges::input_range TRange>
    void append_range(TRange &&range) noexcept
    {
      if constexpr (std::ranges::common_range<TRange>)
      {
        Base::append(std::ranges::begin(range), std::ranges::end(range));
      }
      else
      {
        for (auto &&element : range)
        {
          Base::emplace_back(std::forward<decltype(element)>(element));
        }
      }
    }

    void swap(SmallList &other) noexcept
    {
      Base::swap(other);
    }

    friend void swap(SmallList &a, SmallList &b) noexcept
    {
      a.swap(b);
    }

    KRYS_NODISCARD friend bool operator==(const SmallList &a, const SmallList &b) noexcept
    {
      return std::ranges::equal(a, b);
    }

    KRYS_NODISCARD friend auto operator<=>(const SmallList &a, const SmallList &b) noexcept
    {
      return std::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end());
    }
  };
}
