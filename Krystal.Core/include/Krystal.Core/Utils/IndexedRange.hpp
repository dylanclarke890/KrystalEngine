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

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include <ranges>
#include <utility>

namespace krys
{
  template <typename Iterator>
  class BoundsCheckedIterator
  {
  private:
    Iterator _iterator;
    Iterator _end;

    template <typename Collection>
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): only its end is taken.
    BoundsCheckedIterator(Collection &&collection, Iterator &&iterator)
        : _iterator(krys::move(iterator)), _end(std::end(collection))
    {
    }

  public:
    // We require the caller to ask for 'begin' or 'end', rather than passing
    // us arbitrary 'it' and 'end' iterators, because that way we can prove by
    // construction that we have the correct 'end'.
    template <typename Collection>
    KRYS_NODISCARD static BoundsCheckedIterator begin(Collection &&collection) noexcept
    {
      return BoundsCheckedIterator(std::forward<Collection>(collection), std::begin(collection));
    }

    template <typename Collection>
    KRYS_NODISCARD static BoundsCheckedIterator end(Collection &&collection) noexcept
    {
      return BoundsCheckedIterator(std::forward<Collection>(collection), std::end(collection));
    }

    BoundsCheckedIterator &operator++() noexcept
    {
      krys_assert(_iterator != _end);
      ++_iterator;
      return *this;
    }

    KRYS_NODISCARD auto &&operator*() const noexcept
    {
      krys_assert(_iterator != _end);
      return *_iterator;
    }

    KRYS_NODISCARD bool operator==(const BoundsCheckedIterator &other) const noexcept
    {
      return _iterator == other._iterator;
    }
  };

  template <typename Collection>
  KRYS_NODISCARD auto boundsCheckedBegin(Collection &&collection) noexcept
  {
    return BoundsCheckedIterator<decltype(std::begin(collection))>::begin(
      std::forward<Collection>(collection));
  }

  template <typename Collection>
  KRYS_NODISCARD auto boundsCheckedEnd(Collection &&collection) noexcept
  {
    return BoundsCheckedIterator<decltype(std::end(collection))>::end(std::forward<Collection>(collection));
  }

  template <typename Iterator>
  class IndexedRangeIterator
  {
  private:
    size_t _index {0uz};
    Iterator _iterator;

  public:
    IndexedRangeIterator(Iterator &&iterator) noexcept : _iterator(krys::move(iterator))
    {
    }

    IndexedRangeIterator &operator++() noexcept
    {
      ++_index;
      ++_iterator;
      return *this;
    }

    KRYS_NODISCARD auto operator*() noexcept
    {
      return std::pair<size_t, decltype(*_iterator)> {_index, *_iterator};
    }

    KRYS_NODISCARD bool operator==(const IndexedRangeIterator &other) const noexcept
    {
      return _iterator == other._iterator;
    }
  };

  template <typename Iterator>
  class IndexedRange
  {
  private:
    Iterator _begin;
    Iterator _end;

  public:
    // Constrained so it doesn't hide the copy and move constructors.
    template <typename Collection>
    requires(!SameType<remove_cvref_t<Collection>, IndexedRange>)
    // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward): used for both ends, so never moved.
    explicit IndexedRange(Collection &&collection) noexcept
        : _begin(boundsCheckedBegin(collection)), _end(boundsCheckedEnd(collection))
    {
      // Prevent use after destruction of a returned temporary (a borrowed range's iterators outlive it, so
      // the collection is passed on as an lvalue, for both ends).
      static_assert(std::ranges::borrowed_range<Collection>);
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

  // Usage: for (auto [ index, value ] : indexedRange(collection)) { ... }
  template <typename Collection>
  auto indexedRange(Collection &&collection)
  {
    using Iterator = IndexedRangeIterator<decltype(boundsCheckedBegin(std::forward<Collection>(collection)))>;
    return IndexedRange<Iterator>(std::forward<Collection>(collection));
  }
}
