#pragma once

#include "Krystal.Booey/DOM/BoundaryPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <compare>
#include <cstddef>
#include <iterator>
#include <utility>

namespace krys::boo::dom
{
  /// @brief A range between two boundary points that, unlike a live `Range`, doesn't follow changes to the
  /// tree.
  struct SimpleRange
  {
    BoundaryPoint Start;
    BoundaryPoint End;

    KRYS_NODISCARD Node &StartContainer() const noexcept
    {
      return *Start.Container;
    }
    KRYS_NODISCARD uint64 StartOffset() const noexcept
    {
      return Start.Offset;
    }
    KRYS_NODISCARD Node &EndContainer() const noexcept
    {
      return *End.Container;
    }
    KRYS_NODISCARD uint64 EndOffset() const noexcept
    {
      return End.Offset;
    }

    KRYS_NODISCARD bool Collapsed() const noexcept
    {
      return Start == End;
    }

    KRYS_NODISCARD friend bool operator==(const SimpleRange &, const SimpleRange &) noexcept = default;
  };

  KRYS_NODISCARD inline SimpleRange MakeSimpleRangeHelper(BoundaryPoint &&start, BoundaryPoint &&end) noexcept
  {
    return {krys::move(start), krys::move(end)};
  }

  KRYS_NODISCARD inline Maybe<SimpleRange> MakeSimpleRangeHelper(Maybe<BoundaryPoint> &&start,
                                                                 Maybe<BoundaryPoint> &&end) noexcept
  {
    if (!start || !end)
    {
      return null;
    }
    return MakeSimpleRangeHelper(krys::move(*start), krys::move(*end));
  }

  KRYS_NODISCARD inline SimpleRange MakeSimpleRangeHelper(BoundaryPoint &&point) noexcept
  {
    auto end = point;
    return MakeSimpleRangeHelper(krys::move(point), krys::move(end));
  }

  KRYS_NODISCARD inline Maybe<SimpleRange> MakeSimpleRangeHelper(Maybe<BoundaryPoint> &&point) noexcept
  {
    if (!point)
    {
      return null;
    }
    return MakeSimpleRangeHelper(krys::move(*point));
  }

  KRYS_NODISCARD inline BoundaryPoint MakeBoundaryPointHelper(const BoundaryPoint &point) noexcept
  {
    return point;
  }
  KRYS_NODISCARD inline BoundaryPoint MakeBoundaryPointHelper(BoundaryPoint &&point) noexcept
  {
    return krys::move(point);
  }
  KRYS_NODISCARD inline Maybe<BoundaryPoint>
    MakeBoundaryPointHelper(const Maybe<BoundaryPoint> &point) noexcept
  {
    return point;
  }
  KRYS_NODISCARD inline Maybe<BoundaryPoint> MakeBoundaryPointHelper(Maybe<BoundaryPoint> &&point) noexcept
  {
    return krys::move(point);
  }
  /// @brief Anything with a `MakeBoundaryPoint` (positions and visible positions).
  template <typename T>
  KRYS_NODISCARD auto MakeBoundaryPointHelper(T &&argument) noexcept
    -> decltype(MakeBoundaryPoint(std::forward<T>(argument)))
  {
    return MakeBoundaryPoint(std::forward<T>(argument));
  }

  /// @brief A range from one point, or between two, each a boundary point or anything that makes one; null
  /// when a point is null.
  template <typename... T>
  KRYS_NODISCARD auto MakeSimpleRange(T &&...arguments) noexcept
    -> decltype(MakeSimpleRangeHelper(MakeBoundaryPointHelper(std::forward<T>(arguments))...))
  {
    return MakeSimpleRangeHelper(MakeBoundaryPointHelper(std::forward<T>(arguments))...);
  }

  // FIXME: Would like these two functions to have shorter names; another option is to change prefix to
  // makeSimpleRange.
  KRYS_NODISCARD Maybe<SimpleRange> MakeRangeSelectingNode(Node &node) noexcept;
  KRYS_NODISCARD SimpleRange MakeRangeSelectingNodeContents(Node &node) noexcept;

  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD Node *CommonInclusiveAncestor(const SimpleRange &range) noexcept;
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD bool Contains(const SimpleRange &range, const BoundaryPoint &point) noexcept;
  template <TreeType treeType = TreeType::Tree>
  KRYS_NODISCARD bool Contains(const SimpleRange &range, const Maybe<BoundaryPoint> &point) noexcept
  {
    return point && Contains<treeType>(range, *point);
  }
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD bool Contains(const SimpleRange &outerRange, const SimpleRange &innerRange) noexcept;
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD bool Contains(const SimpleRange &range, const Node &node) noexcept;

  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD bool Intersects(const SimpleRange &a, const SimpleRange &b) noexcept;
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD bool Intersects(const SimpleRange &range, const Node &node) noexcept;

