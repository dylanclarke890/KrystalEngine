#include "Krystal.Booey/DOM/BoundaryPoint.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/StrongOrder.hpp"

namespace krys::boo::dom
{
  std::strong_ordering BoundaryPoint::ComparePositionTo(const BoundaryPoint &other) const noexcept
  {
    {
      const Node &a = *Container;
      const Node &b = *other.Container;

      assert(&TreeQueries::Root(a) == &TreeQueries::Root(b));

      if (&a == &b)
      {
        return Offset <=> other.Offset;
      }

      if (TreeQueries::IsFollowing(a, b))
      {
        return StrongOrder::Reverse(other.ComparePositionTo(*this));
      }

      if (TreeQueries::IsAncestor(a, b))
      {
        const Node *child = &b;
        while (!TreeQueries::IsChild(*child, a))
        {
          child = child->ParentNode();
        }

        if (TreeQueries::Index(*child) < Offset)
        {
          return std::strong_ordering::greater;
        }
      }

      return std::strong_ordering::less;
    }
  }

  Maybe<BoundaryPoint> MakeBoundaryPointBeforeNode(Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    if (!parent)
    {
      return null;
    }
    return BoundaryPoint {ShareRef<Node>(*parent), node.ComputeNodeIndex()};
  }

  Maybe<BoundaryPoint> MakeBoundaryPointAfterNode(Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    if (!parent)
    {
      return null;
    }
    return BoundaryPoint {ShareRef<Node>(*parent), node.ComputeNodeIndex() + 1};
  }

  namespace
  {
    KRYS_NODISCARD bool IsOffsetBeforeChild(const ContainerNode &container, uint64 offset,
                                            const Node &child) noexcept
    {
      if (!offset)
      {
        return true;
      }
      // If the container is not the parent, the child is part of a shadow tree, which we sort between offset
      // 0 and offset 1.
      if (child.ParentNode() != &container)
      {
        return false;
      }
      uint64 currentOffset = 0;
      for (auto *currentChild = container.FirstChild(); currentChild && currentChild != &child;
           currentChild = currentChild->NextSibling())
      {
        if (offset <= ++currentOffset)
        {
          return true;
        }
      }
      return false;
    }
  }

  template <TreeType treeType>
  std::partial_ordering TreeOrder(const BoundaryPoint &a, const BoundaryPoint &b) noexcept
  {
    if (a.Container.get() == b.Container.get())
    {
      return a.Offset <=> b.Offset;
    }

    for (const Node *ancestor = b.Container.get(); ancestor;)
    {
      auto *nextAncestor = Parent<treeType>(*ancestor);
      if (nextAncestor == a.Container.get())
      {
        return IsOffsetBeforeChild(*nextAncestor, a.Offset, *ancestor) ? std::partial_ordering::less
                                                                       : std::partial_ordering::greater;
      }
      ancestor = nextAncestor;
    }

    for (const Node *ancestor = a.Container.get(); ancestor;)
    {
      auto *nextAncestor = Parent<treeType>(*ancestor);
      if (nextAncestor == b.Container.get())
      {
        return IsOffsetBeforeChild(*nextAncestor, b.Offset, *ancestor) ? std::partial_ordering::greater
                                                                       : std::partial_ordering::less;
      }
      ancestor = nextAncestor;
    }

    return TreeOrder<treeType>(*a.Container, *b.Container);
  }

  template std::partial_ordering TreeOrder<TreeType::Tree>(const BoundaryPoint &,
                                                           const BoundaryPoint &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ShadowIncludingTree>(const BoundaryPoint &,
                                                                          const BoundaryPoint &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ComposedTree>(const BoundaryPoint &,
                                                                   const BoundaryPoint &) noexcept;
}