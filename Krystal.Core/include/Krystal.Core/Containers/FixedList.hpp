/*
 * Copyright (C) 2021-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>
#include <compare>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <ranges>

namespace krys
{
  /// @brief A list whose size is set when it's made: one pointer to a single allocation holding the size and
  /// then the elements, and null (with no allocation) when empty.
  ///
  /// It has `std::vector`'s interface less everything that changes the size (but `clear`), for members of
  /// objects that are made often and whose element count doesn't change.
  template <typename T>
  class FixedList
  {
  private:
    struct Header
    {
      size_t Size;
    };

    constexpr static size_t Alignment = alignof(Header) > alignof(T) ? alignof(Header) : alignof(T);
    constexpr static size_t ElementsOffset = (sizeof(Header) + alignof(T) - 1) / alignof(T) * alignof(T);

    Header *_storage {nullptr};

  public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    FixedList() noexcept = default;

    FixedList(const FixedList &other) noexcept : FixedList(other.begin(), other.end())
    {
    }

    FixedList(FixedList &&other) noexcept : _storage(std::exchange(other._storage, nullptr))
    {
    }

    FixedList &operator=(const FixedList &other) noexcept
    {
      if (this != &other)
      {
        FixedList(other).swap(*this);
      }
      return *this;
    }

    FixedList &operator=(FixedList &&other) noexcept
    {
      FixedList(krys::move(other)).swap(*this);
      return *this;
    }

    ~FixedList() noexcept
    {
      Destroy();
    }

    FixedList(std::initializer_list<T> values) noexcept : FixedList(values.begin(), values.end())
    {
    }

    /// @brief A list of @p size value-initialised elements.
    explicit FixedList(size_t size) noexcept
    {
      Construct(size, [](T *element, size_t) { new (element) T(); });
    }

    FixedList(size_t size, const T &value) noexcept
    {
      Construct(size, [&](T *element, size_t) { new (element) T(value); });
    }

    template <std::forward_iterator TIterator, std::sentinel_for<TIterator> TSentinel>
    FixedList(TIterator first, TSentinel last) noexcept
    {
      Construct(static_cast<size_t>(std::ranges::distance(first, last)),
                [&](T *element, size_t) { new (element) T(*first++); });
    }

    /// @brief A list of the elements of @p range.
    template <std::ranges::forward_range TRange>
    FixedList(std::from_range_t, TRange &&range) noexcept
        : FixedList(std::ranges::begin(range), std::ranges::end(range))
    {
    }

    /// @brief A list of @p size elements, element `i` being `generator(i)`.
    /// @brief A list of `mapper(element)` for each element of @p range.
    KRYS_NODISCARD size_t size() const noexcept
    {
      return _storage ? _storage->Size : 0;
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return !_storage;
    }

    KRYS_NODISCARD T *data() noexcept
    {
      return Elements();
    }

    KRYS_NODISCARD const T *data() const noexcept
    {
      return Elements();
    }

    KRYS_NODISCARD iterator begin() noexcept
    {
      return Elements();
    }

    KRYS_NODISCARD iterator end() noexcept
    {
      return Elements() + size();
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return Elements();
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return Elements() + size();
    }

    KRYS_NODISCARD reverse_iterator rbegin() noexcept
    {
      return reverse_iterator(end());
    }

    KRYS_NODISCARD reverse_iterator rend() noexcept
    {
      return reverse_iterator(begin());
    }

    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }

    KRYS_NODISCARD const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }

    KRYS_NODISCARD T &operator[](size_t index) noexcept
    {
      krys_assert(index < size());
      return Elements()[index];
    }

    KRYS_NODISCARD const T &operator[](size_t index) const noexcept
    {
      krys_assert(index < size());
      return Elements()[index];
    }

    KRYS_NODISCARD T &front() noexcept
    {
      return (*this)[0];
    }

    KRYS_NODISCARD const T &front() const noexcept
    {
      return (*this)[0];
    }

    KRYS_NODISCARD T &back() noexcept
    {
      return (*this)[size() - 1];
    }

    KRYS_NODISCARD const T &back() const noexcept
    {
      return (*this)[size() - 1];
    }

    void clear() noexcept
    {
      Destroy();
      _storage = nullptr;
    }

    void swap(FixedList &other) noexcept
    {
      std::swap(_storage, other._storage);
    }

    friend void swap(FixedList &a, FixedList &b) noexcept
    {
      a.swap(b);
    }

    KRYS_NODISCARD friend bool operator==(const FixedList &a, const FixedList &b) noexcept
    {
      return std::ranges::equal(a, b);
    }

  private:
    KRYS_NODISCARD T *Elements() const noexcept
    {
      return _storage
               ? std::launder(reinterpret_cast<T *>(reinterpret_cast<byte *>(_storage) + ElementsOffset))
               : nullptr;
    }

    /// @brief Allocates room for @p size elements and constructs each with `construct(element, index)`.
    void Construct(size_t size, const auto &construct) noexcept
    {
      krys_debug_assert(!_storage);
      if (!size)
      {
        return;
      }

      void *memory = ::operator new(ElementsOffset + size * sizeof(T), std::align_val_t {Alignment});
      _storage = new (memory) Header {size};
      T *elements = Elements();
      for (size_t index = 0; index < size; ++index)
      {
        construct(elements + index, index);
      }
    }

    void Destroy() noexcept
    {
      if (!_storage)
      {
        return;
      }

      std::destroy_n(Elements(), _storage->Size);
      _storage->~Header();
      ::operator delete(_storage, std::align_val_t {Alignment});
    }
  };

  static_assert(sizeof(FixedList<int>) == sizeof(void *));
}
