#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::dom
{
  Maybe<SimpleRange> MakeRangeSelectingNode(Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    if (!parent)
    {
      return null;
    }
    uint64 offset = node.ComputeNodeIndex();
    return SimpleRange {{ShareRef<Node>(*parent), offset}, {ShareRef<Node>(*parent), offset + 1}};
  }

  SimpleRange MakeRangeSelectingNodeContents(Node &node) noexcept
  {
    return {MakeBoundaryPointBeforeNodeContents(node), MakeBoundaryPointAfterNodeContents(node)};
  }

  OffsetRange CharacterDataOffsetRange(const SimpleRange &range, const Node &node) noexcept
  {
    return {&node == range.Start.Container.get() ? range.Start.Offset : 0,
            &node == range.End.Container.get() ? range.End.Offset : std::numeric_limits<uint64>::max()};
  }

  namespace
  {
    KRYS_NODISCARD RefPtr<Node> FirstIntersectingNode(const SimpleRange &range) noexcept
    {
      if (Is<CharacterData>(*range.Start.Container))
      {
        return range.Start.Container;
      }
      if (auto *child = range.Start.Container->TraverseToChildAt(static_cast<uint32>(range.Start.Offset)))
      {
        return ShareRefPtr(child);
      }
      return ShareRefPtr(TreeQueries::NextSkippingChildren(*range.Start.Container));
    }

    KRYS_NODISCARD RefPtr<Node> NodePastLastIntersectingNode(const SimpleRange &range) noexcept
    {
      if (Is<CharacterData>(*range.End.Container))
      {
        return ShareRefPtr(TreeQueries::NextSkippingChildren(*range.End.Container));
      }
      if (auto *child = range.End.Container->TraverseToChildAt(static_cast<uint32>(range.End.Offset)))
      {
        return ShareRefPtr(child);
      }
      return ShareRefPtr(TreeQueries::NextSkippingChildren(*range.End.Container));
    }

    KRYS_NODISCARD RefPtr<Node>
      FirstIntersectingNodeWithDeprecatedZeroOffsetStartQuirk(const SimpleRange &range) noexcept
    {
      if (Is<CharacterData>(*range.Start.Container))
      {
        return range.Start.Container;
      }
      if (auto *child = range.Start.Container->TraverseToChildAt(static_cast<uint32>(range.Start.Offset)))
      {
        return ShareRefPtr(child);
      }
      if (!range.Start.Offset)
      {
        return range.Start.Container;
      }
      return ShareRefPtr(TreeQueries::NextSkippingChildren(*range.Start.Container));
    }
  }

  IntersectingNodeIterator::IntersectingNodeIterator(const SimpleRange &range) noexcept
      : _node(FirstIntersectingNode(range)), _pastLastNode(NodePastLastIntersectingNode(range))
  {
    EnforceEndInvariant();
  }

  IntersectingNodeIterator::IntersectingNodeIterator(const SimpleRange &range, QuirkFlag) noexcept
      : _node(FirstIntersectingNodeWithDeprecatedZeroOffsetStartQuirk(range)),
        _pastLastNode(NodePastLastIntersectingNode(range))
  {
    EnforceEndInvariant();
  }

  void IntersectingNodeIterator::Advance() noexcept
  {
    krys_debug_assert(_node);
    _node = ShareRefPtr(TreeQueries::Next(*_node));
    EnforceEndInvariant();
  }

  void IntersectingNodeIterator::AdvanceSkippingChildren() noexcept
  {
    krys_debug_assert(_node);
    _node =
      _node->Contains(_pastLastNode.get()) ? nullptr : ShareRefPtr(TreeQueries::NextSkippingChildren(*_node));
    EnforceEndInvariant();
  }

  void IntersectingNodeIterator::EnforceEndInvariant() noexcept
  {
    if (_node == _pastLastNode || !_node)
    {
      _node = nullptr;
      _pastLastNode = nullptr;
    }
  }

  template <TreeType treeType>
  Node *CommonInclusiveAncestor(const SimpleRange &range) noexcept
  {
    return CommonInclusiveAncestor<treeType>(*range.Start.Container, *range.End.Container);
  }

  template Node *CommonInclusiveAncestor<TreeType::Tree>(const SimpleRange &) noexcept;
  template Node *CommonInclusiveAncestor<TreeType::ComposedTree>(const SimpleRange &) noexcept;

  template <TreeType treeType>
  bool Contains(const SimpleRange &range, const BoundaryPoint &point) noexcept
  {
    return std::is_lteq(TreeOrder<treeType>(range.Start, point))
           && std::is_lteq(TreeOrder<treeType>(point, range.End));
  }

  template bool Contains<TreeType::Tree>(const SimpleRange &, const BoundaryPoint &) noexcept;
  template bool Contains<TreeType::ShadowIncludingTree>(const SimpleRange &, const BoundaryPoint &) noexcept;
  template bool Contains<TreeType::ComposedTree>(const SimpleRange &, const BoundaryPoint &) noexcept;

  template <TreeType treeType>
  std::partial_ordering TreeOrder(const SimpleRange &range, const BoundaryPoint &point) noexcept
  {
    if (auto order = TreeOrder<treeType>(range.Start, point); !std::is_lt(order))
    {
      return order;
    }
    if (auto order = TreeOrder<treeType>(range.End, point); !std::is_gt(order))
    {
      return order;
    }
    return std::partial_ordering::equivalent;
  }

  template <TreeType treeType>
  std::partial_ordering TreeOrder(const BoundaryPoint &point, const SimpleRange &range) noexcept
  {
    if (auto order = TreeOrder<treeType>(point, range.Start); !std::is_gt(order))
    {
      return order;
    }
    if (auto order = TreeOrder<treeType>(point, range.End); !std::is_lt(order))
    {
      return order;
    }
    return std::partial_ordering::equivalent;
  }

  template std::partial_ordering TreeOrder<TreeType::Tree>(const SimpleRange &,
                                                           const BoundaryPoint &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::Tree>(const BoundaryPoint &,
                                                           const SimpleRange &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ComposedTree>(const SimpleRange &,
                                                                   const BoundaryPoint &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ComposedTree>(const BoundaryPoint &,
                                                                   const SimpleRange &) noexcept;

  template <TreeType treeType>
  bool Contains(const SimpleRange &outerRange, const SimpleRange &innerRange) noexcept
  {
    return std::is_lteq(TreeOrder<treeType>(outerRange.Start, innerRange.Start))
           && std::is_gteq(TreeOrder<treeType>(outerRange.End, innerRange.End));
  }

  template bool Contains<TreeType::Tree>(const SimpleRange &, const SimpleRange &) noexcept;
  template bool Contains<TreeType::ShadowIncludingTree>(const SimpleRange &, const SimpleRange &) noexcept;
  template bool Contains<TreeType::ComposedTree>(const SimpleRange &, const SimpleRange &) noexcept;

  template <TreeType treeType>
  bool Intersects(const SimpleRange &a, const SimpleRange &b) noexcept
  {
    return std::is_lteq(TreeOrder<treeType>(a.Start, b.End))
           && std::is_lteq(TreeOrder<treeType>(b.Start, a.End));
  }

  template bool Intersects<TreeType::Tree>(const SimpleRange &, const SimpleRange &) noexcept;
  template bool Intersects<TreeType::ShadowIncludingTree>(const SimpleRange &, const SimpleRange &) noexcept;
  template bool Intersects<TreeType::ComposedTree>(const SimpleRange &, const SimpleRange &) noexcept;

  namespace
  {
    KRYS_NODISCARD bool CompareByComposedTreeOrder(const BoundaryPoint &a, const BoundaryPoint &b) noexcept
    {
      return std::is_lt(TreeOrder<TreeType::ComposedTree>(a, b));
    }
  }

  SimpleRange UnionRange(const SimpleRange &a, const SimpleRange &b) noexcept
  {
    return {std::min(a.Start, b.Start, CompareByComposedTreeOrder),
            std::max(a.End, b.End, CompareByComposedTreeOrder)};
  }

  Maybe<SimpleRange> Intersection(const Maybe<SimpleRange> &a, const Maybe<SimpleRange> &b) noexcept
  {
    // FIXME: Can this be done more efficiently, with fewer calls to treeOrder?
    if (!a || !b || !Intersects<TreeType::ComposedTree>(*a, *b))
    {
      return null;
    }
    return SimpleRange {std::max(a->Start, b->Start, CompareByComposedTreeOrder),
                        std::min(a->End, b->End, CompareByComposedTreeOrder)};
  }

  template <TreeType treeType>
  bool Contains(const SimpleRange &range, const Node &node) noexcept
  {
    // FIXME: Consider a more efficient algorithm that avoids always computing the node index.
    // FIXME: Does this const_cast point to a design problem?
    auto nodeRange = MakeRangeSelectingNode(const_cast<Node &>(node));
    return nodeRange && Contains<treeType>(range, *nodeRange);
  }

  template bool Contains<TreeType::Tree>(const SimpleRange &, const Node &) noexcept;
  template bool Contains<TreeType::ShadowIncludingTree>(const SimpleRange &, const Node &) noexcept;
  template bool Contains<TreeType::ComposedTree>(const SimpleRange &, const Node &) noexcept;

  namespace
  {
    template <TreeType treeType>
    KRYS_NODISCARD bool ContainsNode(const Node &outer, const Node &inner) noexcept
    {
      for (auto *inclusiveAncestor = &inner; inclusiveAncestor;
           inclusiveAncestor = Parent<treeType>(*inclusiveAncestor))
      {
        if (inclusiveAncestor == &outer)
        {
          return true;
        }
      }
      return false;
    }
  }

  template <TreeType treeType>
  bool Intersects(const SimpleRange &range, const Node &node) noexcept
  {
    // FIXME: Consider a more efficient algorithm that avoids always computing the node index.
    // FIXME: Does this const_cast point to a design problem?
    auto nodeRange = MakeRangeSelectingNode(const_cast<Node &>(node));
    if (!nodeRange)
    {
      return ContainsNode<treeType>(node, *range.Start.Container);
    }
    return std::is_lt(TreeOrder<treeType>(nodeRange->Start, range.End))
           && std::is_lt(TreeOrder<treeType>(range.Start, nodeRange->End));
  }

  template bool Intersects<TreeType::Tree>(const SimpleRange &, const Node &) noexcept;
  template bool Intersects<TreeType::ShadowIncludingTree>(const SimpleRange &, const Node &) noexcept;
  template bool Intersects<TreeType::ComposedTree>(const SimpleRange &, const Node &) noexcept;
}
