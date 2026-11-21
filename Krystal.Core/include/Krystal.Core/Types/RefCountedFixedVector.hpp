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

#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"

namespace krys
{
  /// @brief A reference-counted, fixed-size vector, so that copies of its owners share the elements.
  template <typename T>
  class RefCountedFixedVector : public RefCounted<RefCountedFixedVector<T>>
  {
  public:
    using Container = FixedVector<T>;
    using value_type = typename Container::value_type;
    using const_iterator = typename Container::const_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

  private:
    Container _values;

  public:
    KRYS_NODISCARD static Ref<RefCountedFixedVector> create(T &&value)
    {
      return AdoptRef(*new RefCountedFixedVector(Container {krys::move(value)}));
    }

    KRYS_NODISCARD static Ref<RefCountedFixedVector> create(std::initializer_list<T> values)
    {
      return AdoptRef(*new RefCountedFixedVector(Container {values}));
    }

    template <std::invocable<size_t> Generator>
    KRYS_NODISCARD static Ref<RefCountedFixedVector> createWithSizeFromGenerator(size_t size,
                                                                                 Generator &&generator)
    {
      return AdoptRef(*new RefCountedFixedVector(
        Container::createWithSizeFromGenerator(size, std::forward<Generator>(generator))));
    }

    template <typename SizedRange, typename Mapper>
    KRYS_NODISCARD static Ref<RefCountedFixedVector> map(SizedRange &&range, Mapper &&mapper)
    {
      return AdoptRef(*new RefCountedFixedVector(
        Container::map(std::forward<SizedRange>(range), std::forward<Mapper>(mapper))));
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

    KRYS_NODISCARD bool isEmpty() const noexcept
    {
      return _values.isEmpty();
    }

    KRYS_NODISCARD size_t size() const noexcept
    {
      return _values.size();
    }

    KRYS_NODISCARD const T &operator[](size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return _values[index];
    }

    KRYS_NODISCARD const T &first() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.first();
    }

    KRYS_NODISCARD const T &last() const noexcept KRYS_LIFETIME_BOUND
    {
      return _values.last();
    }

    KRYS_NODISCARD bool operator==(const RefCountedFixedVector &other) const
    {
      return _values == other._values;
    }

  private:
    explicit RefCountedFixedVector(Container &&values) noexcept : _values {krys::move(values)}
    {
    }
  };
}
