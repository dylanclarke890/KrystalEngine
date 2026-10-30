#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace krys
{
  template <typename T, typename Allocator = std::allocator<T>>
  class FixedVector
  {
  public:
    using Container = std::vector<T, Allocator>;
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using difference_type = typename Container::difference_type;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using pointer = typename Container::pointer;
    using const_pointer = typename Container::const_pointer;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using reverse_iterator = typename Container::reverse_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

    FixedVector() = default;
    FixedVector(const FixedVector &) = default;
    FixedVector(FixedVector &&) noexcept = default;
    FixedVector &operator=(const FixedVector &) = default;
    FixedVector &operator=(FixedVector &&) noexcept = default;

    FixedVector(std::initializer_list<T> values) : _values(values)
    {
    }

    FixedVector(const T &value) : _values {value}
    {
    }

    FixedVector(T &&value) : _values {std::move(value)}
    {
    }

    explicit FixedVector(size_t size) : _values(size)
    {
    }

    FixedVector(size_t size, const T &value) : _values(size, value)
    {
    }

    template <typename InputIterator>
    FixedVector(InputIterator first, InputIterator last) : _values(first, last)
    {
    }

    template <typename U, size_t Extent>
    explicit FixedVector(std::span<U, Extent> values) : _values(values.begin(), values.end())
    {
    }

    template <std::invocable<size_t> Generator>
    static FixedVector createWithSizeFromGenerator(size_t size, Generator &&generator)
    {
      FixedVector result;
      result._values.reserve(size);
      for (size_t i = 0; i < size; ++i)
      {
        result._values.emplace_back(std::invoke(generator, i));
      }
      return result;
    }

    template <typename SizedRange, typename Mapper>
    static FixedVector map(SizedRange &&range, Mapper &&mapper)
    {
      FixedVector result;
      if constexpr (std::ranges::sized_range<SizedRange>)
      {
        result._values.reserve(std::ranges::size(range));
      }
      for (auto &&value : range)
      {
        result._values.emplace_back(std::invoke(mapper, value));
      }
      return result;
    }

    size_t size() const noexcept
    {
      return _values.size();
    }

    bool isEmpty() const noexcept
    {
      return _values.empty();
    }

    size_t byteSize() const noexcept
    {
      return _values.size() * sizeof(T);
    }

    iterator begin() noexcept
    {
      return _values.begin();
    }

    iterator end() noexcept
    {
      return _values.end();
    }

    const_iterator begin() const noexcept
    {
      return _values.begin();
    }

    const_iterator end() const noexcept
    {
      return _values.end();
    }

    reverse_iterator rbegin() noexcept
    {
      return _values.rbegin();
    }

    reverse_iterator rend() noexcept
    {
      return _values.rend();
    }

    const_reverse_iterator rbegin() const noexcept
    {
      return _values.rbegin();
    }

    const_reverse_iterator rend() const noexcept
    {
      return _values.rend();
    }

    reference at(size_t index)
    {
      return _values.at(index);
    }

    const_reference at(size_t index) const
    {
      return _values.at(index);
    }

    reference operator[](size_t index)
    {
      return at(index);
    }

    const_reference operator[](size_t index) const
    {
      return at(index);
    }

    reference first()
    {
      return _values.front();
    }

    const_reference first() const
    {
      return _values.front();
    }

    reference last()
    {
      return _values.back();
    }

    const_reference last() const
    {
      return _values.back();
    }

    void clear() noexcept
    {
      _values.clear();
    }

    void fill(const T &value)
    {
      std::ranges::fill(_values, value);
    }

    template <typename U>
    bool contains(const U &value) const
    {
      return find(value) != notFound;
    }

    template <typename Predicate>
    bool containsIf(Predicate &&predicate) const
    {
      return findIf(std::forward<Predicate>(predicate)) != notFound;
    }

    template <typename U>
    size_t find(const U &value) const
    {
      return findIf([&](const T &item) { return item == value; });
    }

    template <typename Predicate>
    size_t findIf(Predicate &&predicate) const
    {
      for (size_t i = 0; i < _values.size(); ++i)
      {
        if (std::invoke(predicate, _values[i]))
        {
          return i;
        }
      }
      return notFound;
    }

    template <typename U>
    size_t reverseFind(const U &value) const
    {
      return reverseFindIf([&](const T &item) { return item == value; });
    }

    template <typename Predicate>
    size_t reverseFindIf(Predicate &&predicate) const
    {
      for (size_t i = _values.size(); i > 0; --i)
      {
        if (std::invoke(predicate, _values[i - 1]))
        {
          return i - 1;
        }
      }
      return notFound;
    }

    template <typename Mapper>
    auto map(Mapper &&mapper) const
    {
      using Result = std::remove_cvref_t<std::invoke_result_t<Mapper, const T &>>;
      std::vector<Result> result;
      result.reserve(_values.size());
      for (const auto &value : _values)
      {
        result.emplace_back(std::invoke(mapper, value));
      }
      return result;
    }

    std::span<const T> span() const noexcept
    {
      return _values;
    }

    std::span<T> mutableSpan() noexcept
    {
      return _values;
    }

    void swap(FixedVector &other) noexcept(noexcept(_values.swap(other._values)))
    {
      _values.swap(other._values);
    }

    bool operator==(const FixedVector &) const = default;

    constexpr static size_t notFound = std::numeric_limits<size_t>::max();

  private:
    Container _values;
  };

  template <typename T, typename Allocator>
  void swap(FixedVector<T, Allocator> &lhs, FixedVector<T, Allocator> &rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }
}
