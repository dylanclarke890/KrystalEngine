#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Hashing/FastHash.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <utility>

namespace krys
{
  /// @brief A set that keeps its elements in the order they were added, and can move an element to the end
  /// or insert one before another.
  ///
  /// The elements live in one array, linked in order and indexed by a `HashMap` from element to slot; a
  /// removed element's slot is reused by a later insert.
  /// @note Iterators and references stay valid until their element is removed, whatever else is inserted or
  /// removed meanwhile.
  template <typename T, typename THash = FastHash<T>, typename TKeyEqual = std::equal_to<T>>
  class ListHashSet
  {
    constexpr static uint32 None = ~0u;

    struct Node
    {
      Maybe<T> Value;
      uint32 Previous {None};
      uint32 Next {None};
    };

  private:
    List<Node> _nodes;
    HashMap<T, uint32, THash, TKeyEqual> _index;
    uint32 _head {None};
    uint32 _tail {None};
    // The first of the free slots, chained through their `Next`.
    uint32 _free {None};

  public:
    class const_iterator
    {
      friend class ListHashSet;

    private:
      const ListHashSet *_set {nullptr};
      uint32 _slot {None};

      constexpr const_iterator(const ListHashSet *set, uint32 slot) noexcept : _set(set), _slot(slot)
      {
      }

    public:
      using iterator_category = std::bidirectional_iterator_tag;
      using value_type = T;
      using difference_type = std::ptrdiff_t;
      using pointer = const T *;
      using reference = const T &;

      constexpr const_iterator() noexcept = default;

      KRYS_NODISCARD constexpr const T &operator*() const noexcept
      {
        return *_set->_nodes[_slot].Value;
      }

      KRYS_NODISCARD constexpr const T *operator->() const noexcept
      {
        return &**this;
      }

      constexpr const_iterator &operator++() noexcept
      {
        _slot = _set->_nodes[_slot].Next;
        return *this;
      }

      constexpr const_iterator operator++(int) noexcept
      {
        auto copy = *this;
        ++*this;
        return copy;
      }

      constexpr const_iterator &operator--() noexcept
      {
        _slot = _slot == None ? _set->_tail : _set->_nodes[_slot].Previous;
        return *this;
      }

      constexpr const_iterator operator--(int) noexcept
      {
        auto copy = *this;
        --*this;
        return copy;
      }

      KRYS_NODISCARD constexpr bool operator==(const const_iterator &other) const noexcept = default;
    };

    using value_type = T;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using reference = const T &;
    using const_reference = const T &;
    using iterator = const_iterator;
    using reverse_iterator = std::reverse_iterator<const_iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    ListHashSet() noexcept = default;

    ListHashSet(std::initializer_list<T> values)
    {
      for (const auto &value : values)
      {
        insert(value);
      }
    }

    /// @brief A set of the elements of @p range, in order, without repeats.
    template <std::ranges::input_range TRange>
    ListHashSet(std::from_range_t, TRange &&range)
    {
      for (auto &&value : range)
      {
        insert(std::forward<decltype(value)>(value));
      }
    }

    ListHashSet(const ListHashSet &other)
    {
      for (const auto &value : other)
      {
        insert(value);
      }
    }

    ListHashSet(ListHashSet &&other) noexcept
        : _nodes(krys::move(other._nodes)), _index(krys::move(other._index)), _head(other._head),
          _tail(other._tail), _free(other._free)
    {
      other.ResetLinks();
    }

    ListHashSet &operator=(const ListHashSet &other)
    {
      if (this != &other)
      {
        ListHashSet copy(other);
        swap(copy);
      }
      return *this;
    }

    ListHashSet &operator=(ListHashSet &&other) noexcept
    {
      if (this != &other)
      {
        ListHashSet moved(krys::move(other));
        swap(moved);
      }
      return *this;
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return _index.size();
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return _index.empty();
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return {this, _head};
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return {this, None};
    }

    KRYS_NODISCARD const_iterator cbegin() const noexcept
    {
      return begin();
    }

    KRYS_NODISCARD const_iterator cend() const noexcept
    {
      return end();
    }

    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    KRYS_NODISCARD const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    KRYS_NODISCARD const T &front() const noexcept
    {
      return *_nodes[_head].Value;
    }

