#pragma once

#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <compare>

namespace Krys::HTML
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