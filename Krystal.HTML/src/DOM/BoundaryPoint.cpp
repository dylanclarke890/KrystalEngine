#include "Krystal.HTML/DOM/BoundaryPoint.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Utils/StrongOrder.hpp"

namespace Krys::HTML
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
        RawPtr<const Node> child = &b;
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