    KRYS_NODISCARD const T &back() const noexcept
    {
      return *_nodes[_tail].Value;
    }

    KRYS_NODISCARD bool contains(const T &value) const noexcept
    {
      return _index.contains(value);
    }

    KRYS_NODISCARD const_iterator find(const T &value) const noexcept
    {
      auto found = _index.find(value);
      return found == _index.end() ? end() : const_iterator(this, found->second);
    }

    /// @brief Adds @p value at the end, unless it's already in the set.
    std::pair<const_iterator, bool> insert(T value)
    {
      return InsertBeforeSlot(None, krys::move(value));
    }

    /// @brief Adds @p value before @p position, unless it's already in the set.
    std::pair<const_iterator, bool> InsertBefore(const_iterator position, T value)
    {
      return InsertBeforeSlot(position._slot, krys::move(value));
    }

    /// @brief Adds @p value at the end, or moves it there if it's already in the set.
    /// @return Where it is, and whether it was added.
    std::pair<const_iterator, bool> AppendOrMoveToLast(T value)
    {
      if (auto found = _index.find(value); found != _index.end())
      {
        auto slot = found->second;
        Unlink(slot);
        LinkBefore(slot, None);
        return {const_iterator(this, slot), false};
      }
      return insert(krys::move(value));
    }

    /// @brief Removes @p value, if it's in the set.
    /// @return The number of elements removed (0 or 1).
    size_t erase(const T &value) noexcept
    {
      auto found = _index.find(value);
      if (found == _index.end())
      {
        return 0;
      }

      auto slot = found->second;
      _index.erase(found);
      Release(slot);
      return 1;
    }

    /// @brief Removes the element at @p position.
    /// @return The element after it.
    const_iterator erase(const_iterator position) noexcept
    {
      auto next = _nodes[position._slot].Next;
      erase(*position);
      return {this, next};
    }

    void clear() noexcept
    {
      _index.clear();
      _nodes.clear();
      ResetLinks();
    }

    void swap(ListHashSet &other) noexcept
    {
      using std::swap;
      swap(_nodes, other._nodes);
      swap(_index, other._index);
      swap(_head, other._head);
      swap(_tail, other._tail);
      swap(_free, other._free);
    }

    friend void swap(ListHashSet &lhs, ListHashSet &rhs) noexcept
    {
      lhs.swap(rhs);
    }

    KRYS_NODISCARD bool operator==(const ListHashSet &other) const noexcept
    {
      return std::ranges::equal(*this, other);
    }

  private:
    void ResetLinks() noexcept
    {
      _head = None;
      _tail = None;
      _free = None;
    }

    std::pair<const_iterator, bool> InsertBeforeSlot(uint32 before, T value)
    {
      if (auto found = _index.find(value); found != _index.end())
      {
        return {const_iterator(this, found->second), false};
      }

      uint32 slot;
      if (_free != None)
      {
        slot = _free;
        _free = _nodes[slot].Next;
        _nodes[slot].Value.emplace(krys::move(value));
      }
      else
      {
        slot = static_cast<uint32>(_nodes.size());
        _nodes.push_back(Node {krys::move(value)});
      }

      _index.emplace(*_nodes[slot].Value, slot);
      LinkBefore(slot, before);
      return {const_iterator(this, slot), true};
    }

    /// Links @p slot in before @p before, or at the end when @p before is `None`.
    void LinkBefore(uint32 slot, uint32 before) noexcept
    {
      auto &node = _nodes[slot];
      node.Next = before;
      node.Previous = before == None ? _tail : _nodes[before].Previous;
      (node.Previous == None ? _head : _nodes[node.Previous].Next) = slot;
      (before == None ? _tail : _nodes[before].Previous) = slot;
    }

    void Unlink(uint32 slot) noexcept
    {
      auto &node = _nodes[slot];
      (node.Previous == None ? _head : _nodes[node.Previous].Next) = node.Next;
      (node.Next == None ? _tail : _nodes[node.Next].Previous) = node.Previous;
    }

    void Release(uint32 slot) noexcept
    {
      Unlink(slot);
      auto &node = _nodes[slot];
      node.Value.reset();
      node.Previous = None;
      node.Next = _free;
      _free = slot;
    }
  };
}
