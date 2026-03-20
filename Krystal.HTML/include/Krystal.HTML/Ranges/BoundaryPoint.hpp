#pragma once

#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Utils/StrongOrder.hpp"
#include <compare>

namespace Krys::HTML
{
  struct BoundaryPoint
  {
    Ref<Node> Container;
    uint64 Offset;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-position
    auto ComparePositionTo(const BoundaryPoint &other) const noexcept
    {
      const Node &a = *Container;
      const Node &b = *other.Container;

      assert(TreeQueries::HasSameRoot(a, b));

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
        RawPtr<const Node> child = &b;
        while (!TreeQueries::IsChild(a, *child))
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
  };
}