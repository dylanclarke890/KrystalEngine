#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeQueries.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Utils/StrongOrder.hpp"
#include <cassert>
#include <compare>

namespace Krys::HTML
{
  struct BoundaryPoint
  {
    Ref<Node> Container;
    uint64 Offset;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-position
    constexpr auto operator<=>(const BoundaryPoint &other) const noexcept
    {
      const Node &a = *Container;
      const Node &b = *other.Container;

      assert(NodeQueries::HasSameRoot(a, b));

      if (&a == &b)
      {
        return Offset <=> other.Offset;
      }

      if (NodeQueries::IsFollowing(a, b))
      {
        return StrongOrder::Reverse(other <=> *this);
      }

      if (NodeQueries::IsAncestorOf(a, b))
      {
        RawPtr<const Node> child = &b;
        while (child && !NodeQueries::IsChildOf(a, *child))
        {
          child = child->ParentNode();
        }

        uint32 childIndex = 0;
        RawPtr<const Node> currentChild = a.FirstChild();
        while (currentChild && currentChild != child)
        {
          ++childIndex;
          currentChild = currentChild->NextSibling();
        }

        if (currentChild == child && childIndex < Offset)
        {
          return std::strong_ordering::greater;
        }
      }

      return std::strong_ordering::less;
    }
  };

  class AbstractRange
  {
  protected:
    BoundaryPoint _start;
    BoundaryPoint _end;

  protected:
    constexpr AbstractRange(BoundaryPoint start, BoundaryPoint end) noexcept : _start(start), _end(end)
    {
    }

    constexpr virtual ~AbstractRange() noexcept = default;

  public:
    KRYS_NODISCARD constexpr bool IsCollapsed() const noexcept
    {
      return _start.Container == _end.Container && _start.Offset == _end.Offset;
    }

    KRYS_NODISCARD constexpr RawPtr<Node> StartContainer() const noexcept
    {
      return _start.Container.get();
    }

    KRYS_NODISCARD constexpr uint64 StartOffset() const noexcept
    {
      return _start.Offset;
    }

    KRYS_NODISCARD constexpr RawPtr<Node> EndContainer() const noexcept
    {
      return _end.Container.get();
    }

    KRYS_NODISCARD constexpr uint64 EndOffset() const noexcept
    {
      return _end.Offset;
    }
  };
}