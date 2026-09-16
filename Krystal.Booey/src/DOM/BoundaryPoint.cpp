#include "Krystal.Booey/DOM/BoundaryPoint.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
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
}