/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Utils/IndexedRange.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys
{
  template <typename IteratorA, typename IteratorB>
  class ZippedRangeIterator
  {
  private:
    IteratorA _iteratorA;
    IteratorB _iteratorB;

  public:
    ZippedRangeIterator(IteratorA &&iteratorA, IteratorB &&iteratorB)
        : _iteratorA(krys::move(iteratorA)), _iteratorB(krys::move(iteratorB))
    {
    }

    ZippedRangeIterator &operator++() noexcept
    {
      ++_iteratorA;
      ++_iteratorB;
      return *this;
    }

    KRYS_NODISCARD auto operator*() noexcept
    {
      return std::pair<decltype(*_iteratorA), decltype(*_iteratorB)> {*_iteratorA, *_iteratorB};
    }

    KRYS_NODISCARD bool operator==(const ZippedRangeIterator &other) const noexcept
    {
      // To ensure that we compare equal to end() even when we iterate two
      // collections of different sizes, we need to compare both A and B.
      // (Otherwise comparing either A or B would be sufficient, since they
      // increment in lockstep.)
      return _iteratorA == other._iteratorA || _iteratorB == other._iteratorB;
    }
  };

  template <typename Iterator>
  class ZippedRange
  {
  private:
    Iterator _begin;
    Iterator _end;

  public:
    template <typename CollectionA, typename CollectionB>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): used for both ends, so never moved.
    ZippedRange(CollectionA &&collectionA, CollectionB &&collectionB) noexcept
        : _begin(boundsCheckedBegin(collectionA), boundsCheckedBegin(collectionB)),
          _end(boundsCheckedEnd(collectionA), boundsCheckedEnd(collectionB))
    {
      // Prevent use after destruction of a returned temporary (a borrowed range's iterators outlive it, so
      // the collections are passed on as lvalues, for both ends).
      static_assert(std::ranges::borrowed_range<CollectionA>);
      static_assert(std::ranges::borrowed_range<CollectionB>);
    }

    KRYS_NODISCARD auto begin() noexcept
    {
      return _begin;
    }

    KRYS_NODISCARD auto end() noexcept
    {
      return _end;
    }
  };

  // Usage: for (auto [ valueA, valueB ] : zippedRange(collectionA, collectionB)) { ... }
  template <typename CollectionA, typename CollectionB>
  KRYS_NODISCARD auto zippedRange(CollectionA &&collectionA, CollectionB &&collectionB) noexcept
  {
    using Iterator =
      ZippedRangeIterator<decltype(boundsCheckedBegin(std::forward<CollectionA>(collectionA))),
                          decltype(boundsCheckedBegin(std::forward<CollectionB>(collectionB)))>;
    return ZippedRange<Iterator>(std::forward<CollectionA>(collectionA),
                                 std::forward<CollectionB>(collectionB));
  }
}
