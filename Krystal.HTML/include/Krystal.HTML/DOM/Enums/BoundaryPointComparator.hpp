#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::BoundaryPointComparator, 4u)