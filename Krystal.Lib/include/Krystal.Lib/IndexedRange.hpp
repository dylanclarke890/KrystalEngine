#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <cassert>
#include <ranges>
#include <utility>

namespace Krys
{
  template <typename Iterator>
  class BoundsCheckedIterator
  {
  private:
    Iterator _iterator;
    Iterator _end;

    template <typename Collection>
    BoundsCheckedIterator(Collection &&collection, Iterator &&iterator)
        : _iterator(Krys::Move(iterator)), _end(std::end(collection))
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
      assert(_iterator != _end);
      ++_iterator;
      return *this;
    }

    KRYS_NODISCARD auto &&operator*() const noexcept
    {
      assert(_iterator != _end);
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
    IndexedRangeIterator(Iterator &&iterator) noexcept : _iterator(Krys::Move(iterator))
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
    template <typename Collection>
    IndexedRange(Collection &&collection) noexcept
        : _begin(boundsCheckedBegin(std::forward<Collection>(collection))),
          _end(boundsCheckedEnd(std::forward<Collection>(collection)))
    {
      // Prevent use after destruction of a returned temporary.
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