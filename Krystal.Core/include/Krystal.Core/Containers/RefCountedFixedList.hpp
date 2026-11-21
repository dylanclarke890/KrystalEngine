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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Containers/FixedList.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Move.hpp"
#include <initializer_list>
#include <utility>

namespace krys
{
  /// @brief A reference-counted `FixedList`, so that copies of its owners share the elements.
  template <typename T>
  class RefCountedFixedList : public RefCounted<RefCountedFixedList<T>>
  {
  public:
    using Container = FixedList<T>;
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using const_reference = typename Container::const_reference;
    using const_pointer = typename Container::const_pointer;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

  private:
    Container _values;

  public:
    KRYS_NODISCARD static Ref<RefCountedFixedList> Create(T value) noexcept
    {
      return AdoptRef(*new RefCountedFixedList(Container {krys::move(value)}));
    }

    KRYS_NODISCARD static Ref<RefCountedFixedList> Create(std::initializer_list<T> values) noexcept
    {
      return AdoptRef(*new RefCountedFixedList(Container {values}));
    }

    /// @brief A list of the elements of @p range.
    template <std::ranges::forward_range TRange>
    KRYS_NODISCARD static Ref<RefCountedFixedList> Create(std::from_range_t, TRange &&range) noexcept
    {
      return AdoptRef(*new RefCountedFixedList(Container(std::from_range, std::forward<TRange>(range))));
    }

    /// @brief A list of @p size elements, element `i` being `generator(i)`.
    /// @brief A list of `mapper(element)` for each element of @p range.
    KRYS_NODISCARD size_t size() const noexcept
    {
      return _values.size();
    }

    KRYS_NODISCARD bool empty() const noexcept
    {
      return _values.empty();
    }

    KRYS_NODISCARD const T *data() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.data();
    }

    KRYS_NODISCARD const_iterator begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.begin();
    }

    KRYS_NODISCARD const_iterator end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.end();
    }

    KRYS_NODISCARD const_reverse_iterator rbegin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.rbegin();
    }

    KRYS_NODISCARD const_reverse_iterator rend() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.rend();
    }

    KRYS_NODISCARD const T &operator[](size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return _values[index];
    }

    KRYS_NODISCARD const T &front() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.front();
    }

    KRYS_NODISCARD const T &back() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.back();
    }

    KRYS_NODISCARD bool operator==(const RefCountedFixedList &other) const noexcept
    {
      return _values == other._values;
    }

  private:
    explicit RefCountedFixedList(Container values) noexcept : _values {krys::move(values)}
    {
    }
  };
}
