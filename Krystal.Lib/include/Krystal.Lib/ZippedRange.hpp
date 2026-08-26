#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/IndexedRange.hpp"

namespace Krys
{
  template <typename IteratorA, typename IteratorB>
  class ZippedRangeIterator
  {
  private:
    IteratorA _iteratorA;
    IteratorB _iteratorB;

  public:
    ZippedRangeIterator(IteratorA &&iteratorA, IteratorB &&iteratorB)
        : _iteratorA(Krys::Move(iteratorA)), _iteratorB(Krys::Move(iteratorB))
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
    ZippedRange(CollectionA &&collectionA, CollectionB &&collectionB) noexcept
        : _begin(boundsCheckedBegin(std::forward<CollectionA>(collectionA)),
                 boundsCheckedBegin(std::forward<CollectionB>(collectionB))),
          _end(boundsCheckedEnd(std::forward<CollectionA>(collectionA)),
               boundsCheckedEnd(std::forward<CollectionB>(collectionB)))
    {
      // Prevent use after destruction of a returned temporary.
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