  // Returns equivalent if point is in range.
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD std::partial_ordering TreeOrder(const SimpleRange &range,
                                                 const BoundaryPoint &point) noexcept;
  template <TreeType = TreeType::Tree>
  KRYS_NODISCARD std::partial_ordering TreeOrder(const BoundaryPoint &point,
                                                 const SimpleRange &range) noexcept;

  /// @brief The part of a character data node's text a range covers.
  struct OffsetRange
  {
    uint64 Start {0};
    uint64 End {0};
  };

  KRYS_NODISCARD OffsetRange CharacterDataOffsetRange(const SimpleRange &range, const Node &node) noexcept;

  // FIXME: Start of functions that are deprecated since they silently default to ComposedTree.
  KRYS_NODISCARD SimpleRange UnionRange(const SimpleRange &a, const SimpleRange &b) noexcept;
  KRYS_NODISCARD Maybe<SimpleRange> Intersection(const Maybe<SimpleRange> &a,
                                                 const Maybe<SimpleRange> &b) noexcept;
  // FIXME: End of functions that are deprecated since they silently default to ComposedTree.

  /// @brief Walks the nodes a range intersects, in tree order.
  class IntersectingNodeIterator
  {
  private:
    RefPtr<Node> _node;
    RefPtr<Node> _pastLastNode;

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Node;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type *;
    using reference = value_type &;

    enum QuirkFlag
    {
      DeprecatedZeroOffsetStartQuirk
    };

    explicit IntersectingNodeIterator(const SimpleRange &range) noexcept;
    IntersectingNodeIterator(const SimpleRange &range, QuirkFlag) noexcept;

    KRYS_NODISCARD Node &operator*() const noexcept
    {
      return *_node;
    }
    KRYS_NODISCARD Node *operator->() const noexcept
    {
      return _node.get();
    }
    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _node != nullptr;
    }
    KRYS_NODISCARD bool operator==(std::nullptr_t) const noexcept
    {
      return !_node;
    }

    IntersectingNodeIterator &operator++() noexcept
    {
      Advance();
      return *this;
    }

    void Advance() noexcept;
    void AdvanceSkippingChildren() noexcept;

  private:
    void EnforceEndInvariant() noexcept;
  };

  class IntersectingNodeRange
  {
  private:
    SimpleRange _range;

  public:
    explicit IntersectingNodeRange(const SimpleRange &range) noexcept : _range(range)
    {
    }

    KRYS_NODISCARD IntersectingNodeIterator begin() const noexcept
    {
      return IntersectingNodeIterator(_range);
    }
    KRYS_NODISCARD static constexpr std::nullptr_t end() noexcept
    {
      return nullptr;
    }
  };

  /// @brief The nodes a range intersects.
  /// FIXME: Deprecated since it silently defaults to ComposedTree.
  KRYS_NODISCARD inline IntersectingNodeRange IntersectingNodes(const SimpleRange &range) noexcept
  {
    return IntersectingNodeRange(range);
  }

  class IntersectingNodeRangeWithQuirk
  {
  private:
    SimpleRange _range;

  public:
    explicit IntersectingNodeRangeWithQuirk(const SimpleRange &range) noexcept : _range(range)
    {
    }

    KRYS_NODISCARD IntersectingNodeIterator begin() const noexcept
    {
      return {_range, IntersectingNodeIterator::DeprecatedZeroOffsetStartQuirk};
    }
    KRYS_NODISCARD static constexpr std::nullptr_t end() noexcept
    {
      return nullptr;
    }
  };

  /// @brief The nodes a range intersects, starting with its start container when the range starts at its
  /// offset 0 and the container has no children.
  /// FIXME: Deprecated since it silently defaults to ComposedTree.
  KRYS_NODISCARD inline IntersectingNodeRangeWithQuirk
    IntersectingNodesWithDeprecatedZeroOffsetStartQuirk(const SimpleRange &range) noexcept
  {
    return IntersectingNodeRangeWithQuirk(range);
  }
}
