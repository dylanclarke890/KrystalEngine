#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dom-range-compareboundarypoints
  enum class BoundaryPointComparator
  {
    StartToStart,
    StartToEnd,
    EndToEnd,
    EndToStart,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::BoundaryPointComparator, 4u)