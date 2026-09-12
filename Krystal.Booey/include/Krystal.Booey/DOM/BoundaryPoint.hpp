#pragma once

#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <compare>

namespace krys::boo::dom
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#boundary-points
  struct BoundaryPoint
  {
    Ref<Node> Container;
    uint64 Offset;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-position
    std::strong_ordering ComparePositionTo(const BoundaryPoint &other) const noexcept;
  };
}