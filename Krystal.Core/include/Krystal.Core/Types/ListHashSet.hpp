#pragma once

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <list>
#include <unordered_map>
#include <utility>

namespace krys
{
  template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>>
  class ListHashSet
  {
    using Storage = std::list<T>;
    using Index = std::unordered_map<T, typename Storage::iterator, Hash, KeyEqual>;

  public:
    using value_type = T;
    using size_type = typename Storage::size_type;
    using const_iterator = typename Storage::const_iterator;
    using const_reverse_iterator = typename Storage::const_reverse_iterator;

    ListHashSet() = default;

    ListHashSet(std::initializer_list<T> values)
    {
      for (const auto &value : values)
      {
        add(value);
      }
    }

    ListHashSet(const ListHashSet &other)
    {
      for (const auto &value : other)
      {
        add(value);
      }
    }

    ListHashSet(ListHashSet &&) noexcept = default;

    ListHashSet &operator=(const ListHashSet &other)
    {
      if (this != &other)
      {
        ListHashSet copy(other);
        swap(copy);
      }
      return *this;
    }

    ListHashSet &operator=(ListHashSet &&) noexcept = default;

    size_t size() const noexcept
    {
      return _values.size();
    }

    bool isEmpty() const noexcept
    {
      return _values.empty();
    }

    bool empty() const noexcept
    {
      return isEmpty();
    }

    const_iterator begin() const noexcept
    {
      return _values.cbegin();
    }

    const_iterator end() const noexcept
    {
      return _values.cend();
    }

    const_iterator cbegin() const noexcept
    {
      return begin();
    }

    const_iterator cend() const noexcept
    {
      return end();
    }

    const_reverse_iterator rbegin() const noexcept
    {
      return _values.crbegin();
    }

    const_reverse_iterator rend() const noexcept
    {
      return _values.crend();
    }

    const T &first() const
    {
      return _values.front();
    }

    const T &last() const
    {
      return _values.back();
    }

    bool contains(const T &value) const
    {
      return _index.contains(value);
    }

    const_iterator find(const T &value) const
    {
      auto found = _index.find(value);
      return found == _index.end() ? end() : const_iterator(found->second);
    }

    std::pair<const_iterator, bool> add(const T &value)
    {
      if (auto found = _index.find(value); found != _index.end())
      {
        return {const_iterator(found->second), false};
      }

      _values.push_back(value);
      auto position = std::prev(_values.end());
      _index.emplace(*position, position);
      return {const_iterator(position), true};
    }

    std::pair<const_iterator, bool> add(T &&value)
    {
      if (auto found = _index.find(value); found != _index.end())
      {
        return {const_iterator(found->second), false};
      }

      _values.push_back(std::move(value));
      auto position = std::prev(_values.end());
      _index.emplace(*position, position);
      return {const_iterator(position), true};
    }

    bool remove(const T &value)
    {
      auto found = _index.find(value);
      if (found == _index.end())
      {
        return false;
      }

      _values.erase(found->second);
      _index.erase(found);
      return true;
    }

    void clear() noexcept
    {
      _index.clear();
      _values.clear();
    }

    void swap(ListHashSet &other) noexcept(noexcept(_index.swap(other._index)))
    {
      _values.swap(other._values);
      _index.swap(other._index);
    }

    bool operator==(const ListHashSet &other) const
    {
      return _values == other._values;
    }

  private:
    Storage _values;
    Index _index;
  };

  template <typename T, typename Hash, typename KeyEqual>
  void swap(ListHashSet<T, Hash, KeyEqual> &lhs,
            ListHashSet<T, Hash, KeyEqual> &rhs) noexcept(noexcept(lhs.swap(rhs)))
  {
    lhs.swap(rhs);
  }
}
