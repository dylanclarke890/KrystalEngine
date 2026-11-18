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
